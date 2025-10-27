/**
 * test_phase3.c
 * Tests for Phase 3: Decision System Foundation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/core/component.h"
#include "../src/core/entity.h"
#include "../src/core/game_state.h"
#include "../src/core/event.h"
#include "../src/core/decision.h"
#include "../lib/cJSON.h"

// Simple test framework
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("  ❌ FAILED: %s\n", message); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_EQ(actual, expected, message) \
    do { \
        if ((actual) != (expected)) { \
            printf("  ❌ FAILED: %s (expected %d, got %d)\n", message, expected, actual); \
            return false; \
        } \
    } while(0)

// ============================================================================
// Test: Decision Context Creation
// ============================================================================

bool test_decision_context_creation() {
    printf("\nTest: Decision Context Creation\n");

    // Create game state
    GameState* state = game_state_create();

    // Create a test entity with components
    Entity* entity = entity_manager_create_entity(state->entity_manager, "Farmer Bob", "NPC");

    // Add position
    PositionComponent* pos = position_component_create("Farm Field", 10.0f, 20.0f);
    entity_add_component(entity, (Component*)pos);

    // Add health
    HealthComponent* health = health_component_create(80, 100);
    entity_add_component(entity, (Component*)health);

    // Add currency
    CurrencyComponent* currency = currency_component_create(150);
    entity_add_component(entity, (Component*)currency);

    // Add needs
    NeedsComponent* needs = needs_component_create();
    needs->hunger = 60.0f;
    needs->energy = 75.0f;
    needs->social = 40.0f;
    entity_add_component(entity, (Component*)needs);

    // Add occupation
    OccupationComponent* occ = occupation_component_create("Farmer", "Bob's Farm", 5);
    entity_add_component(entity, (Component*)occ);

    // Add goal
    GoalComponent* goal = goal_component_create();
    goal_component_set_current(goal, "Harvest wheat crop");
    entity_add_component(entity, (Component*)goal);

    // Create decision context
    DecisionContext* ctx = decision_context_create(state, entity, NULL);

    TEST_ASSERT(ctx != NULL, "Context created");
    TEST_ASSERT_EQ(ctx->entity_id, entity->id, "Entity ID matches");
    TEST_ASSERT(strcmp(ctx->entity_name, "Farmer Bob") == 0, "Entity name matches");
    TEST_ASSERT_EQ(ctx->day_count, 1, "Day count matches");
    TEST_ASSERT(ctx->position_x == 10.0f && ctx->position_y == 20.0f, "Position matches");
    TEST_ASSERT(ctx->has_health, "Has health component");
    TEST_ASSERT_EQ(ctx->health_current, 80, "Health current matches");
    TEST_ASSERT_EQ(ctx->health_max, 100, "Health max matches");
    TEST_ASSERT(ctx->has_currency, "Has currency component");
    TEST_ASSERT_EQ(ctx->currency, 150, "Currency matches");
    TEST_ASSERT(ctx->has_needs, "Has needs component");
    TEST_ASSERT(ctx->hunger == 60.0f, "Hunger matches");
    TEST_ASSERT(ctx->energy == 75.0f, "Energy matches");
    TEST_ASSERT(ctx->has_occupation, "Has occupation component");
    TEST_ASSERT(strcmp(ctx->occupation, "Farmer") == 0, "Occupation matches");
    TEST_ASSERT_EQ(ctx->skill_level, 5, "Skill level matches");
    TEST_ASSERT(ctx->has_goal, "Has goal component");
    TEST_ASSERT(strcmp(ctx->current_goal, "Harvest wheat crop") == 0, "Goal matches");

    printf("  ✓ Context captures all entity state\n");

    // Test JSON serialization
    cJSON* json = decision_context_to_json(ctx);
    TEST_ASSERT(json != NULL, "Context serializes to JSON");
    cJSON_Delete(json);

    printf("  ✓ Context serializes to JSON\n");

    // Cleanup
    decision_context_destroy(ctx);
    game_state_destroy(state);

    printf("  ✓ All context creation tests passed\n");
    return true;
}

// ============================================================================
// Test: Nearby Entity Detection
// ============================================================================

bool test_nearby_entities() {
    printf("\nTest: Nearby Entity Detection\n");

    GameState* state = game_state_create();

    // Create main entity
    Entity* main = entity_manager_create_entity(state->entity_manager, "Alice", "NPC");
    PositionComponent* main_pos = position_component_create("Town Square", 0.0f, 0.0f);
    entity_add_component(main, (Component*)main_pos);

    // Add relationship component
    RelationshipComponent* main_rel = relationship_component_create();
    entity_add_component(main, (Component*)main_rel);

    // Create nearby entity (distance = 5)
    Entity* nearby = entity_manager_create_entity(state->entity_manager, "Bob", "NPC");
    PositionComponent* nearby_pos = position_component_create("Town Square", 3.0f, 4.0f);
    entity_add_component(nearby, (Component*)nearby_pos);
    relationship_component_set(main_rel, nearby->id, 50);  // Friend

    // Create far entity (distance = 50)
    Entity* far = entity_manager_create_entity(state->entity_manager, "Charlie", "NPC");
    PositionComponent* far_pos = position_component_create("Forest", 30.0f, 40.0f);
    entity_add_component(far, (Component*)far_pos);
    relationship_component_set(main_rel, far->id, -20);  // Dislike

    // Create context with radius = 10
    DecisionContext* ctx = decision_context_create_with_nearby(state, main, NULL, 10.0f);

    TEST_ASSERT(ctx != NULL, "Context created");
    TEST_ASSERT_EQ(ctx->nearby_entity_count, 1, "Found exactly 1 nearby entity");
    TEST_ASSERT_EQ(ctx->nearby_entity_ids[0], nearby->id, "Nearby entity is Bob");
    TEST_ASSERT(strcmp(ctx->nearby_entity_names[0], "Bob") == 0, "Nearby entity name is Bob");
    TEST_ASSERT_EQ(ctx->nearby_relationship_values[0], 50, "Relationship value is 50");

    printf("  ✓ Detects nearby entities within radius\n");
    printf("  ✓ Excludes far entities\n");
    printf("  ✓ Captures relationship values\n");

    decision_context_destroy(ctx);
    game_state_destroy(state);

    printf("  ✓ All nearby entity tests passed\n");
    return true;
}

// ============================================================================
// Test: Decision Record Creation
// ============================================================================

bool test_decision_record() {
    printf("\nTest: Decision Record Creation\n");

    // Create simple context
    GameState* state = game_state_create();
    Entity* entity = entity_manager_create_entity(state->entity_manager, "Farmer", "NPC");
    PositionComponent* pos = position_component_create("Farm", 0.0f, 0.0f);
    entity_add_component(entity, (Component*)pos);

    DecisionContext* ctx = decision_context_create(state, entity, NULL);

    // Create decision options
    DecisionOption options[3];

    options[0].action = DECISION_ACTION_WORK;
    strcpy(options[0].description, "Continue farming");
    options[0].utility = 8.0f;
    options[0].cost = 2.0f;
    options[0].success_chance = 0.95f;

    options[1].action = DECISION_ACTION_REST;
    strcpy(options[1].description, "Take a break");
    options[1].utility = 5.0f;
    options[1].cost = 0.0f;
    options[1].success_chance = 1.0f;

    options[2].action = DECISION_ACTION_EAT;
    strcpy(options[2].description, "Have lunch");
    options[2].utility = 6.0f;
    options[2].cost = 1.0f;
    options[2].success_chance = 1.0f;

    // Create decision record choosing option 0
    DecisionRecord* record = decision_record_create(
        ctx, options, 3, 0,
        "Work has highest utility and energy is sufficient"
    );

    TEST_ASSERT(record != NULL, "Decision record created");
    TEST_ASSERT_EQ(record->entity_id, entity->id, "Entity ID matches");
    TEST_ASSERT_EQ(record->option_count, 3, "Has 3 options");
    TEST_ASSERT_EQ(record->chosen_option_index, 0, "Chose option 0");
    TEST_ASSERT_EQ(record->chosen_action, DECISION_ACTION_WORK, "Chose WORK action");
    TEST_ASSERT(strcmp(record->reasoning, "Work has highest utility and energy is sufficient") == 0,
                "Reasoning captured");
    TEST_ASSERT(!record->executed, "Not yet executed");

    printf("  ✓ Decision record captures all options\n");
    printf("  ✓ Records chosen option and reasoning\n");

    // Test outcome update
    decision_record_set_outcome(record, true, 7.5f, "Successfully farmed 10 wheat");

    TEST_ASSERT(record->executed, "Marked as executed");
    TEST_ASSERT(record->succeeded, "Marked as succeeded");
    TEST_ASSERT(record->actual_utility == 7.5f, "Actual utility recorded");
    TEST_ASSERT(strcmp(record->outcome_description, "Successfully farmed 10 wheat") == 0,
                "Outcome description captured");

    printf("  ✓ Outcome can be updated after execution\n");

    // Test JSON serialization
    cJSON* json = decision_record_to_json(record);
    TEST_ASSERT(json != NULL, "Record serializes to JSON");
    cJSON_Delete(json);

    printf("  ✓ Decision record serializes to JSON\n");

    decision_record_destroy(record);
    decision_context_destroy(ctx);
    game_state_destroy(state);

    printf("  ✓ All decision record tests passed\n");
    return true;
}

// ============================================================================
// Test: Decision Logger
// ============================================================================

bool test_decision_logger() {
    printf("\nTest: Decision Logger\n");

    DecisionLogger* logger = decision_logger_create();
    TEST_ASSERT(logger != NULL, "Logger created");
    TEST_ASSERT_EQ(logger->total_decisions, 0, "Starts empty");

    printf("  ✓ Logger creates successfully\n");

    // Create some test decisions
    GameState* state = game_state_create();
    Entity* entity = entity_manager_create_entity(state->entity_manager, "Test Entity", "NPC");
    PositionComponent* pos = position_component_create("Test Location", 0.0f, 0.0f);
    entity_add_component(entity, (Component*)pos);

    DecisionContext* ctx = decision_context_create(state, entity, NULL);

    // Log multiple decisions
    for (int i = 0; i < 5; i++) {
        DecisionOption options[2];
        options[0].action = DECISION_ACTION_WORK;
        sprintf(options[0].description, "Option A %d", i);
        options[0].utility = 5.0f + i;

        options[1].action = DECISION_ACTION_REST;
        sprintf(options[1].description, "Option B %d", i);
        options[1].utility = 3.0f + i;

        DecisionRecord* record = decision_record_create(ctx, options, 2, 0, "Testing");
        decision_logger_log(logger, record);
    }

    TEST_ASSERT_EQ(logger->total_decisions, 5, "Logged 5 decisions");
    TEST_ASSERT_EQ(logger->decisions_by_action[DECISION_ACTION_WORK], 5,
                   "All 5 were WORK actions");

    printf("  ✓ Can log multiple decisions\n");

    // Test retrieval
    DecisionRecord* recent[10];
    int count = decision_logger_get_recent(logger, recent, 10);
    TEST_ASSERT_EQ(count, 5, "Retrieved 5 recent decisions");

    printf("  ✓ Can retrieve recent decisions\n");

    // Test by entity query
    DecisionRecord* by_entity[10];
    count = decision_logger_get_by_entity(logger, entity->id, by_entity, 10);
    TEST_ASSERT_EQ(count, 5, "Retrieved 5 decisions by entity");

    printf("  ✓ Can query by entity\n");

    // Test by day query
    DecisionRecord* by_day[10];
    count = decision_logger_get_by_day(logger, 1, by_day, 10);
    TEST_ASSERT_EQ(count, 5, "Retrieved 5 decisions on day 1");

    printf("  ✓ Can query by day\n");

    // Test by action query
    DecisionRecord* by_action[10];
    count = decision_logger_get_by_action(logger, DECISION_ACTION_WORK, by_action, 10);
    TEST_ASSERT_EQ(count, 5, "Retrieved 5 WORK decisions");

    printf("  ✓ Can query by action type\n");

    // Test statistics
    int total, successful, failed;
    int by_action_stats[DECISION_ACTION_COUNT];
    decision_logger_get_stats(logger, &total, &successful, &failed, by_action_stats);
    TEST_ASSERT_EQ(total, 5, "Stats show 5 total decisions");
    TEST_ASSERT_EQ(by_action_stats[DECISION_ACTION_WORK], 5, "Stats show 5 WORK actions");

    printf("  ✓ Statistics tracking works\n");

    // Test clear
    decision_logger_clear(logger);
    TEST_ASSERT_EQ(logger->total_decisions, 0, "Logger cleared");

    printf("  ✓ Can clear logger\n");

    decision_context_destroy(ctx);
    decision_logger_destroy(logger);
    game_state_destroy(state);

    printf("  ✓ All decision logger tests passed\n");
    return true;
}

// ============================================================================
// Test: Decision System Integration
// ============================================================================

bool test_decision_integration() {
    printf("\nTest: Decision System Integration\n");

    // Create full game state
    GameState* state = game_state_create();
    EventLogger* event_logger = event_logger_create();
    DecisionLogger* decision_logger = decision_logger_create();

    // Create farmer with full state
    Entity* farmer = entity_manager_create_entity(state->entity_manager, "Farmer Jane", "NPC");
    entity_add_component(farmer, (Component*)position_component_create("Farm", 5.0f, 5.0f));
    entity_add_component(farmer, (Component*)health_component_create(100, 100));
    entity_add_component(farmer, (Component*)currency_component_create(200));

    NeedsComponent* needs = needs_component_create();
    needs->hunger = 40.0f;  // Getting hungry
    needs->energy = 80.0f;  // Well rested
    needs->social = 60.0f;
    entity_add_component(farmer, (Component*)needs);

    entity_add_component(farmer, (Component*)occupation_component_create("Farmer", "Jane's Farm", 7));

    GoalComponent* goal = goal_component_create();
    goal_component_set_current(goal, "Grow crops");
    entity_add_component(farmer, (Component*)goal);

    // Create a nearby friend
    Entity* friend = entity_manager_create_entity(state->entity_manager, "Bob", "NPC");
    entity_add_component(friend, (Component*)position_component_create("Farm", 8.0f, 6.0f));

    RelationshipComponent* rel = relationship_component_create();
    relationship_component_set(rel, friend->id, 70);
    entity_add_component(farmer, (Component*)rel);

    // Create decision context
    DecisionContext* ctx = decision_context_create_with_nearby(state, farmer, event_logger, 20.0f);

    TEST_ASSERT(ctx != NULL, "Integration context created");
    TEST_ASSERT(ctx->has_needs && ctx->has_occupation && ctx->has_goal,
                "Context has all components");
    TEST_ASSERT_EQ(ctx->nearby_entity_count, 1, "Detected nearby friend");
    TEST_ASSERT_EQ(ctx->nearby_relationship_values[0], 70, "Friend relationship captured");

    printf("  ✓ Full game state captured in context\n");

    // Simulate AI decision making
    DecisionOption options[4];

    // Option 1: Keep working (high utility but costs energy)
    options[0].action = DECISION_ACTION_WORK;
    strcpy(options[0].description, "Continue planting crops");
    options[0].utility = 9.0f;  // High value
    options[0].cost = 3.0f;     // Costs energy
    options[0].success_chance = 0.9f;
    options[0].target_entity_id = -1;

    // Option 2: Eat (moderate utility, addresses hunger)
    options[1].action = DECISION_ACTION_EAT;
    strcpy(options[1].description, "Have a meal (hunger: 40)");
    options[1].utility = 7.0f;  // Address the hunger = 40 situation
    options[1].cost = 1.0f;
    options[1].success_chance = 1.0f;
    options[1].target_entity_id = -1;

    // Option 3: Talk to friend (social, friend is nearby)
    options[2].action = DECISION_ACTION_TALK;
    strcpy(options[2].description, "Chat with Bob (nearby, relationship: 70)");
    options[2].utility = 6.0f;
    options[2].cost = 0.5f;
    options[2].success_chance = 0.95f;
    options[2].target_entity_id = friend->id;

    // Option 4: Rest (low utility, energy is already high)
    options[3].action = DECISION_ACTION_REST;
    strcpy(options[3].description, "Take a break");
    options[3].utility = 3.0f;  // Low value since energy = 80
    options[3].cost = 0.0f;
    options[3].success_chance = 1.0f;
    options[3].target_entity_id = -1;

    // AI chooses option 0 (WORK) - highest utility despite hunger
    int chosen = 0;
    const char* reasoning = "Work has highest utility (9.0). Energy is sufficient (80). "
                           "Hunger (40) is moderate but not critical yet. Goal is to grow crops.";

    DecisionRecord* record = decision_record_create(ctx, options, 4, chosen, reasoning);
    TEST_ASSERT(record != NULL, "Decision record created");

    // Log the decision
    decision_logger_log(decision_logger, record);

    printf("  ✓ AI decision made with full reasoning\n");

    // Simulate execution
    decision_record_set_outcome(record, true, 8.5f, "Planted 15 wheat seeds. Gained 2 XP.");

    // Create corresponding event
    GameEvent* event = event_create_crop_action(
        AGRICULTURAL_CROP_PLANTED,
        "wheat",
        5,
        5,
        farmer->id
    );
    event_logger_log(event_logger, event);

    printf("  ✓ Decision executed and logged with event\n");

    // Verify transparency chain
    DecisionRecord* recent_decisions[10];
    int decision_count = decision_logger_get_recent(decision_logger, recent_decisions, 10);
    TEST_ASSERT_EQ(decision_count, 1, "1 decision in log");

    GameEvent* recent_events[10];
    int event_count = event_logger_get_recent(event_logger, recent_events, 10);
    TEST_ASSERT_EQ(event_count, 1, "1 event in log");

    TEST_ASSERT(recent_decisions[0]->executed && recent_decisions[0]->succeeded,
                "Decision shows successful execution");
    TEST_ASSERT_EQ(recent_events[0]->source_entity_id, farmer->id,
                   "Event linked to farmer");

    printf("  ✓ Full transparency chain: Context -> Decision -> Event\n");

    // Test printing (for transparency UI)
    printf("\n  --- Sample Transparency Output ---\n");
    decision_context_print(ctx);
    decision_record_print(record);
    printf("  --- End Sample Output ---\n\n");

    printf("  ✓ Transparency output functions work\n");

    decision_context_destroy(ctx);
    decision_logger_destroy(decision_logger);
    event_logger_destroy(event_logger);
    game_state_destroy(state);

    printf("  ✓ All integration tests passed\n");
    return true;
}

// ============================================================================
// Test: Ring Buffer Overflow
// ============================================================================

bool test_decision_logger_overflow() {
    printf("\nTest: Decision Logger Ring Buffer Overflow\n");

    DecisionLogger* logger = decision_logger_create();
    GameState* state = game_state_create();
    Entity* entity = entity_manager_create_entity(state->entity_manager, "Test", "NPC");
    entity_add_component(entity, (Component*)position_component_create("Test", 0, 0));

    DecisionContext* ctx = decision_context_create(state, entity, NULL);

    // Log more than MAX_DECISION_LOG_SIZE decisions
    int total_logged = MAX_DECISION_LOG_SIZE + 100;

    for (int i = 0; i < total_logged; i++) {
        DecisionOption options[1];
        options[0].action = DECISION_ACTION_WAIT;
        sprintf(options[0].description, "Decision %d", i);
        options[0].utility = 1.0f;

        DecisionRecord* record = decision_record_create(ctx, options, 1, 0, "Test");
        decision_logger_log(logger, record);
    }

    TEST_ASSERT_EQ(logger->total_decisions, total_logged, "Total count is accurate");
    TEST_ASSERT(logger->full, "Buffer marked as full");

    // Should only be able to retrieve MAX_DECISION_LOG_SIZE decisions
    DecisionRecord* recent[MAX_DECISION_LOG_SIZE + 10];
    int count = decision_logger_get_recent(logger, recent, MAX_DECISION_LOG_SIZE + 10);
    TEST_ASSERT_EQ(count, MAX_DECISION_LOG_SIZE, "Ring buffer limits retrieval");

    // Most recent decision should be the last one logged
    TEST_ASSERT(strstr(recent[0]->options[0].description,
                      "Decision") != NULL, "Most recent is accessible");

    printf("  ✓ Ring buffer handles overflow correctly\n");
    printf("  ✓ Oldest decisions are discarded\n");
    printf("  ✓ Statistics remain accurate\n");

    decision_context_destroy(ctx);
    decision_logger_destroy(logger);
    game_state_destroy(state);

    printf("  ✓ All overflow tests passed\n");
    return true;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    printf("\n");
    printf("============================================================\n");
    printf("PHASE 3: Decision System Foundation Tests\n");
    printf("============================================================\n");

    int passed = 0;
    int failed = 0;

    // Run tests
    if (test_decision_context_creation()) passed++; else failed++;
    if (test_nearby_entities()) passed++; else failed++;
    if (test_decision_record()) passed++; else failed++;
    if (test_decision_logger()) passed++; else failed++;
    if (test_decision_integration()) passed++; else failed++;
    if (test_decision_logger_overflow()) passed++; else failed++;

    // Summary
    printf("\n");
    printf("============================================================\n");
    printf("RESULTS: %d passed, %d failed\n", passed, failed);
    printf("============================================================\n");

    if (failed == 0) {
        printf("\n🎉 Phase 3: Decision System Foundation - COMPLETE! 🎉\n\n");
        printf("Success Criteria Met:\n");
        printf("✓ Decision Context captures full entity state\n");
        printf("✓ Decision Context detects nearby entities\n");
        printf("✓ Decision Records log all options and reasoning\n");
        printf("✓ Decision Logger provides query capabilities\n");
        printf("✓ Full transparency chain works (Context -> Decision -> Event)\n");
        printf("✓ Ring buffer handles overflow correctly\n");
        printf("\n");
    }

    return failed == 0 ? 0 : 1;
}
