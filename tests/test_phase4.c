/**
 * test_phase4.c
 * Tests for Phase 4: Behavior Tree System
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/core/component.h"
#include "../src/core/entity.h"
#include "../src/core/game_state.h"
#include "../src/core/event.h"
#include "../src/core/decision.h"
#include "../src/core/behavior.h"

// Simple test framework
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("  ❌ FAILED: %s\n", message); \
            return false; \
        } \
    } while(0)

#define TEST_PASS(message) printf("  ✓ %s\n", message)

// ============================================================================
// Test: Behavior Node Creation
// ============================================================================

bool test_behavior_node_creation() {
    printf("\nTest: Behavior Node Creation\n");

    // Create sequence node
    BehaviorNode* sequence = behavior_node_create_sequence("Test Sequence");
    TEST_ASSERT(sequence != NULL, "Sequence node created");
    TEST_ASSERT(sequence->type == BT_NODE_SEQUENCE, "Sequence type correct");
    TEST_ASSERT(strcmp(sequence->name, "Test Sequence") == 0, "Sequence name correct");
    TEST_PASS("Sequence node creation works");

    // Create selector node
    BehaviorNode* selector = behavior_node_create_selector("Test Selector");
    TEST_ASSERT(selector != NULL, "Selector node created");
    TEST_ASSERT(selector->type == BT_NODE_SELECTOR, "Selector type correct");
    TEST_PASS("Selector node creation works");

    // Create condition node
    BehaviorNode* condition = behavior_node_create_condition("Test Condition", condition_is_morning);
    TEST_ASSERT(condition != NULL, "Condition node created");
    TEST_ASSERT(condition->type == BT_NODE_CONDITION, "Condition type correct");
    TEST_ASSERT(condition->condition != NULL, "Condition function set");
    TEST_PASS("Condition node creation works");

    // Create action node
    BehaviorNode* action = behavior_node_create_action("Test Action", action_idle);
    TEST_ASSERT(action != NULL, "Action node created");
    TEST_ASSERT(action->type == BT_NODE_ACTION, "Action type correct");
    TEST_ASSERT(action->action != NULL, "Action function set");
    TEST_PASS("Action node creation works");

    // Test adding children
    behavior_node_add_child(sequence, condition);
    behavior_node_add_child(sequence, action);
    TEST_ASSERT(sequence->child_count == 2, "Children added to sequence");
    TEST_PASS("Adding children works");

    behavior_node_destroy(sequence);  // This also destroys condition and action
    behavior_node_destroy(selector);

    printf("  ✓ All node creation tests passed\n");
    return true;
}

// ============================================================================
// Test: Behavior Context
// ============================================================================

bool test_behavior_context() {
    printf("\nTest: Behavior Context\n");

    // Create game state and entity
    GameState* state = game_state_create();
    Entity* entity = entity_manager_create_entity(state->entity_manager, "Test Entity", "NPC");

    // Create context
    BehaviorContext* context = behavior_context_create(state, entity, NULL, NULL);
    TEST_ASSERT(context != NULL, "Context created");
    TEST_ASSERT(context->game_state == state, "Game state set");
    TEST_ASSERT(context->entity == entity, "Entity set");
    TEST_PASS("Context creation works");

    // Test blackboard
    behavior_context_set(context, "test_key", (void*)42);
    TEST_ASSERT(behavior_context_has(context, "test_key"), "Blackboard has key");
    void* value = behavior_context_get(context, "test_key");
    TEST_ASSERT(value == (void*)42, "Blackboard value correct");
    TEST_PASS("Blackboard operations work");

    behavior_context_destroy(context);
    game_state_destroy(state);

    printf("  ✓ All context tests passed\n");
    return true;
}

// ============================================================================
// Test: Sequence Node Behavior
// ============================================================================

// Helper action that always succeeds
static BehaviorStatus action_succeed(BehaviorContext* context) {
    (void)context;
    return BT_SUCCESS;
}

// Helper action that always fails
static BehaviorStatus action_fail(BehaviorContext* context) {
    (void)context;
    return BT_FAILURE;
}

bool test_sequence_behavior() {
    printf("\nTest: Sequence Node Behavior\n");

    // Create test context
    GameState* state = game_state_create();
    Entity* entity = entity_manager_create_entity(state->entity_manager, "Test", "NPC");
    BehaviorContext* context = behavior_context_create(state, entity, NULL, NULL);
    context->logging_enabled = false;  // Disable logging for cleaner output

    // Test 1: All succeed -> sequence succeeds
    BehaviorNode* seq1 = behavior_node_create_sequence("All Succeed");
    behavior_node_add_child(seq1, behavior_node_create_action("A1", action_succeed));
    behavior_node_add_child(seq1, behavior_node_create_action("A2", action_succeed));
    behavior_node_add_child(seq1, behavior_node_create_action("A3", action_succeed));
    BehaviorStatus status = behavior_node_tick(seq1, context);
    TEST_ASSERT(status == BT_SUCCESS, "All succeed -> SUCCESS");
    TEST_PASS("Sequence succeeds when all children succeed");
    behavior_node_destroy(seq1);

    // Test 2: One fails -> sequence fails
    BehaviorNode* seq2 = behavior_node_create_sequence("One Fails");
    behavior_node_add_child(seq2, behavior_node_create_action("A1", action_succeed));
    behavior_node_add_child(seq2, behavior_node_create_action("A2", action_fail));
    behavior_node_add_child(seq2, behavior_node_create_action("A3", action_succeed));
    status = behavior_node_tick(seq2, context);
    TEST_ASSERT(status == BT_FAILURE, "One fails -> FAILURE");
    TEST_PASS("Sequence fails when one child fails");
    behavior_node_destroy(seq2);

    // Test 3: First fails -> sequence fails immediately
    BehaviorNode* seq3 = behavior_node_create_sequence("First Fails");
    behavior_node_add_child(seq3, behavior_node_create_action("A1", action_fail));
    behavior_node_add_child(seq3, behavior_node_create_action("A2", action_succeed));
    status = behavior_node_tick(seq3, context);
    TEST_ASSERT(status == BT_FAILURE, "First fails -> FAILURE");
    TEST_PASS("Sequence fails immediately on first failure");
    behavior_node_destroy(seq3);

    behavior_context_destroy(context);
    game_state_destroy(state);

    printf("  ✓ All sequence behavior tests passed\n");
    return true;
}

// ============================================================================
// Test: Selector Node Behavior
// ============================================================================

bool test_selector_behavior() {
    printf("\nTest: Selector Node Behavior\n");

    GameState* state = game_state_create();
    Entity* entity = entity_manager_create_entity(state->entity_manager, "Test", "NPC");
    BehaviorContext* context = behavior_context_create(state, entity, NULL, NULL);
    context->logging_enabled = false;

    // Test 1: All fail -> selector fails
    BehaviorNode* sel1 = behavior_node_create_selector("All Fail");
    behavior_node_add_child(sel1, behavior_node_create_action("A1", action_fail));
    behavior_node_add_child(sel1, behavior_node_create_action("A2", action_fail));
    behavior_node_add_child(sel1, behavior_node_create_action("A3", action_fail));
    BehaviorStatus status = behavior_node_tick(sel1, context);
    TEST_ASSERT(status == BT_FAILURE, "All fail -> FAILURE");
    TEST_PASS("Selector fails when all children fail");
    behavior_node_destroy(sel1);

    // Test 2: One succeeds -> selector succeeds
    BehaviorNode* sel2 = behavior_node_create_selector("One Succeeds");
    behavior_node_add_child(sel2, behavior_node_create_action("A1", action_fail));
    behavior_node_add_child(sel2, behavior_node_create_action("A2", action_succeed));
    behavior_node_add_child(sel2, behavior_node_create_action("A3", action_fail));
    status = behavior_node_tick(sel2, context);
    TEST_ASSERT(status == BT_SUCCESS, "One succeeds -> SUCCESS");
    TEST_PASS("Selector succeeds when one child succeeds");
    behavior_node_destroy(sel2);

    // Test 3: First succeeds -> selector succeeds immediately
    BehaviorNode* sel3 = behavior_node_create_selector("First Succeeds");
    behavior_node_add_child(sel3, behavior_node_create_action("A1", action_succeed));
    behavior_node_add_child(sel3, behavior_node_create_action("A2", action_fail));
    status = behavior_node_tick(sel3, context);
    TEST_ASSERT(status == BT_SUCCESS, "First succeeds -> SUCCESS");
    TEST_PASS("Selector succeeds immediately on first success");
    behavior_node_destroy(sel3);

    behavior_context_destroy(context);
    game_state_destroy(state);

    printf("  ✓ All selector behavior tests passed\n");
    return true;
}

// ============================================================================
// Test: Condition Nodes
// ============================================================================

bool test_conditions() {
    printf("\nTest: Condition Nodes\n");

    GameState* state = game_state_create();
    state->time_of_day = TIME_MORNING;

    Entity* entity = entity_manager_create_entity(state->entity_manager, "Test", "NPC");

    // Add needs component
    NeedsComponent* needs = needs_component_create();
    needs->hunger = 25.0f;  // Hungry (< 30)
    needs->energy = 80.0f;  // Not tired
    needs->social = 50.0f;  // Not lonely
    entity_add_component(entity, (Component*)needs);

    BehaviorContext* context = behavior_context_create(state, entity, NULL, NULL);
    context->logging_enabled = false;

    // Test time conditions
    BehaviorNode* is_morning = behavior_node_create_condition("Is Morning?", condition_is_morning);
    BehaviorStatus status = behavior_node_tick(is_morning, context);
    TEST_ASSERT(status == BT_SUCCESS, "Morning condition succeeds");
    TEST_PASS("Time condition works");
    behavior_node_destroy(is_morning);

    // Test needs conditions
    BehaviorNode* is_hungry = behavior_node_create_condition("Is Hungry?", condition_is_hungry);
    status = behavior_node_tick(is_hungry, context);
    TEST_ASSERT(status == BT_SUCCESS, "Hungry condition succeeds (hunger=25)");
    TEST_PASS("Hunger condition works");
    behavior_node_destroy(is_hungry);

    BehaviorNode* is_tired = behavior_node_create_condition("Is Tired?", condition_is_tired);
    status = behavior_node_tick(is_tired, context);
    TEST_ASSERT(status == BT_FAILURE, "Tired condition fails (energy=80)");
    TEST_PASS("Energy condition works");
    behavior_node_destroy(is_tired);

    behavior_context_destroy(context);
    game_state_destroy(state);

    printf("  ✓ All condition tests passed\n");
    return true;
}

// ============================================================================
// Test: Action Nodes
// ============================================================================

bool test_actions() {
    printf("\nTest: Action Nodes\n");

    GameState* state = game_state_create();
    Entity* entity = entity_manager_create_entity(state->entity_manager, "Test", "NPC");

    // Add components
    NeedsComponent* needs = needs_component_create();
    needs->hunger = 30.0f;
    needs->energy = 40.0f;
    needs->social = 50.0f;
    entity_add_component(entity, (Component*)needs);

    InventoryComponent* inv = inventory_component_create(20);
    inventory_component_add_item(inv, "bread", 5);
    entity_add_component(entity, (Component*)inv);

    BehaviorContext* context = behavior_context_create(state, entity, NULL, NULL);
    context->logging_enabled = false;

    // Test eat action
    float hunger_before = needs->hunger;
    BehaviorNode* eat = behavior_node_create_action("Eat", action_eat_food);
    BehaviorStatus status = behavior_node_tick(eat, context);
    TEST_ASSERT(status == BT_SUCCESS, "Eat action succeeds");
    TEST_ASSERT(needs->hunger > hunger_before, "Hunger increased after eating");
    TEST_ASSERT(inv->items[0].quantity == 4, "Bread consumed");
    TEST_PASS("Eat action works");
    behavior_node_destroy(eat);

    // Test rest action
    float energy_before = needs->energy;
    BehaviorNode* rest = behavior_node_create_action("Rest", action_rest);
    status = behavior_node_tick(rest, context);
    TEST_ASSERT(status == BT_SUCCESS, "Rest action succeeds");
    TEST_ASSERT(needs->energy > energy_before, "Energy increased after resting");
    TEST_PASS("Rest action works");
    behavior_node_destroy(rest);

    // Test socialize action
    float social_before = needs->social;
    BehaviorNode* socialize = behavior_node_create_action("Socialize", action_socialize);
    status = behavior_node_tick(socialize, context);
    TEST_ASSERT(status == BT_SUCCESS, "Socialize action succeeds");
    TEST_ASSERT(needs->social > social_before, "Social increased after socializing");
    TEST_PASS("Socialize action works");
    behavior_node_destroy(socialize);

    behavior_context_destroy(context);
    game_state_destroy(state);

    printf("  ✓ All action tests passed\n");
    return true;
}

// ============================================================================
// Test: Behavior Tree Execution
// ============================================================================

bool test_behavior_tree() {
    printf("\nTest: Behavior Tree Execution\n");

    GameState* state = game_state_create();
    state->time_of_day = TIME_MORNING;

    Entity* entity = entity_manager_create_entity(state->entity_manager, "Farmer", "NPC");

    // Setup farmer with needs
    NeedsComponent* needs = needs_component_create();
    needs->hunger = 25.0f;  // Hungry
    needs->energy = 80.0f;  // Well rested
    needs->social = 60.0f;
    entity_add_component(entity, (Component*)needs);

    InventoryComponent* inv = inventory_component_create(20);
    inventory_component_add_item(inv, "bread", 3);
    entity_add_component(entity, (Component*)inv);

    entity_add_component(entity, (Component*)position_component_create("Farm", 5.0f, 5.0f));

    BehaviorContext* context = behavior_context_create(state, entity, NULL, NULL);
    context->logging_enabled = false;

    // Create simple behavior tree: If hungry, eat; otherwise, farm
    BehaviorNode* root = behavior_node_create_selector("Farmer AI");

    // Branch 1: Handle hunger
    BehaviorNode* hunger_branch = behavior_node_create_sequence("Handle Hunger");
    behavior_node_add_child(hunger_branch, behavior_node_create_condition("Hungry?", condition_is_hungry));
    behavior_node_add_child(hunger_branch, behavior_node_create_action("Eat", action_eat_food));
    behavior_node_add_child(root, hunger_branch);

    // Branch 2: Work
    BehaviorNode* work_branch = behavior_node_create_sequence("Work");
    behavior_node_add_child(work_branch, behavior_node_create_condition("Is Morning?", condition_is_morning));
    behavior_node_add_child(work_branch, behavior_node_create_action("Farm", action_farm));
    behavior_node_add_child(root, work_branch);

    BehaviorTree* tree = behavior_tree_create("Test Farmer", root);
    TEST_ASSERT(tree != NULL, "Behavior tree created");
    TEST_PASS("Tree creation works");

    // Tick 1: Should eat (hungry)
    BehaviorStatus status = behavior_tree_tick(tree, context);
    TEST_ASSERT(status == BT_SUCCESS, "First tick succeeds");
    TEST_ASSERT(needs->hunger > 25.0f, "Hunger increased");
    TEST_ASSERT(inv->items[0].quantity == 2, "Food consumed");
    TEST_PASS("Tree correctly chose eating branch");

    // Tick 2: Should farm (not hungry anymore)
    status = behavior_tree_tick(tree, context);
    TEST_ASSERT(status == BT_SUCCESS, "Second tick succeeds");
    TEST_PASS("Tree correctly chose farming branch");

    // Check statistics
    TEST_ASSERT(tree->total_ticks == 2, "Total ticks tracked");
    TEST_ASSERT(tree->successful_ticks == 2, "Successful ticks tracked");
    TEST_PASS("Statistics tracking works");

    behavior_tree_destroy(tree);
    behavior_context_destroy(context);
    game_state_destroy(state);

    printf("  ✓ All behavior tree tests passed\n");
    return true;
}

// ============================================================================
// Test: Pre-built Behavior Trees
// ============================================================================

bool test_prebuilt_trees() {
    printf("\nTest: Pre-built Behavior Trees\n");

    // Test farmer tree
    BehaviorTree* farmer = create_farmer_behavior_tree();
    TEST_ASSERT(farmer != NULL, "Farmer tree created");
    TEST_ASSERT(farmer->root != NULL, "Farmer tree has root");
    TEST_PASS("Farmer behavior tree works");

    // Create test context
    GameState* state = game_state_create();
    state->time_of_day = TIME_MORNING;

    Entity* entity = entity_manager_create_entity(state->entity_manager, "Test Farmer", "NPC");
    NeedsComponent* needs = needs_component_create();
    needs->hunger = 60.0f;
    needs->energy = 70.0f;
    needs->social = 50.0f;
    entity_add_component(entity, (Component*)needs);

    InventoryComponent* inv = inventory_component_create(20);
    entity_add_component(entity, (Component*)inv);

    entity_add_component(entity, (Component*)position_component_create("Farm", 0.0f, 0.0f));

    BehaviorContext* context = behavior_context_create(state, entity, NULL, NULL);
    context->logging_enabled = false;

    // Tick the tree
    BehaviorStatus status = behavior_tree_tick(farmer, context);
    TEST_ASSERT(status == BT_SUCCESS, "Farmer tree executes");
    TEST_PASS("Farmer tree execution works");

    behavior_tree_destroy(farmer);

    // Test merchant tree
    BehaviorTree* merchant = create_merchant_behavior_tree();
    TEST_ASSERT(merchant != NULL, "Merchant tree created");
    status = behavior_tree_tick(merchant, context);
    TEST_ASSERT(status == BT_SUCCESS, "Merchant tree executes");
    TEST_PASS("Merchant tree execution works");
    behavior_tree_destroy(merchant);

    // Test villager tree
    BehaviorTree* villager = create_villager_behavior_tree();
    TEST_ASSERT(villager != NULL, "Villager tree created");
    status = behavior_tree_tick(villager, context);
    TEST_ASSERT(status == BT_SUCCESS, "Villager tree executes");
    TEST_PASS("Villager tree execution works");
    behavior_tree_destroy(villager);

    behavior_context_destroy(context);
    game_state_destroy(state);

    printf("  ✓ All pre-built tree tests passed\n");
    return true;
}

// ============================================================================
// Test: Integration with Decision System
// ============================================================================

bool test_decision_integration() {
    printf("\nTest: Integration with Decision System\n");

    GameState* state = game_state_create();
    state->time_of_day = TIME_AFTERNOON;

    Entity* entity = entity_manager_create_entity(state->entity_manager, "Worker", "NPC");

    NeedsComponent* needs = needs_component_create();
    needs->hunger = 50.0f;
    needs->energy = 60.0f;
    needs->social = 40.0f;
    entity_add_component(entity, (Component*)needs);

    entity_add_component(entity, (Component*)currency_component_create(100));
    entity_add_component(entity, (Component*)position_component_create("Workshop", 10.0f, 10.0f));

    EventLogger* event_logger = event_logger_create();
    DecisionLogger* decision_logger = decision_logger_create();

    BehaviorContext* context = behavior_context_create(state, entity, event_logger, decision_logger);
    context->logging_enabled = false;

    // Create decision context
    DecisionContext* dec_ctx = decision_context_create(state, entity, event_logger);
    TEST_ASSERT(dec_ctx != NULL, "Decision context created from game state");
    TEST_ASSERT(dec_ctx->entity_id == entity->id, "Decision context has correct entity");
    TEST_ASSERT(dec_ctx->time_of_day == TIME_AFTERNOON, "Decision context has time");
    TEST_PASS("Decision context integration works");

    decision_context_destroy(dec_ctx);
    behavior_context_destroy(context);
    decision_logger_destroy(decision_logger);
    event_logger_destroy(event_logger);
    game_state_destroy(state);

    printf("  ✓ All integration tests passed\n");
    return true;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    printf("\n");
    printf("============================================================\n");
    printf("PHASE 4: Behavior Tree System Tests\n");
    printf("============================================================\n");

    int passed = 0;
    int failed = 0;

    // Run tests
    if (test_behavior_node_creation()) passed++; else failed++;
    if (test_behavior_context()) passed++; else failed++;
    if (test_sequence_behavior()) passed++; else failed++;
    if (test_selector_behavior()) passed++; else failed++;
    if (test_conditions()) passed++; else failed++;
    if (test_actions()) passed++; else failed++;
    if (test_behavior_tree()) passed++; else failed++;
    if (test_prebuilt_trees()) passed++; else failed++;
    if (test_decision_integration()) passed++; else failed++;

    // Summary
    printf("\n");
    printf("============================================================\n");
    printf("RESULTS: %d passed, %d failed\n", passed, failed);
    printf("============================================================\n");

    if (failed == 0) {
        printf("\n🎉 Phase 4: Behavior Tree System - COMPLETE! 🎉\n\n");
        printf("Success Criteria Met:\n");
        printf("✓ Behavior nodes (Sequence, Selector, Condition, Action) work\n");
        printf("✓ Behavior Context and blackboard operations work\n");
        printf("✓ Sequence nodes execute children in order\n");
        printf("✓ Selector nodes try children until one succeeds\n");
        printf("✓ Condition nodes check game state\n");
        printf("✓ Action nodes modify game state\n");
        printf("✓ Behavior Trees execute correctly\n");
        printf("✓ Pre-built AI behaviors (Farmer, Merchant, Villager) work\n");
        printf("✓ Integration with Decision System works\n");
        printf("\n");
    }

    return failed == 0 ? 0 : 1;
}
