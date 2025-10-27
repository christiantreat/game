/**
 * component.c
 * Component System Implementation
 */

#include "component.h"
#include "../../lib/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// ============================================================================
// Position Component
// ============================================================================

PositionComponent* position_component_create(const char* location, float x, float y) {
    PositionComponent* comp = (PositionComponent*)malloc(sizeof(PositionComponent));
    if (!comp) return NULL;

    comp->base.type = COMPONENT_POSITION;
    comp->base.entity_id = -1;
    strncpy(comp->location, location, MAX_LOCATION_NAME - 1);
    comp->location[MAX_LOCATION_NAME - 1] = '\0';
    comp->x = x;
    comp->y = y;

    return comp;
}

void position_component_destroy(PositionComponent* comp) {
    free(comp);
}

cJSON* position_component_to_json(const PositionComponent* comp) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "position");
    cJSON_AddNumberToObject(json, "entity_id", comp->base.entity_id);
    cJSON_AddStringToObject(json, "location", comp->location);
    cJSON_AddNumberToObject(json, "x", comp->x);
    cJSON_AddNumberToObject(json, "y", comp->y);
    return json;
}

PositionComponent* position_component_from_json(cJSON* json) {
    cJSON* location = cJSON_GetObjectItem(json, "location");
    cJSON* x = cJSON_GetObjectItem(json, "x");
    cJSON* y = cJSON_GetObjectItem(json, "y");

    PositionComponent* comp = position_component_create(
        location ? location->valuestring : "Unknown",
        x ? (float)x->valuedouble : 0.0f,
        y ? (float)y->valuedouble : 0.0f
    );

    cJSON* entity_id = cJSON_GetObjectItem(json, "entity_id");
    if (entity_id) comp->base.entity_id = entity_id->valueint;

    return comp;
}

// ============================================================================
// Health Component
// ============================================================================

HealthComponent* health_component_create(int current, int maximum) {
    HealthComponent* comp = (HealthComponent*)malloc(sizeof(HealthComponent));
    if (!comp) return NULL;

    comp->base.type = COMPONENT_HEALTH;
    comp->base.entity_id = -1;
    comp->current = current;
    comp->maximum = maximum;

    return comp;
}

void health_component_destroy(HealthComponent* comp) {
    free(comp);
}

bool health_component_is_alive(const HealthComponent* comp) {
    return comp->current > 0;
}

void health_component_damage(HealthComponent* comp, int amount) {
    comp->current -= amount;
    if (comp->current < 0) comp->current = 0;
}

void health_component_heal(HealthComponent* comp, int amount) {
    comp->current += amount;
    if (comp->current > comp->maximum) comp->current = comp->maximum;
}

cJSON* health_component_to_json(const HealthComponent* comp) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "health");
    cJSON_AddNumberToObject(json, "entity_id", comp->base.entity_id);
    cJSON_AddNumberToObject(json, "current", comp->current);
    cJSON_AddNumberToObject(json, "maximum", comp->maximum);
    return json;
}

HealthComponent* health_component_from_json(cJSON* json) {
    cJSON* current = cJSON_GetObjectItem(json, "current");
    cJSON* maximum = cJSON_GetObjectItem(json, "maximum");

    HealthComponent* comp = health_component_create(
        current ? current->valueint : 100,
        maximum ? maximum->valueint : 100
    );

    cJSON* entity_id = cJSON_GetObjectItem(json, "entity_id");
    if (entity_id) comp->base.entity_id = entity_id->valueint;

    return comp;
}

// ============================================================================
// Inventory Component
// ============================================================================

InventoryComponent* inventory_component_create(int capacity) {
    InventoryComponent* comp = (InventoryComponent*)malloc(sizeof(InventoryComponent));
    if (!comp) return NULL;

    comp->base.type = COMPONENT_INVENTORY;
    comp->base.entity_id = -1;
    comp->item_count = 0;
    comp->capacity = capacity;

    return comp;
}

void inventory_component_destroy(InventoryComponent* comp) {
    free(comp);
}

bool inventory_component_add_item(InventoryComponent* comp, const char* item_name, int quantity) {
    // Check if item already exists
    for (int i = 0; i < comp->item_count; i++) {
        if (strcmp(comp->items[i].item_name, item_name) == 0) {
            comp->items[i].quantity += quantity;
            return true;
        }
    }

    // Add new item if we have space
    if (comp->item_count < comp->capacity) {
        strncpy(comp->items[comp->item_count].item_name, item_name, MAX_ITEM_NAME - 1);
        comp->items[comp->item_count].item_name[MAX_ITEM_NAME - 1] = '\0';
        comp->items[comp->item_count].quantity = quantity;
        comp->item_count++;
        return true;
    }

    return false;
}

bool inventory_component_remove_item(InventoryComponent* comp, const char* item_name, int quantity) {
    for (int i = 0; i < comp->item_count; i++) {
        if (strcmp(comp->items[i].item_name, item_name) == 0) {
            if (comp->items[i].quantity >= quantity) {
                comp->items[i].quantity -= quantity;

                // Remove if quantity is 0
                if (comp->items[i].quantity == 0) {
                    // Shift remaining items
                    for (int j = i; j < comp->item_count - 1; j++) {
                        comp->items[j] = comp->items[j + 1];
                    }
                    comp->item_count--;
                }
                return true;
            }
            return false;
        }
    }
    return false;
}

bool inventory_component_has_item(const InventoryComponent* comp, const char* item_name, int quantity) {
    return inventory_component_get_count(comp, item_name) >= quantity;
}

int inventory_component_get_count(const InventoryComponent* comp, const char* item_name) {
    for (int i = 0; i < comp->item_count; i++) {
        if (strcmp(comp->items[i].item_name, item_name) == 0) {
            return comp->items[i].quantity;
        }
    }
    return 0;
}

cJSON* inventory_component_to_json(const InventoryComponent* comp) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "inventory");
    cJSON_AddNumberToObject(json, "entity_id", comp->base.entity_id);
    cJSON_AddNumberToObject(json, "capacity", comp->capacity);

    cJSON* items = cJSON_CreateObject();
    for (int i = 0; i < comp->item_count; i++) {
        cJSON_AddNumberToObject(items, comp->items[i].item_name, comp->items[i].quantity);
    }
    cJSON_AddItemToObject(json, "items", items);

    return json;
}

InventoryComponent* inventory_component_from_json(cJSON* json) {
    cJSON* capacity = cJSON_GetObjectItem(json, "capacity");

    InventoryComponent* comp = inventory_component_create(
        capacity ? capacity->valueint : 20
    );

    cJSON* entity_id = cJSON_GetObjectItem(json, "entity_id");
    if (entity_id) comp->base.entity_id = entity_id->valueint;

    cJSON* items = cJSON_GetObjectItem(json, "items");
    if (items) {
        cJSON* item = NULL;
        cJSON_ArrayForEach(item, items) {
            if (item->string && cJSON_IsNumber(item)) {
                inventory_component_add_item(comp, item->string, item->valueint);
            }
        }
    }

    return comp;
}

// ============================================================================
// Currency Component
// ============================================================================

CurrencyComponent* currency_component_create(int amount) {
    CurrencyComponent* comp = (CurrencyComponent*)malloc(sizeof(CurrencyComponent));
    if (!comp) return NULL;

    comp->base.type = COMPONENT_CURRENCY;
    comp->base.entity_id = -1;
    comp->amount = amount;

    return comp;
}

void currency_component_destroy(CurrencyComponent* comp) {
    free(comp);
}

void currency_component_add(CurrencyComponent* comp, int value) {
    comp->amount += value;
}

bool currency_component_remove(CurrencyComponent* comp, int value) {
    if (comp->amount >= value) {
        comp->amount -= value;
        return true;
    }
    return false;
}

bool currency_component_has(const CurrencyComponent* comp, int value) {
    return comp->amount >= value;
}

cJSON* currency_component_to_json(const CurrencyComponent* comp) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "currency");
    cJSON_AddNumberToObject(json, "entity_id", comp->base.entity_id);
    cJSON_AddNumberToObject(json, "amount", comp->amount);
    return json;
}

CurrencyComponent* currency_component_from_json(cJSON* json) {
    cJSON* amount = cJSON_GetObjectItem(json, "amount");

    CurrencyComponent* comp = currency_component_create(
        amount ? amount->valueint : 0
    );

    cJSON* entity_id = cJSON_GetObjectItem(json, "entity_id");
    if (entity_id) comp->base.entity_id = entity_id->valueint;

    return comp;
}

// ============================================================================
// Relationship Component
// ============================================================================

RelationshipComponent* relationship_component_create(void) {
    RelationshipComponent* comp = (RelationshipComponent*)malloc(sizeof(RelationshipComponent));
    if (!comp) return NULL;

    comp->base.type = COMPONENT_RELATIONSHIP;
    comp->base.entity_id = -1;
    comp->relationship_count = 0;

    return comp;
}

void relationship_component_destroy(RelationshipComponent* comp) {
    free(comp);
}

int relationship_component_get(const RelationshipComponent* comp, int entity_id) {
    for (int i = 0; i < comp->relationship_count; i++) {
        if (comp->relationships[i].entity_id == entity_id) {
            return comp->relationships[i].value;
        }
    }
    return 0;  // Neutral
}

void relationship_component_set(RelationshipComponent* comp, int entity_id, int value) {
    // Clamp value
    if (value < -100) value = -100;
    if (value > 100) value = 100;

    // Check if relationship exists
    for (int i = 0; i < comp->relationship_count; i++) {
        if (comp->relationships[i].entity_id == entity_id) {
            comp->relationships[i].value = value;
            return;
        }
    }

    // Add new relationship
    if (comp->relationship_count < MAX_RELATIONSHIPS) {
        comp->relationships[comp->relationship_count].entity_id = entity_id;
        comp->relationships[comp->relationship_count].value = value;
        comp->relationship_count++;
    }
}

void relationship_component_modify(RelationshipComponent* comp, int entity_id, int delta) {
    int current = relationship_component_get(comp, entity_id);
    relationship_component_set(comp, entity_id, current + delta);
}

const char* relationship_component_get_level(const RelationshipComponent* comp, int entity_id) {
    int value = relationship_component_get(comp, entity_id);

    if (value < -50) return "enemy";
    if (value < -10) return "dislike";
    if (value < 10) return "neutral";
    if (value < 50) return "friendly";
    if (value < 75) return "friend";
    return "close_friend";
}

cJSON* relationship_component_to_json(const RelationshipComponent* comp) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "relationship");
    cJSON_AddNumberToObject(json, "entity_id", comp->base.entity_id);

    cJSON* relationships = cJSON_CreateObject();
    for (int i = 0; i < comp->relationship_count; i++) {
        char key[32];
        snprintf(key, sizeof(key), "%d", comp->relationships[i].entity_id);
        cJSON_AddNumberToObject(relationships, key, comp->relationships[i].value);
    }
    cJSON_AddItemToObject(json, "relationships", relationships);

    return json;
}

RelationshipComponent* relationship_component_from_json(cJSON* json) {
    RelationshipComponent* comp = relationship_component_create();

    cJSON* entity_id = cJSON_GetObjectItem(json, "entity_id");
    if (entity_id) comp->base.entity_id = entity_id->valueint;

    cJSON* relationships = cJSON_GetObjectItem(json, "relationships");
    if (relationships) {
        cJSON* rel = NULL;
        cJSON_ArrayForEach(rel, relationships) {
            if (rel->string && cJSON_IsNumber(rel)) {
                int eid = atoi(rel->string);
                relationship_component_set(comp, eid, rel->valueint);
            }
        }
    }

    return comp;
}

// ============================================================================
// Needs Component
// ============================================================================

NeedsComponent* needs_component_create(void) {
    NeedsComponent* comp = (NeedsComponent*)malloc(sizeof(NeedsComponent));
    if (!comp) return NULL;

    comp->base.type = COMPONENT_NEEDS;
    comp->base.entity_id = -1;
    comp->hunger = 50.0f;
    comp->energy = 100.0f;
    comp->social = 50.0f;

    return comp;
}

void needs_component_destroy(NeedsComponent* comp) {
    free(comp);
}

void needs_component_decay(NeedsComponent* comp, float delta_time) {
    comp->hunger -= delta_time * 5.0f;
    if (comp->hunger < 0.0f) comp->hunger = 0.0f;

    comp->energy -= delta_time * 3.0f;
    if (comp->energy < 0.0f) comp->energy = 0.0f;

    comp->social -= delta_time * 2.0f;
    if (comp->social < 0.0f) comp->social = 0.0f;
}

void needs_component_eat(NeedsComponent* comp, float food_value) {
    comp->hunger += food_value;
    if (comp->hunger > 100.0f) comp->hunger = 100.0f;
}

void needs_component_rest(NeedsComponent* comp, float rest_value) {
    comp->energy += rest_value;
    if (comp->energy > 100.0f) comp->energy = 100.0f;
}

void needs_component_socialize(NeedsComponent* comp, float social_value) {
    comp->social += social_value;
    if (comp->social > 100.0f) comp->social = 100.0f;
}

const char* needs_component_get_most_urgent(const NeedsComponent* comp) {
    float hunger_urgency = 100.0f - comp->hunger;
    float energy_urgency = 100.0f - comp->energy;
    float social_urgency = 100.0f - comp->social;

    if (hunger_urgency >= energy_urgency && hunger_urgency >= social_urgency) {
        return "hunger";
    } else if (energy_urgency >= social_urgency) {
        return "energy";
    } else {
        return "social";
    }
}

cJSON* needs_component_to_json(const NeedsComponent* comp) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "needs");
    cJSON_AddNumberToObject(json, "entity_id", comp->base.entity_id);
    cJSON_AddNumberToObject(json, "hunger", comp->hunger);
    cJSON_AddNumberToObject(json, "energy", comp->energy);
    cJSON_AddNumberToObject(json, "social", comp->social);
    return json;
}

NeedsComponent* needs_component_from_json(cJSON* json) {
    NeedsComponent* comp = needs_component_create();

    cJSON* entity_id = cJSON_GetObjectItem(json, "entity_id");
    if (entity_id) comp->base.entity_id = entity_id->valueint;

    cJSON* hunger = cJSON_GetObjectItem(json, "hunger");
    if (hunger) comp->hunger = (float)hunger->valuedouble;

    cJSON* energy = cJSON_GetObjectItem(json, "energy");
    if (energy) comp->energy = (float)energy->valuedouble;

    cJSON* social = cJSON_GetObjectItem(json, "social");
    if (social) comp->social = (float)social->valuedouble;

    return comp;
}

// ============================================================================
// Schedule Component (Continued in next part...)
// ============================================================================

ScheduleComponent* schedule_component_create(void) {
    ScheduleComponent* comp = (ScheduleComponent*)malloc(sizeof(ScheduleComponent));
    if (!comp) return NULL;

    comp->base.type = COMPONENT_SCHEDULE;
    comp->base.entity_id = -1;
    comp->entry_count = 0;

    return comp;
}

void schedule_component_destroy(ScheduleComponent* comp) {
    free(comp);
}

const char* schedule_component_get_activity(const ScheduleComponent* comp, const char* time_of_day) {
    for (int i = 0; i < comp->entry_count; i++) {
        if (strcmp(comp->entries[i].time_of_day, time_of_day) == 0) {
            return comp->entries[i].activity;
        }
    }
    return NULL;
}

void schedule_component_set_activity(ScheduleComponent* comp, const char* time_of_day, const char* activity) {
    // Check if entry exists
    for (int i = 0; i < comp->entry_count; i++) {
        if (strcmp(comp->entries[i].time_of_day, time_of_day) == 0) {
            strncpy(comp->entries[i].activity, activity, MAX_ACTIVITY_NAME - 1);
            comp->entries[i].activity[MAX_ACTIVITY_NAME - 1] = '\0';
            return;
        }
    }

    // Add new entry
    if (comp->entry_count < MAX_SCHEDULE_ENTRIES) {
        strncpy(comp->entries[comp->entry_count].time_of_day, time_of_day, MAX_TIME_NAME - 1);
        comp->entries[comp->entry_count].time_of_day[MAX_TIME_NAME - 1] = '\0';
        strncpy(comp->entries[comp->entry_count].activity, activity, MAX_ACTIVITY_NAME - 1);
        comp->entries[comp->entry_count].activity[MAX_ACTIVITY_NAME - 1] = '\0';
        comp->entry_count++;
    }
}

cJSON* schedule_component_to_json(const ScheduleComponent* comp) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "schedule");
    cJSON_AddNumberToObject(json, "entity_id", comp->base.entity_id);

    cJSON* schedule = cJSON_CreateObject();
    for (int i = 0; i < comp->entry_count; i++) {
        cJSON_AddStringToObject(schedule, comp->entries[i].time_of_day, comp->entries[i].activity);
    }
    cJSON_AddItemToObject(json, "schedule", schedule);

    return json;
}

ScheduleComponent* schedule_component_from_json(cJSON* json) {
    ScheduleComponent* comp = schedule_component_create();

    cJSON* entity_id = cJSON_GetObjectItem(json, "entity_id");
    if (entity_id) comp->base.entity_id = entity_id->valueint;

    cJSON* schedule = cJSON_GetObjectItem(json, "schedule");
    if (schedule) {
        cJSON* entry = NULL;
        cJSON_ArrayForEach(entry, schedule) {
            if (entry->string && cJSON_IsString(entry)) {
                schedule_component_set_activity(comp, entry->string, entry->valuestring);
            }
        }
    }

    return comp;
}

// ============================================================================
// Occupation Component
// ============================================================================

OccupationComponent* occupation_component_create(const char* occupation, const char* workplace, int skill_level) {
    OccupationComponent* comp = (OccupationComponent*)malloc(sizeof(OccupationComponent));
    if (!comp) return NULL;

    comp->base.type = COMPONENT_OCCUPATION;
    comp->base.entity_id = -1;
    strncpy(comp->occupation, occupation, MAX_OCCUPATION_NAME - 1);
    comp->occupation[MAX_OCCUPATION_NAME - 1] = '\0';
    strncpy(comp->workplace, workplace, MAX_WORKPLACE_NAME - 1);
    comp->workplace[MAX_WORKPLACE_NAME - 1] = '\0';
    comp->skill_level = skill_level;

    return comp;
}

void occupation_component_destroy(OccupationComponent* comp) {
    free(comp);
}

cJSON* occupation_component_to_json(const OccupationComponent* comp) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "occupation");
    cJSON_AddNumberToObject(json, "entity_id", comp->base.entity_id);
    cJSON_AddStringToObject(json, "occupation", comp->occupation);
    cJSON_AddStringToObject(json, "workplace", comp->workplace);
    cJSON_AddNumberToObject(json, "skill_level", comp->skill_level);
    return json;
}

OccupationComponent* occupation_component_from_json(cJSON* json) {
    cJSON* occupation = cJSON_GetObjectItem(json, "occupation");
    cJSON* workplace = cJSON_GetObjectItem(json, "workplace");
    cJSON* skill_level = cJSON_GetObjectItem(json, "skill_level");

    OccupationComponent* comp = occupation_component_create(
        occupation ? occupation->valuestring : "Villager",
        workplace ? workplace->valuestring : "None",
        skill_level ? skill_level->valueint : 1
    );

    cJSON* entity_id = cJSON_GetObjectItem(json, "entity_id");
    if (entity_id) comp->base.entity_id = entity_id->valueint;

    return comp;
}

// ============================================================================
// Memory and Goal Components (simplified for now)
// ============================================================================

MemoryComponent* memory_component_create(int max_memories) {
    MemoryComponent* comp = (MemoryComponent*)malloc(sizeof(MemoryComponent));
    if (!comp) return NULL;

    comp->base.type = COMPONENT_MEMORY;
    comp->base.entity_id = -1;
    comp->memory_count = 0;
    comp->max_memories = max_memories > MAX_MEMORIES ? MAX_MEMORIES : max_memories;

    return comp;
}

void memory_component_destroy(MemoryComponent* comp) {
    free(comp);
}

void memory_component_add(MemoryComponent* comp, const char* text, int day, const char* time_of_day) {
    if (comp->memory_count >= comp->max_memories) {
        // Shift memories to remove oldest
        for (int i = 0; i < comp->max_memories - 1; i++) {
            comp->memories[i] = comp->memories[i + 1];
        }
        comp->memory_count = comp->max_memories - 1;
    }

    strncpy(comp->memories[comp->memory_count].text, text, MAX_MEMORY_TEXT - 1);
    comp->memories[comp->memory_count].text[MAX_MEMORY_TEXT - 1] = '\0';
    comp->memories[comp->memory_count].day = day;
    strncpy(comp->memories[comp->memory_count].time_of_day, time_of_day, MAX_TIME_NAME - 1);
    comp->memories[comp->memory_count].time_of_day[MAX_TIME_NAME - 1] = '\0';
    comp->memory_count++;
}

cJSON* memory_component_to_json(const MemoryComponent* comp) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "memory");
    cJSON_AddNumberToObject(json, "entity_id", comp->base.entity_id);
    cJSON_AddNumberToObject(json, "max_memories", comp->max_memories);

    cJSON* memories = cJSON_CreateArray();
    for (int i = 0; i < comp->memory_count; i++) {
        cJSON* mem = cJSON_CreateObject();
        cJSON_AddStringToObject(mem, "text", comp->memories[i].text);
        cJSON_AddNumberToObject(mem, "day", comp->memories[i].day);
        cJSON_AddStringToObject(mem, "time_of_day", comp->memories[i].time_of_day);
        cJSON_AddItemToArray(memories, mem);
    }
    cJSON_AddItemToObject(json, "memories", memories);

    return json;
}

MemoryComponent* memory_component_from_json(cJSON* json) {
    cJSON* max_memories_json = cJSON_GetObjectItem(json, "max_memories");
    MemoryComponent* comp = memory_component_create(
        max_memories_json ? max_memories_json->valueint : 50
    );

    cJSON* entity_id = cJSON_GetObjectItem(json, "entity_id");
    if (entity_id) comp->base.entity_id = entity_id->valueint;

    cJSON* memories = cJSON_GetObjectItem(json, "memories");
    if (memories) {
        cJSON* mem = NULL;
        cJSON_ArrayForEach(mem, memories) {
            cJSON* text = cJSON_GetObjectItem(mem, "text");
            cJSON* day = cJSON_GetObjectItem(mem, "day");
            cJSON* time_of_day = cJSON_GetObjectItem(mem, "time_of_day");

            if (text && day && time_of_day) {
                memory_component_add(comp, text->valuestring, day->valueint, time_of_day->valuestring);
            }
        }
    }

    return comp;
}

GoalComponent* goal_component_create(void) {
    GoalComponent* comp = (GoalComponent*)malloc(sizeof(GoalComponent));
    if (!comp) return NULL;

    comp->base.type = COMPONENT_GOAL;
    comp->base.entity_id = -1;
    comp->current_goal[0] = '\0';
    comp->goal_count = 0;

    return comp;
}

void goal_component_destroy(GoalComponent* comp) {
    free(comp);
}

void goal_component_set_current(GoalComponent* comp, const char* goal) {
    strncpy(comp->current_goal, goal, MAX_GOAL_TEXT - 1);
    comp->current_goal[MAX_GOAL_TEXT - 1] = '\0';
}

void goal_component_add_goal(GoalComponent* comp, const char* goal) {
    if (comp->goal_count < MAX_GOALS) {
        // Check if goal already exists
        for (int i = 0; i < comp->goal_count; i++) {
            if (strcmp(comp->goals[i], goal) == 0) return;
        }

        strncpy(comp->goals[comp->goal_count], goal, MAX_GOAL_TEXT - 1);
        comp->goals[comp->goal_count][MAX_GOAL_TEXT - 1] = '\0';
        comp->goal_count++;
    }
}

void goal_component_complete(GoalComponent* comp, const char* goal) {
    for (int i = 0; i < comp->goal_count; i++) {
        if (strcmp(comp->goals[i], goal) == 0) {
            // Shift remaining goals
            for (int j = i; j < comp->goal_count - 1; j++) {
                strcpy(comp->goals[j], comp->goals[j + 1]);
            }
            comp->goal_count--;

            // Clear current goal if it matches
            if (strcmp(comp->current_goal, goal) == 0) {
                comp->current_goal[0] = '\0';
            }
            return;
        }
    }
}

cJSON* goal_component_to_json(const GoalComponent* comp) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "goal");
    cJSON_AddNumberToObject(json, "entity_id", comp->base.entity_id);
    cJSON_AddStringToObject(json, "current_goal", comp->current_goal);

    cJSON* goals = cJSON_CreateArray();
    for (int i = 0; i < comp->goal_count; i++) {
        cJSON_AddItemToArray(goals, cJSON_CreateString(comp->goals[i]));
    }
    cJSON_AddItemToObject(json, "goals", goals);

    return json;
}

GoalComponent* goal_component_from_json(cJSON* json) {
    GoalComponent* comp = goal_component_create();

    cJSON* entity_id = cJSON_GetObjectItem(json, "entity_id");
    if (entity_id) comp->base.entity_id = entity_id->valueint;

    cJSON* current_goal = cJSON_GetObjectItem(json, "current_goal");
    if (current_goal && cJSON_IsString(current_goal)) {
        goal_component_set_current(comp, current_goal->valuestring);
    }

    cJSON* goals = cJSON_GetObjectItem(json, "goals");
    if (goals) {
        cJSON* goal = NULL;
        cJSON_ArrayForEach(goal, goals) {
            if (cJSON_IsString(goal)) {
                goal_component_add_goal(comp, goal->valuestring);
            }
        }
    }

    return comp;
}

// ============================================================================
// Component Factory
// ============================================================================

Component* component_from_json(cJSON* json) {
    cJSON* type = cJSON_GetObjectItem(json, "type");
    if (!type || !cJSON_IsString(type)) return NULL;

    const char* type_str = type->valuestring;

    if (strcmp(type_str, "position") == 0) {
        return (Component*)position_component_from_json(json);
    } else if (strcmp(type_str, "health") == 0) {
        return (Component*)health_component_from_json(json);
    } else if (strcmp(type_str, "inventory") == 0) {
        return (Component*)inventory_component_from_json(json);
    } else if (strcmp(type_str, "currency") == 0) {
        return (Component*)currency_component_from_json(json);
    } else if (strcmp(type_str, "relationship") == 0) {
        return (Component*)relationship_component_from_json(json);
    } else if (strcmp(type_str, "needs") == 0) {
        return (Component*)needs_component_from_json(json);
    } else if (strcmp(type_str, "schedule") == 0) {
        return (Component*)schedule_component_from_json(json);
    } else if (strcmp(type_str, "occupation") == 0) {
        return (Component*)occupation_component_from_json(json);
    } else if (strcmp(type_str, "memory") == 0) {
        return (Component*)memory_component_from_json(json);
    } else if (strcmp(type_str, "goal") == 0) {
        return (Component*)goal_component_from_json(json);
    }

    return NULL;
}

void component_destroy(Component* comp) {
    if (!comp) return;

    switch (comp->type) {
        case COMPONENT_POSITION:
            position_component_destroy((PositionComponent*)comp);
            break;
        case COMPONENT_HEALTH:
            health_component_destroy((HealthComponent*)comp);
            break;
        case COMPONENT_INVENTORY:
            inventory_component_destroy((InventoryComponent*)comp);
            break;
        case COMPONENT_CURRENCY:
            currency_component_destroy((CurrencyComponent*)comp);
            break;
        case COMPONENT_RELATIONSHIP:
            relationship_component_destroy((RelationshipComponent*)comp);
            break;
        case COMPONENT_NEEDS:
            needs_component_destroy((NeedsComponent*)comp);
            break;
        case COMPONENT_SCHEDULE:
            schedule_component_destroy((ScheduleComponent*)comp);
            break;
        case COMPONENT_OCCUPATION:
            occupation_component_destroy((OccupationComponent*)comp);
            break;
        case COMPONENT_MEMORY:
            memory_component_destroy((MemoryComponent*)comp);
            break;
        case COMPONENT_GOAL:
            goal_component_destroy((GoalComponent*)comp);
            break;
        default:
            free(comp);
            break;
    }
}

cJSON* component_to_json(const Component* comp) {
    if (!comp) return NULL;

    switch (comp->type) {
        case COMPONENT_POSITION:
            return position_component_to_json((const PositionComponent*)comp);
        case COMPONENT_HEALTH:
            return health_component_to_json((const HealthComponent*)comp);
        case COMPONENT_INVENTORY:
            return inventory_component_to_json((const InventoryComponent*)comp);
        case COMPONENT_CURRENCY:
            return currency_component_to_json((const CurrencyComponent*)comp);
        case COMPONENT_RELATIONSHIP:
            return relationship_component_to_json((const RelationshipComponent*)comp);
        case COMPONENT_NEEDS:
            return needs_component_to_json((const NeedsComponent*)comp);
        case COMPONENT_SCHEDULE:
            return schedule_component_to_json((const ScheduleComponent*)comp);
        case COMPONENT_OCCUPATION:
            return occupation_component_to_json((const OccupationComponent*)comp);
        case COMPONENT_MEMORY:
            return memory_component_to_json((const MemoryComponent*)comp);
        case COMPONENT_GOAL:
            return goal_component_to_json((const GoalComponent*)comp);
        default:
            return NULL;
    }
}

const char* component_type_to_string(ComponentType type) {
    switch (type) {
        case COMPONENT_POSITION: return "position";
        case COMPONENT_HEALTH: return "health";
        case COMPONENT_INVENTORY: return "inventory";
        case COMPONENT_CURRENCY: return "currency";
        case COMPONENT_RELATIONSHIP: return "relationship";
        case COMPONENT_NEEDS: return "needs";
        case COMPONENT_SCHEDULE: return "schedule";
        case COMPONENT_OCCUPATION: return "occupation";
        case COMPONENT_MEMORY: return "memory";
        case COMPONENT_GOAL: return "goal";
        default: return "unknown";
    }
}
