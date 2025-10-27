/**
 * behavior.h
 * Behavior Tree System for AI
 *
 * Implements a transparent behavior tree system where AI decisions
 * are made using a hierarchy of behavior nodes. All decisions are
 * logged for full transparency.
 */

#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include <stdbool.h>
#include <stdint.h>
#include "decision.h"
#include "entity.h"
#include "game_state.h"
#include "event.h"

// Forward declarations
typedef struct BehaviorNode BehaviorNode;
typedef struct BehaviorTree BehaviorTree;

#define MAX_BEHAVIOR_CHILDREN 10
#define MAX_BEHAVIOR_NAME 64
#define MAX_BLACKBOARD_ENTRIES 50

// ============================================================================
// Behavior Node Status
// ============================================================================

typedef enum {
    BT_SUCCESS,   // Node completed successfully
    BT_FAILURE,   // Node failed
    BT_RUNNING,   // Node is still executing
} BehaviorStatus;

const char* behavior_status_to_string(BehaviorStatus status);

// ============================================================================
// Behavior Node Types
// ============================================================================

typedef enum {
    BT_NODE_SEQUENCE,     // Execute children in order, fail on first failure
    BT_NODE_SELECTOR,     // Execute children until one succeeds
    BT_NODE_PARALLEL,     // Execute all children simultaneously
    BT_NODE_CONDITION,    // Check a condition
    BT_NODE_ACTION,       // Perform an action
    BT_NODE_DECORATOR,    // Modify child behavior (inverter, repeater, etc.)
} BehaviorNodeType;

// ============================================================================
// Behavior Context (passed to nodes during execution)
// ============================================================================

typedef struct {
    GameState* game_state;
    Entity* entity;
    EventLogger* event_logger;
    DecisionLogger* decision_logger;
    DecisionContext* decision_context;

    // Blackboard for sharing data between nodes
    void* blackboard_keys[MAX_BLACKBOARD_ENTRIES];
    void* blackboard_values[MAX_BLACKBOARD_ENTRIES];
    int blackboard_count;

    // Execution tracking
    int tick_count;
    bool logging_enabled;
} BehaviorContext;

BehaviorContext* behavior_context_create(GameState* game_state,
                                         Entity* entity,
                                         EventLogger* event_logger,
                                         DecisionLogger* decision_logger);
void behavior_context_destroy(BehaviorContext* context);

// Blackboard operations
void behavior_context_set(BehaviorContext* context, const char* key, void* value);
void* behavior_context_get(BehaviorContext* context, const char* key);
bool behavior_context_has(const BehaviorContext* context, const char* key);

// ============================================================================
// Behavior Node Function Pointers
// ============================================================================

// Condition function: returns true if condition is met
typedef bool (*ConditionFunc)(BehaviorContext* context);

// Action function: performs an action, returns status
typedef BehaviorStatus (*ActionFunc)(BehaviorContext* context);

// ============================================================================
// Behavior Node Structure
// ============================================================================

struct BehaviorNode {
    BehaviorNodeType type;
    char name[MAX_BEHAVIOR_NAME];

    // Composite nodes (sequence, selector, parallel)
    BehaviorNode* children[MAX_BEHAVIOR_CHILDREN];
    int child_count;
    int current_child;  // For sequence/selector tracking

    // Condition node
    ConditionFunc condition;

    // Action node
    ActionFunc action;

    // Decorator node
    BehaviorNode* decorated_child;
    bool invert_result;  // For inverter decorator
    int repeat_count;    // For repeater decorator
    int current_repeat;

    // Runtime state
    BehaviorStatus last_status;
    int execution_count;
};

// Create nodes
BehaviorNode* behavior_node_create_sequence(const char* name);
BehaviorNode* behavior_node_create_selector(const char* name);
BehaviorNode* behavior_node_create_parallel(const char* name);
BehaviorNode* behavior_node_create_condition(const char* name, ConditionFunc condition);
BehaviorNode* behavior_node_create_action(const char* name, ActionFunc action);
BehaviorNode* behavior_node_create_inverter(const char* name, BehaviorNode* child);
BehaviorNode* behavior_node_create_repeater(const char* name, BehaviorNode* child, int count);

// Add children to composite nodes
void behavior_node_add_child(BehaviorNode* parent, BehaviorNode* child);

// Execute node
BehaviorStatus behavior_node_tick(BehaviorNode* node, BehaviorContext* context);

// Cleanup
void behavior_node_destroy(BehaviorNode* node);

// Debug printing
void behavior_node_print(const BehaviorNode* node, int indent);

// ============================================================================
// Behavior Tree
// ============================================================================

struct BehaviorTree {
    BehaviorNode* root;
    char name[MAX_BEHAVIOR_NAME];
    int entity_id;  // Which entity this tree belongs to

    // Statistics
    int total_ticks;
    int successful_ticks;
    int failed_ticks;
    int running_ticks;
};

BehaviorTree* behavior_tree_create(const char* name, BehaviorNode* root);
void behavior_tree_destroy(BehaviorTree* tree);

// Execute the tree
BehaviorStatus behavior_tree_tick(BehaviorTree* tree, BehaviorContext* context);

// Reset tree state
void behavior_tree_reset(BehaviorTree* tree);

// Statistics
void behavior_tree_print_stats(const BehaviorTree* tree);

// ============================================================================
// Common Conditions
// ============================================================================

// Need-based conditions
bool condition_is_hungry(BehaviorContext* context);
bool condition_is_tired(BehaviorContext* context);
bool condition_is_lonely(BehaviorContext* context);
bool condition_needs_urgent(BehaviorContext* context);

// Resource conditions
bool condition_has_currency(BehaviorContext* context);
bool condition_inventory_full(BehaviorContext* context);
bool condition_inventory_has_item(BehaviorContext* context);

// Time conditions
bool condition_is_morning(BehaviorContext* context);
bool condition_is_afternoon(BehaviorContext* context);
bool condition_is_evening(BehaviorContext* context);
bool condition_is_night(BehaviorContext* context);

// Social conditions
bool condition_nearby_friend(BehaviorContext* context);
bool condition_at_workplace(BehaviorContext* context);

// ============================================================================
// Common Actions
// ============================================================================

// Basic actions
BehaviorStatus action_move_to_location(BehaviorContext* context);
BehaviorStatus action_wait(BehaviorContext* context);
BehaviorStatus action_idle(BehaviorContext* context);

// Need fulfillment actions
BehaviorStatus action_eat_food(BehaviorContext* context);
BehaviorStatus action_rest(BehaviorContext* context);
BehaviorStatus action_socialize(BehaviorContext* context);

// Work actions
BehaviorStatus action_work(BehaviorContext* context);
BehaviorStatus action_farm(BehaviorContext* context);

// Social actions
BehaviorStatus action_talk_to_nearby(BehaviorContext* context);
BehaviorStatus action_give_gift(BehaviorContext* context);

// ============================================================================
// Behavior Tree Builder (for creating common AI behaviors)
// ============================================================================

// Create standard NPC behavior tree
BehaviorTree* create_npc_behavior_tree(const char* name);

// Create farmer behavior tree
BehaviorTree* create_farmer_behavior_tree(void);

// Create merchant behavior tree
BehaviorTree* create_merchant_behavior_tree(void);

// Create villager behavior tree
BehaviorTree* create_villager_behavior_tree(void);

#endif // BEHAVIOR_H
