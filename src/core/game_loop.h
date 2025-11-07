#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include <stdbool.h>
#include <stdint.h>
#include "game_state.h"
#include "entity.h"
#include "world.h"
#include "agriculture.h"
#include "economy.h"
#include "social.h"
#include "behavior.h"

/*
 * Phase 9: Game Loop & Turn Management
 *
 * Brings together all systems into a cohesive turn-based game loop.
 * All actions are transparent and logged.
 */

#define MAX_ACTIONS_PER_TURN 100
#define MAX_TURN_HISTORY 1000

// Action types
typedef enum {
    ACTION_MOVE,
    ACTION_TALK,
    ACTION_GIFT,
    ACTION_PLANT,
    ACTION_WATER,
    ACTION_HARVEST,
    ACTION_BUY,
    ACTION_SELL,
    ACTION_REST,
    ACTION_WORK,
    ACTION_WAIT
} ActionType;

// Action result
typedef enum {
    ACTION_SUCCESS,
    ACTION_FAILED,
    ACTION_INVALID,
    ACTION_BLOCKED
} ActionResult;

// Action
typedef struct {
    int id;
    int entity_id;
    ActionType type;
    int target_entity_id;  // For social actions
    int target_location_id;  // For movement
    char target_item[64];   // For economy actions
    int target_plot_x;      // For farming
    int target_plot_y;
    ActionResult result;
    char result_message[256];
    int turn_number;
} Action;

// Turn
typedef struct {
    int turn_number;
    TimeOfDay time_of_day;
    Season season;
    Weather weather;
    int day;
    Action* actions[MAX_ACTIONS_PER_TURN];
    int action_count;
    int entities_acted;
} Turn;

// Game loop state
typedef struct {
    GameState* game_state;
    EntityManager* entity_manager;
    World* world;
    AgricultureManager* agriculture_manager;
    EconomyManager* economy_manager;
    SocialManager* social_manager;

    Turn* turns[MAX_TURN_HISTORY];
    int turn_count;
    int current_turn;

    bool is_running;
    bool paused;
} GameLoop;

// ============================================================================
// Game Loop Management
// ============================================================================

GameLoop* game_loop_create(void);
void game_loop_destroy(GameLoop* loop);

// Initialize all managers
bool game_loop_initialize(GameLoop* loop);

// Start/stop
bool game_loop_start(GameLoop* loop);
void game_loop_stop(GameLoop* loop);
void game_loop_pause(GameLoop* loop);
void game_loop_resume(GameLoop* loop);

// Process single turn
bool game_loop_process_turn(GameLoop* loop);

// Get current turn
Turn* game_loop_get_current_turn(const GameLoop* loop);

// Get turn history
Turn* game_loop_get_turn(const GameLoop* loop, int turn_number);

// ============================================================================
// Action Management
// ============================================================================

Action* action_create(int id, int entity_id, ActionType type);
void action_destroy(Action* action);

// Execute action
ActionResult action_execute(Action* action, GameLoop* loop);

// Specific action executors
ActionResult action_execute_move(Action* action, GameLoop* loop);
ActionResult action_execute_talk(Action* action, GameLoop* loop);
ActionResult action_execute_gift(Action* action, GameLoop* loop);
ActionResult action_execute_plant(Action* action, GameLoop* loop);
ActionResult action_execute_water(Action* action, GameLoop* loop);
ActionResult action_execute_harvest(Action* action, GameLoop* loop);
ActionResult action_execute_buy(Action* action, GameLoop* loop);
ActionResult action_execute_sell(Action* action, GameLoop* loop);

// Validate action
bool action_validate(const Action* action, const GameLoop* loop);

// ============================================================================
// Turn Management
// ============================================================================

Turn* turn_create(int turn_number, const GameState* state);
void turn_destroy(Turn* turn);

// Add action to turn
bool turn_add_action(Turn* turn, Action* action);

// Get turn summary
const char* turn_get_summary(const Turn* turn);

// ============================================================================
// Entity AI Processing
// ============================================================================

// Process all entity decisions for the turn
void game_loop_process_entities(GameLoop* loop);

// Process single entity
void game_loop_process_entity(GameLoop* loop, Entity* entity);

// Let entity decide action using behavior tree
Action* entity_decide_action(Entity* entity, GameLoop* loop);

// ============================================================================
// Time Progression
// ============================================================================

// Advance time (called after all entities act)
void game_loop_advance_time(GameLoop* loop);

// Update all time-dependent systems
void game_loop_update_systems(GameLoop* loop);

// ============================================================================
// Utility
// ============================================================================

// Get action type name
const char* action_type_to_string(ActionType type);

// Get action result name
const char* action_result_to_string(ActionResult result);

// Print turn summary
void game_loop_print_turn_summary(const GameLoop* loop);

// Get game statistics
void game_loop_get_stats(const GameLoop* loop, int* total_turns, int* total_actions);

#endif // GAME_LOOP_H
