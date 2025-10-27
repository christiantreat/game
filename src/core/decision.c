/**
 * decision.c
 * Decision System Implementation
 */

#include "decision.h"
#include "../lib/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// ============================================================================
// Decision Context Implementation
// ============================================================================

static void extract_entity_info(DecisionContext* ctx, const Entity* entity) {
    ctx->entity_id = entity->id;
    strncpy(ctx->entity_name, entity->name, MAX_ENTITY_NAME - 1);
    strncpy(ctx->entity_type, entity->entity_type, MAX_ENTITY_TYPE - 1);

    // Initialize all "has_X" flags to false
    ctx->has_needs = false;
    ctx->has_health = false;
    ctx->has_currency = false;
    ctx->has_inventory = false;
    ctx->has_occupation = false;
    ctx->has_goal = false;
    ctx->has_relationships = false;
    ctx->has_schedule = false;
    ctx->has_memory = false;

    // Extract component data
    for (int i = 0; i < entity->component_count; i++) {
        Component* comp = entity->components[i];
        if (!comp) continue;

        switch (comp->type) {
            case COMPONENT_POSITION: {
                PositionComponent* pos = (PositionComponent*)comp;
                ctx->position_x = pos->x;
                ctx->position_y = pos->y;
                strncpy(ctx->location, pos->location, 63);
                ctx->location[63] = '\0';
                break;
            }

            case COMPONENT_HEALTH: {
                HealthComponent* health = (HealthComponent*)comp;
                ctx->health_current = health->current;
                ctx->health_max = health->maximum;
                ctx->has_health = true;
                break;
            }

            case COMPONENT_CURRENCY: {
                CurrencyComponent* currency = (CurrencyComponent*)comp;
                ctx->currency = currency->amount;
                ctx->has_currency = true;
                break;
            }

            case COMPONENT_INVENTORY: {
                InventoryComponent* inv = (InventoryComponent*)comp;
                ctx->inventory_item_count = inv->item_count;
                ctx->inventory_capacity = inv->capacity;
                ctx->has_inventory = true;
                break;
            }

            case COMPONENT_NEEDS: {
                NeedsComponent* needs = (NeedsComponent*)comp;
                ctx->hunger = needs->hunger;
                ctx->energy = needs->energy;
                ctx->social = needs->social;
                ctx->has_needs = true;
                break;
            }

            case COMPONENT_OCCUPATION: {
                OccupationComponent* occ = (OccupationComponent*)comp;
                strncpy(ctx->occupation, occ->occupation, 63);
                ctx->occupation[63] = '\0';
                ctx->skill_level = occ->skill_level;
                ctx->has_occupation = true;
                break;
            }

            case COMPONENT_GOAL: {
                GoalComponent* goal = (GoalComponent*)comp;
                strncpy(ctx->current_goal, goal->current_goal, 255);
                ctx->current_goal[255] = '\0';
                ctx->has_goal = true;
                break;
            }

            case COMPONENT_RELATIONSHIP: {
                RelationshipComponent* rel = (RelationshipComponent*)comp;
                ctx->has_relationships = (rel->relationship_count > 0);
                break;
            }

            case COMPONENT_SCHEDULE: {
                ScheduleComponent* sched = (ScheduleComponent*)comp;
                if (sched->entry_count > 0) {
                    strncpy(ctx->current_activity, sched->entries[0].activity, 127);
                    ctx->current_activity[127] = '\0';
                    ctx->schedule_hour_start = 0;  // Not available in ScheduleEntry
                    ctx->schedule_hour_end = 0;    // Not available in ScheduleEntry
                    ctx->has_schedule = true;
                }
                break;
            }

            case COMPONENT_MEMORY: {
                MemoryComponent* mem = (MemoryComponent*)comp;
                ctx->memory_count = mem->memory_count;
                ctx->has_memory = (mem->memory_count > 0);
                break;
            }

            default:
                break;
        }
    }
}

static void extract_world_state(DecisionContext* ctx, const GameState* game_state) {
    ctx->day_count = game_state->day_count;
    ctx->time_of_day = game_state->time_of_day;
    ctx->season = game_state->season;
    ctx->year = game_state->year;
    ctx->weather = game_state->current_weather;

    // Weather description
    const char* weather_str = "Clear";
    switch (ctx->weather) {
        case WEATHER_SUNNY: weather_str = "Sunny"; break;
        case WEATHER_CLOUDY: weather_str = "Cloudy"; break;
        case WEATHER_RAINY: weather_str = "Rainy"; break;
        case WEATHER_STORMY: weather_str = "Stormy"; break;
        case WEATHER_DROUGHT: weather_str = "Drought"; break;
    }
    strncpy(ctx->weather_description, weather_str, 63);
    ctx->weather_description[63] = '\0';
}

static void extract_nearby_entities(DecisionContext* ctx,
                                    const GameState* game_state,
                                    const Entity* self_entity,
                                    float radius) {
    ctx->nearby_entity_count = 0;

    // Get self position
    PositionComponent* self_pos = NULL;
    for (int i = 0; i < self_entity->component_count; i++) {
        if (self_entity->components[i] &&
            self_entity->components[i]->type == COMPONENT_POSITION) {
            self_pos = (PositionComponent*)self_entity->components[i];
            break;
        }
    }

    if (!self_pos) return;

    // Get self relationships
    RelationshipComponent* self_rel = NULL;
    for (int i = 0; i < self_entity->component_count; i++) {
        if (self_entity->components[i] &&
            self_entity->components[i]->type == COMPONENT_RELATIONSHIP) {
            self_rel = (RelationshipComponent*)self_entity->components[i];
            break;
        }
    }

    // Find nearby entities
    for (int i = 0; i < game_state->entity_manager->entity_count; i++) {
        Entity* other = game_state->entity_manager->entities[i];
        if (!other || !other->active || other->id == self_entity->id) continue;

        // Get other's position
        PositionComponent* other_pos = NULL;
        for (int j = 0; j < other->component_count; j++) {
            if (other->components[j] &&
                other->components[j]->type == COMPONENT_POSITION) {
                other_pos = (PositionComponent*)other->components[j];
                break;
            }
        }

        if (!other_pos) continue;

        // Check distance
        float dx = other_pos->x - self_pos->x;
        float dy = other_pos->y - self_pos->y;
        float dist = sqrtf(dx * dx + dy * dy);

        if (dist <= radius && ctx->nearby_entity_count < MAX_NEARBY_ENTITIES) {
            ctx->nearby_entity_ids[ctx->nearby_entity_count] = other->id;
            strncpy(ctx->nearby_entity_names[ctx->nearby_entity_count],
                   other->name,
                   MAX_ENTITY_NAME - 1);

            // Get relationship value
            int rel_value = 0;
            if (self_rel) {
                rel_value = relationship_component_get(self_rel, other->id);
            }
            ctx->nearby_relationship_values[ctx->nearby_entity_count] = rel_value;

            ctx->nearby_entity_count++;
        }
    }
}

static void extract_recent_events(DecisionContext* ctx,
                                  const EventLogger* event_logger,
                                  int entity_id) {
    ctx->recent_event_count = 0;

    if (!event_logger) return;

    // Get recent events for this entity
    GameEvent* events[MAX_RECENT_EVENTS];
    int count = event_logger_get_by_entity(event_logger, entity_id,
                                           events, MAX_RECENT_EVENTS);

    for (int i = 0; i < count && i < MAX_RECENT_EVENTS; i++) {
        ctx->recent_event_ids[ctx->recent_event_count++] = events[i]->id;
    }
}

DecisionContext* decision_context_create(const GameState* game_state,
                                         const Entity* entity,
                                         const EventLogger* event_logger) {
    return decision_context_create_with_nearby(game_state, entity,
                                               event_logger, 100.0f);
}

DecisionContext* decision_context_create_with_nearby(const GameState* game_state,
                                                     const Entity* entity,
                                                     const EventLogger* event_logger,
                                                     float nearby_radius) {
    if (!game_state || !entity) return NULL;

    DecisionContext* ctx = calloc(1, sizeof(DecisionContext));
    if (!ctx) return NULL;

    extract_entity_info(ctx, entity);
    extract_world_state(ctx, game_state);
    extract_nearby_entities(ctx, game_state, entity, nearby_radius);
    extract_recent_events(ctx, event_logger, entity->id);

    return ctx;
}

void decision_context_destroy(DecisionContext* context) {
    if (context) {
        free(context);
    }
}

cJSON* decision_context_to_json(const DecisionContext* context) {
    if (!context) return NULL;

    cJSON* json = cJSON_CreateObject();

    // Entity info
    cJSON_AddNumberToObject(json, "entity_id", context->entity_id);
    cJSON_AddStringToObject(json, "entity_name", context->entity_name);
    cJSON_AddStringToObject(json, "entity_type", context->entity_type);

    // World state
    cJSON_AddNumberToObject(json, "day_count", context->day_count);
    cJSON_AddNumberToObject(json, "time_of_day", context->time_of_day);
    cJSON_AddNumberToObject(json, "season", context->season);
    cJSON_AddNumberToObject(json, "year", context->year);
    cJSON_AddStringToObject(json, "weather", context->weather_description);

    // Position
    cJSON_AddNumberToObject(json, "position_x", context->position_x);
    cJSON_AddNumberToObject(json, "position_y", context->position_y);
    cJSON_AddStringToObject(json, "location", context->location);

    // Needs
    if (context->has_needs) {
        cJSON* needs = cJSON_CreateObject();
        cJSON_AddNumberToObject(needs, "hunger", context->hunger);
        cJSON_AddNumberToObject(needs, "energy", context->energy);
        cJSON_AddNumberToObject(needs, "social", context->social);
        cJSON_AddItemToObject(json, "needs", needs);
    }

    // Health
    if (context->has_health) {
        cJSON* health = cJSON_CreateObject();
        cJSON_AddNumberToObject(health, "current", context->health_current);
        cJSON_AddNumberToObject(health, "max", context->health_max);
        cJSON_AddItemToObject(json, "health", health);
    }

    // Currency
    if (context->has_currency) {
        cJSON_AddNumberToObject(json, "currency", context->currency);
    }

    // Inventory
    if (context->has_inventory) {
        cJSON* inv = cJSON_CreateObject();
        cJSON_AddNumberToObject(inv, "item_count", context->inventory_item_count);
        cJSON_AddNumberToObject(inv, "capacity", context->inventory_capacity);
        cJSON_AddItemToObject(json, "inventory", inv);
    }

    // Occupation
    if (context->has_occupation) {
        cJSON* occ = cJSON_CreateObject();
        cJSON_AddStringToObject(occ, "occupation", context->occupation);
        cJSON_AddNumberToObject(occ, "skill_level", context->skill_level);
        cJSON_AddItemToObject(json, "occupation", occ);
    }

    // Goal
    if (context->has_goal) {
        cJSON_AddStringToObject(json, "current_goal", context->current_goal);
    }

    // Nearby entities
    if (context->nearby_entity_count > 0) {
        cJSON* nearby = cJSON_CreateArray();
        for (int i = 0; i < context->nearby_entity_count; i++) {
            cJSON* entity = cJSON_CreateObject();
            cJSON_AddNumberToObject(entity, "id", context->nearby_entity_ids[i]);
            cJSON_AddStringToObject(entity, "name", context->nearby_entity_names[i]);
            cJSON_AddNumberToObject(entity, "relationship",
                                   context->nearby_relationship_values[i]);
            cJSON_AddItemToArray(nearby, entity);
        }
        cJSON_AddItemToObject(json, "nearby_entities", nearby);
    }

    return json;
}

void decision_context_print(const DecisionContext* context) {
    if (!context) return;

    printf("\n=== Decision Context ===\n");
    printf("Entity: %s (#%d) - %s\n", context->entity_name,
           context->entity_id, context->entity_type);
    printf("Location: (%.1f, %.1f) %s\n", context->position_x,
           context->position_y, context->location);
    printf("Day %d, Year %d, Season %d, Time %d, Weather: %s\n",
           context->day_count, context->year, context->season,
           context->time_of_day, context->weather_description);

    if (context->has_needs) {
        printf("Needs: Hunger=%.1f Energy=%.1f Social=%.1f\n",
               context->hunger, context->energy, context->social);
    }

    if (context->has_health) {
        printf("Health: %d/%d\n", context->health_current, context->health_max);
    }

    if (context->has_currency) {
        printf("Currency: %d gold\n", context->currency);
    }

    if (context->has_occupation) {
        printf("Occupation: %s (Level %d)\n", context->occupation, context->skill_level);
    }

    if (context->has_goal) {
        printf("Goal: %s\n", context->current_goal);
    }

    if (context->nearby_entity_count > 0) {
        printf("Nearby Entities (%d):\n", context->nearby_entity_count);
        for (int i = 0; i < context->nearby_entity_count; i++) {
            printf("  - %s (relationship: %d)\n",
                   context->nearby_entity_names[i],
                   context->nearby_relationship_values[i]);
        }
    }

    printf("=======================\n\n");
}

// ============================================================================
// Decision Action Helpers
// ============================================================================

const char* decision_action_to_string(DecisionAction action) {
    switch (action) {
        case DECISION_ACTION_MOVE: return "Move";
        case DECISION_ACTION_TALK: return "Talk";
        case DECISION_ACTION_TRADE: return "Trade";
        case DECISION_ACTION_WORK: return "Work";
        case DECISION_ACTION_REST: return "Rest";
        case DECISION_ACTION_EAT: return "Eat";
        case DECISION_ACTION_PLANT: return "Plant";
        case DECISION_ACTION_HARVEST: return "Harvest";
        case DECISION_ACTION_WATER: return "Water";
        case DECISION_ACTION_GIVE_GIFT: return "Give Gift";
        case DECISION_ACTION_WAIT: return "Wait";
        case DECISION_ACTION_NONE: return "None";
        default: return "Unknown";
    }
}

// ============================================================================
// Decision Record Implementation
// ============================================================================

DecisionRecord* decision_record_create(const DecisionContext* context,
                                       const DecisionOption* options,
                                       int option_count,
                                       int chosen_index,
                                       const char* reasoning) {
    if (!context || !options || option_count <= 0 || chosen_index < 0 ||
        chosen_index >= option_count) {
        return NULL;
    }

    DecisionRecord* record = calloc(1, sizeof(DecisionRecord));
    if (!record) return NULL;

    record->id = 0;  // Will be set by logger
    record->timestamp = time(NULL);
    record->game_day = context->day_count;

    // Game time
    const char* time_str = "Unknown";
    switch (context->time_of_day) {
        case TIME_MORNING: time_str = "Morning"; break;
        case TIME_AFTERNOON: time_str = "Afternoon"; break;
        case TIME_EVENING: time_str = "Evening"; break;
        case TIME_NIGHT: time_str = "Night"; break;
    }
    strncpy(record->game_time, time_str, 31);
    record->game_time[31] = '\0';

    record->entity_id = context->entity_id;
    strncpy(record->entity_name, context->entity_name, MAX_ENTITY_NAME - 1);

    // Deep copy context
    record->context = malloc(sizeof(DecisionContext));
    if (record->context) {
        memcpy(record->context, context, sizeof(DecisionContext));
    }

    // Copy options
    record->option_count = option_count > MAX_DECISION_OPTIONS ?
                          MAX_DECISION_OPTIONS : option_count;
    for (int i = 0; i < record->option_count; i++) {
        record->options[i] = options[i];
    }

    record->chosen_option_index = chosen_index;
    record->chosen_action = options[chosen_index].action;

    // Copy reasoning
    if (reasoning) {
        strncpy(record->reasoning, reasoning, MAX_DECISION_REASONING - 1);
        record->reasoning[MAX_DECISION_REASONING - 1] = '\0';
    } else {
        record->reasoning[0] = '\0';
    }

    record->executed = false;
    record->succeeded = false;
    record->actual_utility = 0.0f;
    record->outcome_description[0] = '\0';

    return record;
}

void decision_record_destroy(DecisionRecord* record) {
    if (record) {
        if (record->context) {
            free(record->context);
        }
        free(record);
    }
}

void decision_record_set_outcome(DecisionRecord* record,
                                bool succeeded,
                                float actual_utility,
                                const char* outcome_description) {
    if (!record) return;

    record->executed = true;
    record->succeeded = succeeded;
    record->actual_utility = actual_utility;

    if (outcome_description) {
        strncpy(record->outcome_description, outcome_description, 255);
        record->outcome_description[255] = '\0';
    }
}

cJSON* decision_record_to_json(const DecisionRecord* record) {
    if (!record) return NULL;

    cJSON* json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "id", (double)record->id);
    cJSON_AddNumberToObject(json, "timestamp", (double)record->timestamp);
    cJSON_AddNumberToObject(json, "game_day", record->game_day);
    cJSON_AddStringToObject(json, "game_time", record->game_time);
    cJSON_AddNumberToObject(json, "entity_id", record->entity_id);
    cJSON_AddStringToObject(json, "entity_name", record->entity_name);

    // Context
    if (record->context) {
        cJSON_AddItemToObject(json, "context",
                             decision_context_to_json(record->context));
    }

    // Options
    cJSON* options_array = cJSON_CreateArray();
    for (int i = 0; i < record->option_count; i++) {
        const DecisionOption* opt = &record->options[i];
        cJSON* opt_json = cJSON_CreateObject();
        cJSON_AddStringToObject(opt_json, "action",
                               decision_action_to_string(opt->action));
        cJSON_AddStringToObject(opt_json, "description", opt->description);
        cJSON_AddNumberToObject(opt_json, "utility", opt->utility);
        cJSON_AddNumberToObject(opt_json, "cost", opt->cost);
        cJSON_AddNumberToObject(opt_json, "success_chance", opt->success_chance);
        cJSON_AddItemToArray(options_array, opt_json);
    }
    cJSON_AddItemToObject(json, "options", options_array);

    cJSON_AddNumberToObject(json, "chosen_option_index", record->chosen_option_index);
    cJSON_AddStringToObject(json, "chosen_action",
                           decision_action_to_string(record->chosen_action));
    cJSON_AddStringToObject(json, "reasoning", record->reasoning);

    // Outcome
    cJSON_AddBoolToObject(json, "executed", record->executed);
    cJSON_AddBoolToObject(json, "succeeded", record->succeeded);
    cJSON_AddNumberToObject(json, "actual_utility", record->actual_utility);
    cJSON_AddStringToObject(json, "outcome_description", record->outcome_description);

    return json;
}

void decision_record_print(const DecisionRecord* record) {
    if (!record) return;

    printf("\n=== Decision Record #%llu ===\n", (unsigned long long)record->id);
    printf("Entity: %s (#%d)\n", record->entity_name, record->entity_id);
    printf("Day %d, %s\n", record->game_day, record->game_time);

    printf("\nOptions Considered (%d):\n", record->option_count);
    for (int i = 0; i < record->option_count; i++) {
        const DecisionOption* opt = &record->options[i];
        printf("  %s %d. %s - %s (utility: %.2f, cost: %.2f)\n",
               i == record->chosen_option_index ? ">>>" : "   ",
               i + 1,
               decision_action_to_string(opt->action),
               opt->description,
               opt->utility,
               opt->cost);
    }

    printf("\nChosen: %s\n", decision_action_to_string(record->chosen_action));
    printf("Reasoning: %s\n", record->reasoning);

    if (record->executed) {
        printf("\nOutcome: %s\n", record->succeeded ? "SUCCESS" : "FAILED");
        printf("Actual Utility: %.2f\n", record->actual_utility);
        printf("Description: %s\n", record->outcome_description);
    } else {
        printf("\nOutcome: Not yet executed\n");
    }

    printf("============================\n\n");
}

// ============================================================================
// Decision Logger Implementation
// ============================================================================

DecisionLogger* decision_logger_create(void) {
    DecisionLogger* logger = calloc(1, sizeof(DecisionLogger));
    if (!logger) return NULL;

    logger->decision_count = 0;
    logger->head = 0;
    logger->tail = 0;
    logger->full = false;
    logger->next_decision_id = 1;

    logger->total_decisions = 0;
    logger->successful_decisions = 0;
    logger->failed_decisions = 0;

    for (int i = 0; i < DECISION_ACTION_COUNT; i++) {
        logger->decisions_by_action[i] = 0;
    }

    return logger;
}

void decision_logger_destroy(DecisionLogger* logger) {
    if (!logger) return;

    // Free all decision records
    for (int i = 0; i < MAX_DECISION_LOG_SIZE; i++) {
        if (logger->decisions[i]) {
            decision_record_destroy(logger->decisions[i]);
        }
    }

    free(logger);
}

void decision_logger_log(DecisionLogger* logger, DecisionRecord* record) {
    if (!logger || !record) return;

    // Assign ID
    record->id = logger->next_decision_id++;

    // Free old record if overwriting
    if (logger->full && logger->decisions[logger->head]) {
        decision_record_destroy(logger->decisions[logger->head]);
    }

    // Add to ring buffer
    logger->decisions[logger->head] = record;
    logger->head = (logger->head + 1) % MAX_DECISION_LOG_SIZE;

    if (logger->full) {
        logger->tail = (logger->tail + 1) % MAX_DECISION_LOG_SIZE;
    }

    logger->decision_count++;
    if (logger->head == logger->tail && logger->decision_count > 0) {
        logger->full = true;
    }

    // Update statistics
    logger->total_decisions++;
    if (record->chosen_action < DECISION_ACTION_COUNT) {
        logger->decisions_by_action[record->chosen_action]++;
    }
    if (record->executed) {
        if (record->succeeded) {
            logger->successful_decisions++;
        } else {
            logger->failed_decisions++;
        }
    }
}

int decision_logger_get_recent(const DecisionLogger* logger,
                               DecisionRecord** out_decisions,
                               int max_decisions) {
    if (!logger || !out_decisions || max_decisions <= 0) return 0;

    int count = 0;
    int items = logger->full ? MAX_DECISION_LOG_SIZE : logger->decision_count;

    // Start from most recent (head - 1) and go backwards
    for (int i = 0; i < items && count < max_decisions; i++) {
        int idx = (logger->head - 1 - i + MAX_DECISION_LOG_SIZE) % MAX_DECISION_LOG_SIZE;
        if (logger->decisions[idx]) {
            out_decisions[count++] = logger->decisions[idx];
        }
    }

    return count;
}

int decision_logger_get_by_entity(const DecisionLogger* logger,
                                  int entity_id,
                                  DecisionRecord** out_decisions,
                                  int max_decisions) {
    if (!logger || !out_decisions || max_decisions <= 0) return 0;

    int count = 0;
    int items = logger->full ? MAX_DECISION_LOG_SIZE : logger->decision_count;

    for (int i = 0; i < items && count < max_decisions; i++) {
        int idx = (logger->head - 1 - i + MAX_DECISION_LOG_SIZE) % MAX_DECISION_LOG_SIZE;
        DecisionRecord* record = logger->decisions[idx];
        if (record && record->entity_id == entity_id) {
            out_decisions[count++] = record;
        }
    }

    return count;
}

int decision_logger_get_by_day(const DecisionLogger* logger,
                               int day,
                               DecisionRecord** out_decisions,
                               int max_decisions) {
    if (!logger || !out_decisions || max_decisions <= 0) return 0;

    int count = 0;
    int items = logger->full ? MAX_DECISION_LOG_SIZE : logger->decision_count;

    for (int i = 0; i < items && count < max_decisions; i++) {
        int idx = (logger->head - 1 - i + MAX_DECISION_LOG_SIZE) % MAX_DECISION_LOG_SIZE;
        DecisionRecord* record = logger->decisions[idx];
        if (record && record->game_day == day) {
            out_decisions[count++] = record;
        }
    }

    return count;
}

int decision_logger_get_by_action(const DecisionLogger* logger,
                                  DecisionAction action,
                                  DecisionRecord** out_decisions,
                                  int max_decisions) {
    if (!logger || !out_decisions || max_decisions <= 0) return 0;

    int count = 0;
    int items = logger->full ? MAX_DECISION_LOG_SIZE : logger->decision_count;

    for (int i = 0; i < items && count < max_decisions; i++) {
        int idx = (logger->head - 1 - i + MAX_DECISION_LOG_SIZE) % MAX_DECISION_LOG_SIZE;
        DecisionRecord* record = logger->decisions[idx];
        if (record && record->chosen_action == action) {
            out_decisions[count++] = record;
        }
    }

    return count;
}

void decision_logger_get_stats(const DecisionLogger* logger,
                              int* total_decisions,
                              int* successful,
                              int* failed,
                              int* by_action) {
    if (!logger) return;

    if (total_decisions) *total_decisions = logger->total_decisions;
    if (successful) *successful = logger->successful_decisions;
    if (failed) *failed = logger->failed_decisions;

    if (by_action) {
        for (int i = 0; i < DECISION_ACTION_COUNT; i++) {
            by_action[i] = logger->decisions_by_action[i];
        }
    }
}

void decision_logger_clear(DecisionLogger* logger) {
    if (!logger) return;

    // Free all records
    for (int i = 0; i < MAX_DECISION_LOG_SIZE; i++) {
        if (logger->decisions[i]) {
            decision_record_destroy(logger->decisions[i]);
            logger->decisions[i] = NULL;
        }
    }

    logger->decision_count = 0;
    logger->head = 0;
    logger->tail = 0;
    logger->full = false;

    // Keep next_decision_id for unique IDs
    // Reset statistics
    logger->total_decisions = 0;
    logger->successful_decisions = 0;
    logger->failed_decisions = 0;

    for (int i = 0; i < DECISION_ACTION_COUNT; i++) {
        logger->decisions_by_action[i] = 0;
    }
}

cJSON* decision_logger_to_json(const DecisionLogger* logger) {
    if (!logger) return NULL;

    cJSON* json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "total_decisions", logger->total_decisions);
    cJSON_AddNumberToObject(json, "successful_decisions", logger->successful_decisions);
    cJSON_AddNumberToObject(json, "failed_decisions", logger->failed_decisions);

    // Recent decisions
    cJSON* decisions_array = cJSON_CreateArray();
    int items = logger->full ? MAX_DECISION_LOG_SIZE : logger->decision_count;
    for (int i = 0; i < items; i++) {
        int idx = (logger->head - 1 - i + MAX_DECISION_LOG_SIZE) % MAX_DECISION_LOG_SIZE;
        if (logger->decisions[idx]) {
            cJSON_AddItemToArray(decisions_array,
                               decision_record_to_json(logger->decisions[idx]));
        }
    }
    cJSON_AddItemToObject(json, "decisions", decisions_array);

    return json;
}
