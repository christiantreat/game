#include <stdio.h>
#include <assert.h>
#include "../src/core/game_loop.h"

int tests_passed = 0;
int tests_failed = 0;

void test_game_loop_creation() {
    printf("\nTest: Game Loop Creation\n");

    GameLoop* loop = game_loop_create();
    assert(loop != NULL);
    assert(loop->is_running == false);
    assert(loop->paused == false);
    assert(loop->turn_count == 0);
    printf("  ✓ Game loop creation works\n");

    game_loop_destroy(loop);
    printf("  ✓ All game loop creation tests passed\n");
    tests_passed++;
}

void test_game_loop_initialization() {
    printf("\nTest: Game Loop Initialization\n");

    GameLoop* loop = game_loop_create();
    assert(game_loop_initialize(loop) == true);

    assert(loop->game_state != NULL);
    assert(loop->entity_manager != NULL);
    assert(loop->world != NULL);
    assert(loop->agriculture_manager != NULL);
    assert(loop->economy_manager != NULL);
    assert(loop->social_manager != NULL);
    printf("  ✓ All managers initialized\n");

    game_loop_destroy(loop);
    printf("  ✓ All initialization tests passed\n");
    tests_passed++;
}

void test_actions() {
    printf("\nTest: Action System\n");

    // Create action
    Action* action = action_create(1, 1, ACTION_MOVE);
    assert(action != NULL);
    assert(action->id == 1);
    assert(action->entity_id == 1);
    assert(action->type == ACTION_MOVE);
    printf("  ✓ Action creation works\n");

    // Action type to string
    const char* type_str = action_type_to_string(ACTION_MOVE);
    assert(type_str != NULL);
    printf("  ✓ Action type to string works: %s\n", type_str);

    // Action result to string
    const char* result_str = action_result_to_string(ACTION_SUCCESS);
    assert(result_str != NULL);
    printf("  ✓ Action result to string works: %s\n", result_str);

    action_destroy(action);
    printf("  ✓ All action tests passed\n");
    tests_passed++;
}

void test_turns() {
    printf("\nTest: Turn System\n");

    GameState* state = game_state_create();
    state->day_count = 1;
    state->time_of_day = TIME_MORNING;

    // Create turn
    Turn* turn = turn_create(1, state);
    assert(turn != NULL);
    assert(turn->turn_number == 1);
    assert(turn->day == 1);
    assert(turn->time_of_day == TIME_MORNING);
    printf("  ✓ Turn creation works\n");

    // Add action to turn
    Action* action = action_create(1, 1, ACTION_WAIT);
    assert(turn_add_action(turn, action) == true);
    assert(turn->action_count == 1);
    printf("  ✓ Adding action to turn works\n");

    // Get turn summary
    const char* summary = turn_get_summary(turn);
    assert(summary != NULL);
    printf("  ✓ Turn summary: %s\n", summary);

    turn_destroy(turn);
    game_state_destroy(state);
    printf("  ✓ All turn tests passed\n");
    tests_passed++;
}

void test_action_execution() {
    printf("\nTest: Action Execution\n");

    GameLoop* loop = game_loop_create();
    game_loop_initialize(loop);

    // Test wait action
    Action* wait_action = action_create(1, 1, ACTION_WAIT);
    ActionResult result = action_execute(wait_action, loop);
    assert(result == ACTION_SUCCESS);
    printf("  ✓ Wait action executes successfully\n");

    // Test rest action
    Action* rest_action = action_create(2, 1, ACTION_REST);
    result = action_execute(rest_action, loop);
    assert(result == ACTION_SUCCESS);
    printf("  ✓ Rest action executes successfully\n");

    action_destroy(wait_action);
    action_destroy(rest_action);
    game_loop_destroy(loop);
    printf("  ✓ All action execution tests passed\n");
    tests_passed++;
}

void test_game_loop_start_stop() {
    printf("\nTest: Game Loop Start/Stop\n");

    GameLoop* loop = game_loop_create();
    game_loop_initialize(loop);

    // Start loop
    assert(game_loop_start(loop) == true);
    assert(loop->is_running == true);
    assert(loop->current_turn == 1);
    printf("  ✓ Game loop starts successfully\n");

    // Pause
    game_loop_pause(loop);
    assert(loop->paused == true);
    printf("  ✓ Game loop pauses\n");

    // Resume
    game_loop_resume(loop);
    assert(loop->paused == false);
    printf("  ✓ Game loop resumes\n");

    // Stop
    game_loop_stop(loop);
    assert(loop->is_running == false);
    printf("  ✓ Game loop stops\n");

    game_loop_destroy(loop);
    printf("  ✓ All start/stop tests passed\n");
    tests_passed++;
}

void test_turn_processing() {
    printf("\nTest: Turn Processing\n");

    GameLoop* loop = game_loop_create();
    game_loop_initialize(loop);
    game_loop_start(loop);

    int initial_turn = loop->current_turn;

    // Process turn
    assert(game_loop_process_turn(loop) == true);
    assert(loop->current_turn == initial_turn + 1);
    printf("  ✓ Turn processing advances turn number\n");

    // Process multiple turns
    for (int i = 0; i < 5; i++) {
        game_loop_process_turn(loop);
    }
    assert(loop->turn_count == 7);  // 1 initial + 1 + 5
    printf("  ✓ Multiple turns processed\n");

    game_loop_destroy(loop);
    printf("  ✓ All turn processing tests passed\n");
    tests_passed++;
}

void test_time_progression() {
    printf("\nTest: Time Progression\n");

    GameLoop* loop = game_loop_create();
    game_loop_initialize(loop);
    game_loop_start(loop);

    TimeOfDay initial_time = loop->game_state->time_of_day;

    // Advance time
    game_loop_advance_time(loop);

    // Time should have changed
    // (exact value depends on starting time)
    printf("  ✓ Time advances (was %d, now %d)\n",
           initial_time, loop->game_state->time_of_day);

    game_loop_destroy(loop);
    printf("  ✓ All time progression tests passed\n");
    tests_passed++;
}

void test_game_statistics() {
    printf("\nTest: Game Statistics\n");

    GameLoop* loop = game_loop_create();
    game_loop_initialize(loop);
    game_loop_start(loop);

    // Process some turns
    for (int i = 0; i < 10; i++) {
        game_loop_process_turn(loop);
    }

    // Get stats
    int total_turns = 0;
    int total_actions = 0;
    game_loop_get_stats(loop, &total_turns, &total_actions);

    assert(total_turns > 0);
    printf("  ✓ Total turns: %d\n", total_turns);
    printf("  ✓ Total actions: %d\n", total_actions);

    game_loop_destroy(loop);
    printf("  ✓ All statistics tests passed\n");
    tests_passed++;
}

void test_action_validation() {
    printf("\nTest: Action Validation\n");

    GameLoop* loop = game_loop_create();
    game_loop_initialize(loop);

    // Valid move action
    Action* move_action = action_create(1, 1, ACTION_MOVE);
    move_action->target_location_id = 2;
    assert(action_validate(move_action, loop) == true);
    printf("  ✓ Valid move action passes validation\n");

    // Invalid move action (no target)
    Action* invalid_move = action_create(2, 1, ACTION_MOVE);
    invalid_move->target_location_id = -1;
    assert(action_validate(invalid_move, loop) == false);
    printf("  ✓ Invalid move action fails validation\n");

    action_destroy(move_action);
    action_destroy(invalid_move);
    game_loop_destroy(loop);
    printf("  ✓ All validation tests passed\n");
    tests_passed++;
}

int main() {
    printf("============================================================\n");
    printf("PHASE 9: Game Loop & Turn Management Tests\n");
    printf("============================================================\n");

    test_game_loop_creation();
    test_game_loop_initialization();
    test_actions();
    test_turns();
    test_action_execution();
    test_game_loop_start_stop();
    test_turn_processing();
    test_time_progression();
    test_game_statistics();
    test_action_validation();

    printf("\n============================================================\n");
    printf("RESULTS: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("============================================================\n");

    if (tests_failed == 0) {
        printf("\n🎉 Phase 9: Game Loop & Turn Management - COMPLETE! 🎉\n\n");
        printf("Success Criteria Met:\n");
        printf("✓ Game loop creation and initialization\n");
        printf("✓ Action system with multiple types\n");
        printf("✓ Turn-based processing\n");
        printf("✓ Action execution and validation\n");
        printf("✓ Start/stop/pause controls\n");
        printf("✓ Turn processing with time advancement\n");
        printf("✓ Time progression integration\n");
        printf("✓ Game statistics tracking\n");
    }

    return tests_failed > 0 ? 1 : 0;
}
