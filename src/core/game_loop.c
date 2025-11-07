#include "game_loop.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// Game Loop Management
// ============================================================================

GameLoop* game_loop_create(void) {
    GameLoop* loop = calloc(1, sizeof(GameLoop));
    if (!loop) return NULL;

    loop->turn_count = 0;
    loop->current_turn = 0;
    loop->is_running = false;
    loop->paused = false;

    return loop;
}

void game_loop_destroy(GameLoop* loop) {
    if (!loop) return;

    // Destroy turns
    for (int i = 0; i < loop->turn_count; i++) {
        turn_destroy(loop->turns[i]);
    }

    // Note: Managers are owned externally, don't destroy them here

    free(loop);
}

bool game_loop_initialize(GameLoop* loop) {
    if (!loop) return false;

    // Create managers if they don't exist
    if (!loop->game_state) {
        loop->game_state = game_state_create();
        if (!loop->game_state) return false;
    }

    if (!loop->entity_manager) {
        loop->entity_manager = entity_manager_create();
        if (!loop->entity_manager) return false;
    }

    if (!loop->world) {
        loop->world = create_farming_village_world();
        if (!loop->world) return false;
    }

    if (!loop->agriculture_manager) {
        loop->agriculture_manager = agriculture_manager_create();
        if (!loop->agriculture_manager) return false;
        load_default_crop_types(loop->agriculture_manager);
    }

    if (!loop->economy_manager) {
        loop->economy_manager = economy_manager_create();
        if (!loop->economy_manager) return false;
        load_default_item_definitions(loop->economy_manager);
        create_default_shops(loop->economy_manager);
    }

    if (!loop->social_manager) {
        loop->social_manager = social_manager_create();
        if (!loop->social_manager) return false;
        create_default_personalities(loop->social_manager);
        create_default_gift_preferences(loop->social_manager);
    }

    return true;
}

bool game_loop_start(GameLoop* loop) {
    if (!loop) return false;

    loop->is_running = true;
    loop->paused = false;
    loop->current_turn = 0;

    // Create first turn
    Turn* turn = turn_create(1, loop->game_state);
    if (!turn) return false;

    loop->turns[loop->turn_count++] = turn;
    loop->current_turn = 1;

    return true;
}

void game_loop_stop(GameLoop* loop) {
    if (!loop) return;
    loop->is_running = false;
}

void game_loop_pause(GameLoop* loop) {
    if (!loop) return;
    loop->paused = true;
}

void game_loop_resume(GameLoop* loop) {
    if (!loop) return;
    loop->paused = false;
}

bool game_loop_process_turn(GameLoop* loop) {
    if (!loop || !loop->is_running || loop->paused) return false;

    Turn* current = game_loop_get_current_turn(loop);
    if (!current) return false;

    // Process all entities
    game_loop_process_entities(loop);

    // Advance time
    game_loop_advance_time(loop);

    // Update all systems
    game_loop_update_systems(loop);

    // Create next turn
    if (loop->turn_count < MAX_TURN_HISTORY) {
        Turn* next_turn = turn_create(loop->current_turn + 1, loop->game_state);
        if (next_turn) {
            loop->turns[loop->turn_count++] = next_turn;
            loop->current_turn++;
        }
    }

    return true;
}

Turn* game_loop_get_current_turn(const GameLoop* loop) {
    if (!loop || loop->turn_count == 0) return NULL;
    return loop->turns[loop->turn_count - 1];
}

Turn* game_loop_get_turn(const GameLoop* loop, int turn_number) {
    if (!loop || turn_number < 1) return NULL;

    for (int i = 0; i < loop->turn_count; i++) {
        if (loop->turns[i]->turn_number == turn_number) {
            return loop->turns[i];
        }
    }

    return NULL;
}

// ============================================================================
// Action Management
// ============================================================================

Action* action_create(int id, int entity_id, ActionType type) {
    Action* action = calloc(1, sizeof(Action));
    if (!action) return NULL;

    action->id = id;
    action->entity_id = entity_id;
    action->type = type;
    action->target_entity_id = -1;
    action->target_location_id = -1;
    action->target_plot_x = -1;
    action->target_plot_y = -1;
    action->result = ACTION_INVALID;
    action->turn_number = 0;

    return action;
}

void action_destroy(Action* action) {
    free(action);
}

ActionResult action_execute(Action* action, GameLoop* loop) {
    if (!action || !loop) return ACTION_INVALID;

    // Validate first
    if (!action_validate(action, loop)) {
        action->result = ACTION_INVALID;
        strncpy(action->result_message, "Action validation failed", 255);
        return ACTION_INVALID;
    }

    // Execute based on type
    switch (action->type) {
        case ACTION_MOVE:
            return action_execute_move(action, loop);
        case ACTION_TALK:
            return action_execute_talk(action, loop);
        case ACTION_GIFT:
            return action_execute_gift(action, loop);
        case ACTION_PLANT:
            return action_execute_plant(action, loop);
        case ACTION_WATER:
            return action_execute_water(action, loop);
        case ACTION_HARVEST:
            return action_execute_harvest(action, loop);
        case ACTION_BUY:
            return action_execute_buy(action, loop);
        case ACTION_SELL:
            return action_execute_sell(action, loop);
        case ACTION_REST:
        case ACTION_WORK:
        case ACTION_WAIT:
            action->result = ACTION_SUCCESS;
            strncpy(action->result_message, "Completed", 255);
            return ACTION_SUCCESS;
        default:
            action->result = ACTION_INVALID;
            return ACTION_INVALID;
    }
}

ActionResult action_execute_move(Action* action, GameLoop* loop) {
    if (!action || !loop || !loop->world) return ACTION_FAILED;

    // Get entity's current location
    Location* current = world_get_entity_location(loop->world, action->entity_id);
    if (!current) {
        action->result = ACTION_FAILED;
        strncpy(action->result_message, "Entity not in world", 255);
        return ACTION_FAILED;
    }

    // Move to target location
    if (world_move_entity(loop->world, action->entity_id, current->id, action->target_location_id)) {
        action->result = ACTION_SUCCESS;
        snprintf(action->result_message, 255, "Moved to location %d", action->target_location_id);
        return ACTION_SUCCESS;
    }

    action->result = ACTION_FAILED;
    strncpy(action->result_message, "Movement failed", 255);
    return ACTION_FAILED;
}

ActionResult action_execute_talk(Action* action, GameLoop* loop) {
    if (!action || !loop || !loop->social_manager) return ACTION_FAILED;

    if (social_manager_have_conversation(
        loop->social_manager,
        action->entity_id,
        action->target_entity_id,
        TOPIC_WEATHER)) {
        action->result = ACTION_SUCCESS;
        snprintf(action->result_message, 255, "Talked with entity %d", action->target_entity_id);
        return ACTION_SUCCESS;
    }

    action->result = ACTION_FAILED;
    strncpy(action->result_message, "Conversation failed", 255);
    return ACTION_FAILED;
}

ActionResult action_execute_gift(Action* action, GameLoop* loop) {
    if (!action || !loop || !loop->social_manager) return ACTION_FAILED;

    if (social_manager_give_gift(
        loop->social_manager,
        action->entity_id,
        action->target_entity_id,
        action->target_item,
        10)) {
        action->result = ACTION_SUCCESS;
        snprintf(action->result_message, 255, "Gave %s to entity %d",
                 action->target_item, action->target_entity_id);
        return ACTION_SUCCESS;
    }

    action->result = ACTION_FAILED;
    strncpy(action->result_message, "Gift giving failed", 255);
    return ACTION_FAILED;
}

ActionResult action_execute_plant(Action* action, GameLoop* loop) {
    if (!action || !loop || !loop->agriculture_manager) return ACTION_FAILED;

    // Simplified planting (would normally check inventory for seeds)
    action->result = ACTION_SUCCESS;
    snprintf(action->result_message, 255, "Planted at (%d, %d)",
             action->target_plot_x, action->target_plot_y);
    return ACTION_SUCCESS;
}

ActionResult action_execute_water(Action* action, GameLoop* loop) {
    if (!action || !loop || !loop->agriculture_manager) return ACTION_FAILED;

    action->result = ACTION_SUCCESS;
    strncpy(action->result_message, "Watered crops", 255);
    return ACTION_SUCCESS;
}

ActionResult action_execute_harvest(Action* action, GameLoop* loop) {
    if (!action || !loop || !loop->agriculture_manager) return ACTION_FAILED;

    action->result = ACTION_SUCCESS;
    strncpy(action->result_message, "Harvested crops", 255);
    return ACTION_SUCCESS;
}

ActionResult action_execute_buy(Action* action, GameLoop* loop) {
    if (!action || !loop || !loop->economy_manager) return ACTION_FAILED;

    action->result = ACTION_SUCCESS;
    snprintf(action->result_message, 255, "Bought %s", action->target_item);
    return ACTION_SUCCESS;
}

ActionResult action_execute_sell(Action* action, GameLoop* loop) {
    if (!action || !loop || !loop->economy_manager) return ACTION_FAILED;

    action->result = ACTION_SUCCESS;
    snprintf(action->result_message, 255, "Sold %s", action->target_item);
    return ACTION_SUCCESS;
}

bool action_validate(const Action* action, const GameLoop* loop) {
    if (!action || !loop) return false;

    // Basic validation
    if (action->entity_id < 0) return false;

    // Type-specific validation
    switch (action->type) {
        case ACTION_MOVE:
            return action->target_location_id >= 0;
        case ACTION_TALK:
        case ACTION_GIFT:
            return action->target_entity_id >= 0;
        case ACTION_PLANT:
        case ACTION_WATER:
            return action->target_plot_x >= 0 && action->target_plot_y >= 0;
        default:
            return true;
    }
}

// ============================================================================
// Turn Management
// ============================================================================

Turn* turn_create(int turn_number, const GameState* state) {
    Turn* turn = calloc(1, sizeof(Turn));
    if (!turn) return NULL;

    turn->turn_number = turn_number;
    turn->action_count = 0;
    turn->entities_acted = 0;

    if (state) {
        turn->time_of_day = state->time_of_day;
        turn->season = state->season;
        turn->weather = state->current_weather;
        turn->day = state->day_count;
    }

    return turn;
}

void turn_destroy(Turn* turn) {
    if (!turn) return;

    for (int i = 0; i < turn->action_count; i++) {
        action_destroy(turn->actions[i]);
    }

    free(turn);
}

bool turn_add_action(Turn* turn, Action* action) {
    if (!turn || !action) return false;
    if (turn->action_count >= MAX_ACTIONS_PER_TURN) return false;

    action->turn_number = turn->turn_number;
    turn->actions[turn->action_count++] = action;

    return true;
}

const char* turn_get_summary(const Turn* turn) {
    static char summary[512];

    if (!turn) {
        return "No turn data";
    }

    snprintf(summary, 511, "Turn %d: Day %d, %s, %d actions",
             turn->turn_number, turn->day,
             time_of_day_to_string(turn->time_of_day),
             turn->action_count);

    return summary;
}

// ============================================================================
// Entity AI Processing
// ============================================================================

void game_loop_process_entities(GameLoop* loop) {
    if (!loop || !loop->entity_manager) return;

    Turn* turn = game_loop_get_current_turn(loop);
    if (!turn) return;

    // Process each entity
    for (int i = 0; i < loop->entity_manager->entity_count; i++) {
        Entity* entity = loop->entity_manager->entities[i];
        if (entity) {
            game_loop_process_entity(loop, entity);
            turn->entities_acted++;
        }
    }
}

void game_loop_process_entity(GameLoop* loop, Entity* entity) {
    if (!loop || !entity) return;

    // Let entity decide action
    Action* action = entity_decide_action(entity, loop);
    if (!action) return;

    // Execute action
    action_execute(action, loop);

    // Add to turn
    Turn* turn = game_loop_get_current_turn(loop);
    if (turn) {
        turn_add_action(turn, action);
    }
}

Action* entity_decide_action(Entity* entity, GameLoop* loop) {
    if (!entity || !loop) return NULL;

    // Simple decision: just wait
    // In a full implementation, this would use the behavior tree
    Action* action = action_create(0, entity->id, ACTION_WAIT);
    return action;
}

// ============================================================================
// Time Progression
// ============================================================================

void game_loop_advance_time(GameLoop* loop) {
    if (!loop || !loop->game_state) return;

    // Advance time period
    time_advance_period(loop->game_state, loop->agriculture_manager);
}

void game_loop_update_systems(GameLoop* loop) {
    if (!loop) return;

    // Update agriculture (crop growth)
    if (loop->agriculture_manager && loop->game_state) {
        // Crops update happens in time_advance_period
    }

    // Update social (relationship decay)
    if (loop->social_manager) {
        // Apply decay once per day
        if (loop->game_state && loop->game_state->time_of_day == TIME_MORNING) {
            social_manager_update_all(loop->social_manager, 1);
        }
    }
}

// ============================================================================
// Utility
// ============================================================================

const char* action_type_to_string(ActionType type) {
    switch (type) {
        case ACTION_MOVE: return "Move";
        case ACTION_TALK: return "Talk";
        case ACTION_GIFT: return "Gift";
        case ACTION_PLANT: return "Plant";
        case ACTION_WATER: return "Water";
        case ACTION_HARVEST: return "Harvest";
        case ACTION_BUY: return "Buy";
        case ACTION_SELL: return "Sell";
        case ACTION_REST: return "Rest";
        case ACTION_WORK: return "Work";
        case ACTION_WAIT: return "Wait";
        default: return "Unknown";
    }
}

const char* action_result_to_string(ActionResult result) {
    switch (result) {
        case ACTION_SUCCESS: return "Success";
        case ACTION_FAILED: return "Failed";
        case ACTION_INVALID: return "Invalid";
        case ACTION_BLOCKED: return "Blocked";
        default: return "Unknown";
    }
}

void game_loop_print_turn_summary(const GameLoop* loop) {
    if (!loop) return;

    Turn* turn = game_loop_get_current_turn(loop);
    if (!turn) return;

    printf("=== %s ===\n", turn_get_summary(turn));
    printf("Actions:\n");

    for (int i = 0; i < turn->action_count; i++) {
        Action* action = turn->actions[i];
        printf("  Entity %d: %s - %s (%s)\n",
               action->entity_id,
               action_type_to_string(action->type),
               action_result_to_string(action->result),
               action->result_message);
    }
}

void game_loop_get_stats(const GameLoop* loop, int* total_turns, int* total_actions) {
    if (!loop) return;

    if (total_turns) *total_turns = loop->turn_count;

    if (total_actions) {
        int count = 0;
        for (int i = 0; i < loop->turn_count; i++) {
            count += loop->turns[i]->action_count;
        }
        *total_actions = count;
    }
}
