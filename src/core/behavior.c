/**
 * behavior.c
 * Behavior Tree Implementation
 */

#include "behavior.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// Behavior Status Helpers
// ============================================================================

const char* behavior_status_to_string(BehaviorStatus status) {
    switch (status) {
        case BT_SUCCESS: return "SUCCESS";
        case BT_FAILURE: return "FAILURE";
        case BT_RUNNING: return "RUNNING";
        default: return "UNKNOWN";
    }
}

// ============================================================================
// Behavior Context Implementation
// ============================================================================

BehaviorContext* behavior_context_create(GameState* game_state,
                                         Entity* entity,
                                         EventLogger* event_logger,
                                         DecisionLogger* decision_logger) {
    if (!game_state || !entity) return NULL;

    BehaviorContext* context = calloc(1, sizeof(BehaviorContext));
    if (!context) return NULL;

    context->game_state = game_state;
    context->entity = entity;
    context->event_logger = event_logger;
    context->decision_logger = decision_logger;
    context->decision_context = NULL;  // Created on demand
    context->blackboard_count = 0;
    context->tick_count = 0;
    context->logging_enabled = true;

    return context;
}

void behavior_context_destroy(BehaviorContext* context) {
    if (!context) return;

    if (context->decision_context) {
        decision_context_destroy(context->decision_context);
    }

    // Note: We don't free game_state, entity, event_logger, or decision_logger
    // as they're owned by the caller

    free(context);
}

void behavior_context_set(BehaviorContext* context, const char* key, void* value) {
    if (!context || !key) return;

    // Check if key exists
    for (int i = 0; i < context->blackboard_count; i++) {
        if (strcmp((const char*)context->blackboard_keys[i], key) == 0) {
            context->blackboard_values[i] = value;
            return;
        }
    }

    // Add new entry
    if (context->blackboard_count < MAX_BLACKBOARD_ENTRIES) {
        context->blackboard_keys[context->blackboard_count] = (void*)key;
        context->blackboard_values[context->blackboard_count] = value;
        context->blackboard_count++;
    }
}

void* behavior_context_get(BehaviorContext* context, const char* key) {
    if (!context || !key) return NULL;

    for (int i = 0; i < context->blackboard_count; i++) {
        if (strcmp((const char*)context->blackboard_keys[i], key) == 0) {
            return context->blackboard_values[i];
        }
    }

    return NULL;
}

bool behavior_context_has(const BehaviorContext* context, const char* key) {
    if (!context || !key) return false;

    for (int i = 0; i < context->blackboard_count; i++) {
        if (strcmp((const char*)context->blackboard_keys[i], key) == 0) {
            return true;
        }
    }

    return false;
}

// ============================================================================
// Behavior Node Implementation
// ============================================================================

static BehaviorNode* behavior_node_create_base(BehaviorNodeType type, const char* name) {
    BehaviorNode* node = calloc(1, sizeof(BehaviorNode));
    if (!node) return NULL;

    node->type = type;
    if (name) {
        strncpy(node->name, name, MAX_BEHAVIOR_NAME - 1);
    }
    node->child_count = 0;
    node->current_child = 0;
    node->condition = NULL;
    node->action = NULL;
    node->decorated_child = NULL;
    node->invert_result = false;
    node->repeat_count = 1;
    node->current_repeat = 0;
    node->last_status = BT_FAILURE;
    node->execution_count = 0;

    return node;
}

BehaviorNode* behavior_node_create_sequence(const char* name) {
    return behavior_node_create_base(BT_NODE_SEQUENCE, name);
}

BehaviorNode* behavior_node_create_selector(const char* name) {
    return behavior_node_create_base(BT_NODE_SELECTOR, name);
}

BehaviorNode* behavior_node_create_parallel(const char* name) {
    return behavior_node_create_base(BT_NODE_PARALLEL, name);
}

BehaviorNode* behavior_node_create_condition(const char* name, ConditionFunc condition) {
    BehaviorNode* node = behavior_node_create_base(BT_NODE_CONDITION, name);
    if (node) {
        node->condition = condition;
    }
    return node;
}

BehaviorNode* behavior_node_create_action(const char* name, ActionFunc action) {
    BehaviorNode* node = behavior_node_create_base(BT_NODE_ACTION, name);
    if (node) {
        node->action = action;
    }
    return node;
}

BehaviorNode* behavior_node_create_inverter(const char* name, BehaviorNode* child) {
    BehaviorNode* node = behavior_node_create_base(BT_NODE_DECORATOR, name);
    if (node) {
        node->decorated_child = child;
        node->invert_result = true;
    }
    return node;
}

BehaviorNode* behavior_node_create_repeater(const char* name, BehaviorNode* child, int count) {
    BehaviorNode* node = behavior_node_create_base(BT_NODE_DECORATOR, name);
    if (node) {
        node->decorated_child = child;
        node->repeat_count = count;
    }
    return node;
}

void behavior_node_add_child(BehaviorNode* parent, BehaviorNode* child) {
    if (!parent || !child) return;
    if (parent->child_count >= MAX_BEHAVIOR_CHILDREN) return;

    parent->children[parent->child_count++] = child;
}

static BehaviorStatus tick_sequence(BehaviorNode* node, BehaviorContext* context) {
    // Execute children in order until one fails
    for (int i = node->current_child; i < node->child_count; i++) {
        BehaviorStatus status = behavior_node_tick(node->children[i], context);

        if (status == BT_FAILURE) {
            node->current_child = 0;  // Reset for next time
            return BT_FAILURE;
        }

        if (status == BT_RUNNING) {
            node->current_child = i;  // Resume from here next tick
            return BT_RUNNING;
        }

        // SUCCESS - continue to next child
    }

    // All children succeeded
    node->current_child = 0;
    return BT_SUCCESS;
}

static BehaviorStatus tick_selector(BehaviorNode* node, BehaviorContext* context) {
    // Execute children until one succeeds
    for (int i = node->current_child; i < node->child_count; i++) {
        BehaviorStatus status = behavior_node_tick(node->children[i], context);

        if (status == BT_SUCCESS) {
            node->current_child = 0;  // Reset for next time
            return BT_SUCCESS;
        }

        if (status == BT_RUNNING) {
            node->current_child = i;  // Resume from here next tick
            return BT_RUNNING;
        }

        // FAILURE - continue to next child
    }

    // All children failed
    node->current_child = 0;
    return BT_FAILURE;
}

static BehaviorStatus tick_parallel(BehaviorNode* node, BehaviorContext* context) {
    // Execute all children simultaneously
    int success_count = 0;
    int failure_count = 0;
    int running_count = 0;

    for (int i = 0; i < node->child_count; i++) {
        BehaviorStatus status = behavior_node_tick(node->children[i], context);

        switch (status) {
            case BT_SUCCESS: success_count++; break;
            case BT_FAILURE: failure_count++; break;
            case BT_RUNNING: running_count++; break;
        }
    }

    // If any child is still running, return RUNNING
    if (running_count > 0) {
        return BT_RUNNING;
    }

    // If all children succeeded, return SUCCESS
    if (success_count == node->child_count) {
        return BT_SUCCESS;
    }

    // Otherwise, return FAILURE
    return BT_FAILURE;
}

static BehaviorStatus tick_condition(BehaviorNode* node, BehaviorContext* context) {
    if (!node->condition) return BT_FAILURE;

    bool result = node->condition(context);
    return result ? BT_SUCCESS : BT_FAILURE;
}

static BehaviorStatus tick_action(BehaviorNode* node, BehaviorContext* context) {
    if (!node->action) return BT_FAILURE;

    return node->action(context);
}

static BehaviorStatus tick_decorator(BehaviorNode* node, BehaviorContext* context) {
    if (!node->decorated_child) return BT_FAILURE;

    BehaviorStatus status = behavior_node_tick(node->decorated_child, context);

    // Inverter
    if (node->invert_result) {
        if (status == BT_SUCCESS) return BT_FAILURE;
        if (status == BT_FAILURE) return BT_SUCCESS;
        return status;  // RUNNING stays RUNNING
    }

    // Repeater
    if (node->repeat_count > 1) {
        if (status == BT_SUCCESS) {
            node->current_repeat++;
            if (node->current_repeat >= node->repeat_count) {
                node->current_repeat = 0;
                return BT_SUCCESS;
            }
            return BT_RUNNING;  // Keep repeating
        }
        node->current_repeat = 0;
        return status;
    }

    return status;
}

BehaviorStatus behavior_node_tick(BehaviorNode* node, BehaviorContext* context) {
    if (!node || !context) return BT_FAILURE;

    node->execution_count++;

    BehaviorStatus status;

    switch (node->type) {
        case BT_NODE_SEQUENCE:
            status = tick_sequence(node, context);
            break;

        case BT_NODE_SELECTOR:
            status = tick_selector(node, context);
            break;

        case BT_NODE_PARALLEL:
            status = tick_parallel(node, context);
            break;

        case BT_NODE_CONDITION:
            status = tick_condition(node, context);
            break;

        case BT_NODE_ACTION:
            status = tick_action(node, context);
            break;

        case BT_NODE_DECORATOR:
            status = tick_decorator(node, context);
            break;

        default:
            status = BT_FAILURE;
            break;
    }

    node->last_status = status;

    if (context->logging_enabled) {
        printf("[BT] %s: %s\n", node->name, behavior_status_to_string(status));
    }

    return status;
}

void behavior_node_destroy(BehaviorNode* node) {
    if (!node) return;

    // Destroy children
    for (int i = 0; i < node->child_count; i++) {
        behavior_node_destroy(node->children[i]);
    }

    // Destroy decorated child
    if (node->decorated_child) {
        behavior_node_destroy(node->decorated_child);
    }

    free(node);
}

void behavior_node_print(const BehaviorNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) printf("  ");

    const char* type_str = "Unknown";
    switch (node->type) {
        case BT_NODE_SEQUENCE: type_str = "Sequence"; break;
        case BT_NODE_SELECTOR: type_str = "Selector"; break;
        case BT_NODE_PARALLEL: type_str = "Parallel"; break;
        case BT_NODE_CONDITION: type_str = "Condition"; break;
        case BT_NODE_ACTION: type_str = "Action"; break;
        case BT_NODE_DECORATOR: type_str = "Decorator"; break;
    }

    printf("[%s] %s (executions: %d, last: %s)\n",
           type_str, node->name, node->execution_count,
           behavior_status_to_string(node->last_status));

    // Print children
    for (int i = 0; i < node->child_count; i++) {
        behavior_node_print(node->children[i], indent + 1);
    }

    // Print decorated child
    if (node->decorated_child) {
        behavior_node_print(node->decorated_child, indent + 1);
    }
}

// ============================================================================
// Behavior Tree Implementation
// ============================================================================

BehaviorTree* behavior_tree_create(const char* name, BehaviorNode* root) {
    if (!root) return NULL;

    BehaviorTree* tree = calloc(1, sizeof(BehaviorTree));
    if (!tree) return NULL;

    tree->root = root;
    if (name) {
        strncpy(tree->name, name, MAX_BEHAVIOR_NAME - 1);
    }
    tree->entity_id = -1;
    tree->total_ticks = 0;
    tree->successful_ticks = 0;
    tree->failed_ticks = 0;
    tree->running_ticks = 0;

    return tree;
}

void behavior_tree_destroy(BehaviorTree* tree) {
    if (!tree) return;

    if (tree->root) {
        behavior_node_destroy(tree->root);
    }

    free(tree);
}

BehaviorStatus behavior_tree_tick(BehaviorTree* tree, BehaviorContext* context) {
    if (!tree || !tree->root || !context) return BT_FAILURE;

    tree->total_ticks++;
    context->tick_count++;

    BehaviorStatus status = behavior_node_tick(tree->root, context);

    // Update statistics
    switch (status) {
        case BT_SUCCESS: tree->successful_ticks++; break;
        case BT_FAILURE: tree->failed_ticks++; break;
        case BT_RUNNING: tree->running_ticks++; break;
    }

    return status;
}

void behavior_tree_reset(BehaviorTree* tree) {
    if (!tree || !tree->root) return;

    // Reset would traverse the tree and reset current_child indices
    // For now, this is a simple implementation
    tree->root->current_child = 0;
}

void behavior_tree_print_stats(const BehaviorTree* tree) {
    if (!tree) return;

    printf("\n=== Behavior Tree Stats: %s ===\n", tree->name);
    printf("Total Ticks: %d\n", tree->total_ticks);
    printf("Success: %d (%.1f%%)\n", tree->successful_ticks,
           tree->total_ticks > 0 ? 100.0f * tree->successful_ticks / tree->total_ticks : 0);
    printf("Failure: %d (%.1f%%)\n", tree->failed_ticks,
           tree->total_ticks > 0 ? 100.0f * tree->failed_ticks / tree->total_ticks : 0);
    printf("Running: %d (%.1f%%)\n", tree->running_ticks,
           tree->total_ticks > 0 ? 100.0f * tree->running_ticks / tree->total_ticks : 0);
    printf("===============================\n\n");
}

// ============================================================================
// Common Conditions Implementation
// ============================================================================

bool condition_is_hungry(BehaviorContext* context) {
    if (!context || !context->entity) return false;

    NeedsComponent* needs = (NeedsComponent*)entity_get_component(context->entity, COMPONENT_NEEDS);
    if (!needs) return false;

    return needs->hunger < 30.0f;  // Hungry if below 30
}

bool condition_is_tired(BehaviorContext* context) {
    if (!context || !context->entity) return false;

    NeedsComponent* needs = (NeedsComponent*)entity_get_component(context->entity, COMPONENT_NEEDS);
    if (!needs) return false;

    return needs->energy < 30.0f;  // Tired if below 30
}

bool condition_is_lonely(BehaviorContext* context) {
    if (!context || !context->entity) return false;

    NeedsComponent* needs = (NeedsComponent*)entity_get_component(context->entity, COMPONENT_NEEDS);
    if (!needs) return false;

    return needs->social < 30.0f;  // Lonely if below 30
}

bool condition_needs_urgent(BehaviorContext* context) {
    if (!context || !context->entity) return false;

    NeedsComponent* needs = (NeedsComponent*)entity_get_component(context->entity, COMPONENT_NEEDS);
    if (!needs) return false;

    // Any need below 20 is urgent
    return needs->hunger < 20.0f || needs->energy < 20.0f || needs->social < 20.0f;
}

bool condition_has_currency(BehaviorContext* context) {
    if (!context || !context->entity) return false;

    CurrencyComponent* currency = (CurrencyComponent*)entity_get_component(context->entity, COMPONENT_CURRENCY);
    if (!currency) return false;

    // Check if has at least 10 gold
    return currency->amount >= 10;
}

bool condition_inventory_full(BehaviorContext* context) {
    if (!context || !context->entity) return false;

    InventoryComponent* inv = (InventoryComponent*)entity_get_component(context->entity, COMPONENT_INVENTORY);
    if (!inv) return false;

    return inv->item_count >= inv->capacity;
}

bool condition_inventory_has_item(BehaviorContext* context) {
    if (!context || !context->entity) return false;

    InventoryComponent* inv = (InventoryComponent*)entity_get_component(context->entity, COMPONENT_INVENTORY);
    if (!inv) return false;

    // Check for any food item
    return inventory_component_has_item(inv, "bread", 1) ||
           inventory_component_has_item(inv, "wheat", 1);
}

bool condition_is_morning(BehaviorContext* context) {
    if (!context || !context->game_state) return false;
    return context->game_state->time_of_day == TIME_MORNING;
}

bool condition_is_afternoon(BehaviorContext* context) {
    if (!context || !context->game_state) return false;
    return context->game_state->time_of_day == TIME_AFTERNOON;
}

bool condition_is_evening(BehaviorContext* context) {
    if (!context || !context->game_state) return false;
    return context->game_state->time_of_day == TIME_EVENING;
}

bool condition_is_night(BehaviorContext* context) {
    if (!context || !context->game_state) return false;
    return context->game_state->time_of_day == TIME_NIGHT;
}

bool condition_nearby_friend(BehaviorContext* context) {
    if (!context || !context->entity || !context->game_state) return false;

    // Get position
    PositionComponent* pos = (PositionComponent*)entity_get_component(context->entity, COMPONENT_POSITION);
    if (!pos) return false;

    // Get relationships
    RelationshipComponent* rel = (RelationshipComponent*)entity_get_component(context->entity, COMPONENT_RELATIONSHIP);
    if (!rel) return false;

    // Check for nearby entities with positive relationships
    for (int i = 0; i < context->game_state->entity_manager->entity_count; i++) {
        Entity* other = context->game_state->entity_manager->entities[i];
        if (!other || !other->active || other->id == context->entity->id) continue;

        // Get other's position
        PositionComponent* other_pos = (PositionComponent*)entity_get_component(other, COMPONENT_POSITION);
        if (!other_pos) continue;

        // Check distance (within 10 units)
        float dx = other_pos->x - pos->x;
        float dy = other_pos->y - pos->y;
        float dist_sq = dx * dx + dy * dy;

        if (dist_sq <= 100.0f) {  // 10 * 10
            // Check relationship
            int rel_value = relationship_component_get(rel, other->id);
            if (rel_value > 30) {  // Friend if > 30
                return true;
            }
        }
    }

    return false;
}

bool condition_at_workplace(BehaviorContext* context) {
    if (!context || !context->entity) return false;

    PositionComponent* pos = (PositionComponent*)entity_get_component(context->entity, COMPONENT_POSITION);
    OccupationComponent* occ = (OccupationComponent*)entity_get_component(context->entity, COMPONENT_OCCUPATION);

    if (!pos || !occ) return false;

    // Simple check: if workplace is in location string
    return strstr(pos->location, occ->workplace) != NULL;
}

// ============================================================================
// Common Actions Implementation
// ============================================================================

BehaviorStatus action_move_to_location(BehaviorContext* context) {
    if (!context || !context->entity) return BT_FAILURE;

    // Get target location from blackboard
    const char* target = (const char*)behavior_context_get(context, "target_location");
    if (!target) return BT_FAILURE;

    PositionComponent* pos = (PositionComponent*)entity_get_component(context->entity, COMPONENT_POSITION);
    if (!pos) return BT_FAILURE;

    // Simple implementation: just update location
    strncpy(pos->location, target, sizeof(pos->location) - 1);

    return BT_SUCCESS;
}

BehaviorStatus action_wait(BehaviorContext* context) {
    (void)context;  // Unused
    // Just succeed immediately
    return BT_SUCCESS;
}

BehaviorStatus action_idle(BehaviorContext* context) {
    (void)context;  // Unused
    // Just succeed immediately
    return BT_SUCCESS;
}

BehaviorStatus action_eat_food(BehaviorContext* context) {
    if (!context || !context->entity) return BT_FAILURE;

    NeedsComponent* needs = (NeedsComponent*)entity_get_component(context->entity, COMPONENT_NEEDS);
    InventoryComponent* inv = (InventoryComponent*)entity_get_component(context->entity, COMPONENT_INVENTORY);

    if (!needs || !inv) return BT_FAILURE;

    // Try to eat bread
    if (inventory_component_remove_item(inv, "bread", 1)) {
        needs_component_eat(needs, 30.0f);
        return BT_SUCCESS;
    }

    // Try to eat wheat
    if (inventory_component_remove_item(inv, "wheat", 1)) {
        needs_component_eat(needs, 15.0f);
        return BT_SUCCESS;
    }

    return BT_FAILURE;  // No food available
}

BehaviorStatus action_rest(BehaviorContext* context) {
    if (!context || !context->entity) return BT_FAILURE;

    NeedsComponent* needs = (NeedsComponent*)entity_get_component(context->entity, COMPONENT_NEEDS);
    if (!needs) return BT_FAILURE;

    needs_component_rest(needs, 40.0f);
    return BT_SUCCESS;
}

BehaviorStatus action_socialize(BehaviorContext* context) {
    if (!context || !context->entity) return BT_FAILURE;

    NeedsComponent* needs = (NeedsComponent*)entity_get_component(context->entity, COMPONENT_NEEDS);
    if (!needs) return BT_FAILURE;

    needs_component_socialize(needs, 30.0f);
    return BT_SUCCESS;
}

BehaviorStatus action_work(BehaviorContext* context) {
    if (!context || !context->entity) return BT_FAILURE;

    NeedsComponent* needs = (NeedsComponent*)entity_get_component(context->entity, COMPONENT_NEEDS);
    CurrencyComponent* currency = (CurrencyComponent*)entity_get_component(context->entity, COMPONENT_CURRENCY);

    if (needs) {
        needs->energy -= 10.0f;  // Work costs energy
        if (needs->energy < 0) needs->energy = 0;
    }

    if (currency) {
        currency_component_add(currency, 20);  // Earn 20 gold
    }

    return BT_SUCCESS;
}

BehaviorStatus action_farm(BehaviorContext* context) {
    if (!context || !context->entity) return BT_FAILURE;

    NeedsComponent* needs = (NeedsComponent*)entity_get_component(context->entity, COMPONENT_NEEDS);
    InventoryComponent* inv = (InventoryComponent*)entity_get_component(context->entity, COMPONENT_INVENTORY);

    if (needs) {
        needs->energy -= 15.0f;  // Farming costs more energy
        if (needs->energy < 0) needs->energy = 0;
    }

    if (inv) {
        inventory_component_add_item(inv, "wheat", 5);  // Harvest 5 wheat
    }

    return BT_SUCCESS;
}

BehaviorStatus action_talk_to_nearby(BehaviorContext* context) {
    if (!context || !context->entity) return BT_FAILURE;

    NeedsComponent* needs = (NeedsComponent*)entity_get_component(context->entity, COMPONENT_NEEDS);
    if (needs) {
        needs_component_socialize(needs, 25.0f);
    }

    return BT_SUCCESS;
}

BehaviorStatus action_give_gift(BehaviorContext* context) {
    if (!context || !context->entity) return BT_FAILURE;

    InventoryComponent* inv = (InventoryComponent*)entity_get_component(context->entity, COMPONENT_INVENTORY);
    if (!inv) return BT_FAILURE;

    // Give away an item if we have one
    if (inventory_component_remove_item(inv, "wheat", 1)) {
        return BT_SUCCESS;
    }

    return BT_FAILURE;
}

// ============================================================================
// Behavior Tree Builders
// ============================================================================

BehaviorTree* create_npc_behavior_tree(const char* name) {
    // Simple NPC behavior: Address needs, then idle
    BehaviorNode* root = behavior_node_create_selector("NPC Root");

    // Handle urgent needs first
    BehaviorNode* urgent = behavior_node_create_sequence("Handle Urgent Needs");
    behavior_node_add_child(urgent, behavior_node_create_condition("Needs Urgent?", condition_needs_urgent));
    behavior_node_add_child(urgent, behavior_node_create_action("Rest", action_rest));
    behavior_node_add_child(root, urgent);

    // Handle hunger
    BehaviorNode* hunger = behavior_node_create_sequence("Handle Hunger");
    behavior_node_add_child(hunger, behavior_node_create_condition("Hungry?", condition_is_hungry));
    behavior_node_add_child(hunger, behavior_node_create_action("Eat", action_eat_food));
    behavior_node_add_child(root, hunger);

    // Handle tiredness
    BehaviorNode* tired = behavior_node_create_sequence("Handle Tiredness");
    behavior_node_add_child(tired, behavior_node_create_condition("Tired?", condition_is_tired));
    behavior_node_add_child(tired, behavior_node_create_action("Rest", action_rest));
    behavior_node_add_child(root, tired);

    // Socialize if lonely
    BehaviorNode* social = behavior_node_create_sequence("Handle Loneliness");
    behavior_node_add_child(social, behavior_node_create_condition("Lonely?", condition_is_lonely));
    behavior_node_add_child(social, behavior_node_create_action("Socialize", action_socialize));
    behavior_node_add_child(root, social);

    // Default: idle
    behavior_node_add_child(root, behavior_node_create_action("Idle", action_idle));

    return behavior_tree_create(name, root);
}

BehaviorTree* create_farmer_behavior_tree(void) {
    BehaviorNode* root = behavior_node_create_selector("Farmer Root");

    // Urgent needs (below 20)
    BehaviorNode* urgent = behavior_node_create_sequence("Urgent Needs");
    behavior_node_add_child(urgent, behavior_node_create_condition("Needs Urgent?", condition_needs_urgent));
    BehaviorNode* urgent_choice = behavior_node_create_selector("Choose Urgent Action");
    BehaviorNode* eat_urgent = behavior_node_create_sequence("Eat if Food");
    behavior_node_add_child(eat_urgent, behavior_node_create_condition("Has Food?", condition_inventory_has_item));
    behavior_node_add_child(eat_urgent, behavior_node_create_action("Eat", action_eat_food));
    behavior_node_add_child(urgent_choice, eat_urgent);
    behavior_node_add_child(urgent_choice, behavior_node_create_action("Rest", action_rest));
    behavior_node_add_child(urgent, urgent_choice);
    behavior_node_add_child(root, urgent);

    // Morning routine: farm
    BehaviorNode* morning_work = behavior_node_create_sequence("Morning Farming");
    behavior_node_add_child(morning_work, behavior_node_create_condition("Is Morning?", condition_is_morning));
    behavior_node_add_child(morning_work, behavior_node_create_action("Farm", action_farm));
    behavior_node_add_child(root, morning_work);

    // Afternoon: work
    BehaviorNode* afternoon_work = behavior_node_create_sequence("Afternoon Work");
    behavior_node_add_child(afternoon_work, behavior_node_create_condition("Is Afternoon?", condition_is_afternoon));
    behavior_node_add_child(afternoon_work, behavior_node_create_action("Work", action_work));
    behavior_node_add_child(root, afternoon_work);

    // Evening: socialize if friend nearby
    BehaviorNode* evening_social = behavior_node_create_sequence("Evening Socialize");
    behavior_node_add_child(evening_social, behavior_node_create_condition("Is Evening?", condition_is_evening));
    behavior_node_add_child(evening_social, behavior_node_create_condition("Friend Nearby?", condition_nearby_friend));
    behavior_node_add_child(evening_social, behavior_node_create_action("Talk", action_talk_to_nearby));
    behavior_node_add_child(root, evening_social);

    // Night: rest
    BehaviorNode* night_rest = behavior_node_create_sequence("Night Rest");
    behavior_node_add_child(night_rest, behavior_node_create_condition("Is Night?", condition_is_night));
    behavior_node_add_child(night_rest, behavior_node_create_action("Rest", action_rest));
    behavior_node_add_child(root, night_rest);

    // Default: idle
    behavior_node_add_child(root, behavior_node_create_action("Idle", action_idle));

    return behavior_tree_create("Farmer Behavior", root);
}

BehaviorTree* create_merchant_behavior_tree(void) {
    BehaviorNode* root = behavior_node_create_selector("Merchant Root");

    // Handle needs first
    BehaviorNode* needs_seq = behavior_node_create_sequence("Handle Needs");
    behavior_node_add_child(needs_seq, behavior_node_create_condition("Needs Urgent?", condition_needs_urgent));
    behavior_node_add_child(needs_seq, behavior_node_create_action("Rest", action_rest));
    behavior_node_add_child(root, needs_seq);

    // Work during business hours (morning/afternoon)
    BehaviorNode* business = behavior_node_create_sequence("Business Hours");
    BehaviorNode* business_time = behavior_node_create_selector("Is Business Time?");
    behavior_node_add_child(business_time, behavior_node_create_condition("Morning?", condition_is_morning));
    behavior_node_add_child(business_time, behavior_node_create_condition("Afternoon?", condition_is_afternoon));
    behavior_node_add_child(business, business_time);
    behavior_node_add_child(business, behavior_node_create_action("Work", action_work));
    behavior_node_add_child(root, business);

    // Evening: socialize
    BehaviorNode* evening = behavior_node_create_sequence("Evening");
    behavior_node_add_child(evening, behavior_node_create_condition("Evening?", condition_is_evening));
    behavior_node_add_child(evening, behavior_node_create_action("Socialize", action_socialize));
    behavior_node_add_child(root, evening);

    // Default: idle
    behavior_node_add_child(root, behavior_node_create_action("Wait", action_wait));

    return behavior_tree_create("Merchant Behavior", root);
}

BehaviorTree* create_villager_behavior_tree(void) {
    // Simple villager: handle needs, socialize, idle
    return create_npc_behavior_tree("Villager Behavior");
}
