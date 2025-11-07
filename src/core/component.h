/**
 * component.h
 * Component System for Transparent Game Engine
 *
 * Components are data containers that can be attached to entities.
 * This implements an Entity-Component-System (ECS) architecture.
 */

#ifndef COMPONENT_H
#define COMPONENT_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct cJSON cJSON;

// Component Types
typedef enum {
    COMPONENT_POSITION,
    COMPONENT_HEALTH,
    COMPONENT_INVENTORY,
    COMPONENT_CURRENCY,
    COMPONENT_RELATIONSHIP,
    COMPONENT_NEEDS,
    COMPONENT_SCHEDULE,
    COMPONENT_OCCUPATION,
    COMPONENT_MEMORY,
    COMPONENT_GOAL,
    COMPONENT_TYPE_COUNT  // Total number of component types
} ComponentType;

// Base Component (used for polymorphism)
typedef struct {
    ComponentType type;
    int entity_id;  // ID of entity this component is attached to
} Component;

// ============================================================================
// Position Component
// ============================================================================
#define MAX_LOCATION_NAME 64

typedef struct {
    Component base;
    char location[MAX_LOCATION_NAME];  // e.g., "YourFarm", "VillageSquare"
    float x;
    float y;
} PositionComponent;

PositionComponent* position_component_create(const char* location, float x, float y);
void position_component_destroy(PositionComponent* comp);
cJSON* position_component_to_json(const PositionComponent* comp);
PositionComponent* position_component_from_json(cJSON* json);

// ============================================================================
// Health Component
// ============================================================================
typedef struct {
    Component base;
    int current;
    int maximum;
} HealthComponent;

HealthComponent* health_component_create(int current, int maximum);
void health_component_destroy(HealthComponent* comp);
bool health_component_is_alive(const HealthComponent* comp);
void health_component_damage(HealthComponent* comp, int amount);
void health_component_heal(HealthComponent* comp, int amount);
cJSON* health_component_to_json(const HealthComponent* comp);
HealthComponent* health_component_from_json(cJSON* json);

// ============================================================================
// Inventory Component
// ============================================================================
#define MAX_ITEM_NAME 32
#define MAX_INVENTORY_ITEMS 50

typedef struct {
    char item_name[MAX_ITEM_NAME];
    int quantity;
} ItemStack;

typedef struct {
    Component base;
    ItemStack items[MAX_INVENTORY_ITEMS];
    int item_count;  // Number of different item types
    int capacity;    // Maximum number of item stacks
} InventoryComponent;

InventoryComponent* inventory_component_create(int capacity);
void inventory_component_destroy(InventoryComponent* comp);
bool inventory_component_add_item(InventoryComponent* comp, const char* item_name, int quantity);
bool inventory_component_remove_item(InventoryComponent* comp, const char* item_name, int quantity);
bool inventory_component_has_item(const InventoryComponent* comp, const char* item_name, int quantity);
int inventory_component_get_count(const InventoryComponent* comp, const char* item_name);
cJSON* inventory_component_to_json(const InventoryComponent* comp);
InventoryComponent* inventory_component_from_json(cJSON* json);

// ============================================================================
// Currency Component
// ============================================================================
typedef struct {
    Component base;
    int amount;
} CurrencyComponent;

CurrencyComponent* currency_component_create(int amount);
void currency_component_destroy(CurrencyComponent* comp);
void currency_component_add(CurrencyComponent* comp, int value);
bool currency_component_remove(CurrencyComponent* comp, int value);
bool currency_component_has(const CurrencyComponent* comp, int value);
cJSON* currency_component_to_json(const CurrencyComponent* comp);
CurrencyComponent* currency_component_from_json(cJSON* json);

// ============================================================================
// Relationship Component
// ============================================================================
#define MAX_RELATIONSHIPS 100

typedef struct {
    int entity_id;
    int value;  // -100 to +100
} SimpleRelationship;

typedef struct {
    Component base;
    SimpleRelationship relationships[MAX_RELATIONSHIPS];
    int relationship_count;
} RelationshipComponent;

RelationshipComponent* relationship_component_create(void);
void relationship_component_destroy(RelationshipComponent* comp);
int relationship_component_get(const RelationshipComponent* comp, int entity_id);
void relationship_component_set(RelationshipComponent* comp, int entity_id, int value);
void relationship_component_modify(RelationshipComponent* comp, int entity_id, int delta);
const char* relationship_component_get_level(const RelationshipComponent* comp, int entity_id);
cJSON* relationship_component_to_json(const RelationshipComponent* comp);
RelationshipComponent* relationship_component_from_json(cJSON* json);

// ============================================================================
// Needs Component
// ============================================================================
typedef struct {
    Component base;
    float hunger;   // 0-100 (0 = starving, 100 = full)
    float energy;   // 0-100 (0 = exhausted, 100 = fully rested)
    float social;   // 0-100 (0 = lonely, 100 = socially fulfilled)
} NeedsComponent;

NeedsComponent* needs_component_create(void);
void needs_component_destroy(NeedsComponent* comp);
void needs_component_decay(NeedsComponent* comp, float delta_time);
void needs_component_eat(NeedsComponent* comp, float food_value);
void needs_component_rest(NeedsComponent* comp, float rest_value);
void needs_component_socialize(NeedsComponent* comp, float social_value);
const char* needs_component_get_most_urgent(const NeedsComponent* comp);
cJSON* needs_component_to_json(const NeedsComponent* comp);
NeedsComponent* needs_component_from_json(cJSON* json);

// ============================================================================
// Schedule Component
// ============================================================================
#define MAX_SCHEDULE_ENTRIES 10
#define MAX_TIME_NAME 32
#define MAX_ACTIVITY_NAME 64

typedef struct {
    char time_of_day[MAX_TIME_NAME];
    char activity[MAX_ACTIVITY_NAME];
} ScheduleEntry;

typedef struct {
    Component base;
    ScheduleEntry entries[MAX_SCHEDULE_ENTRIES];
    int entry_count;
} ScheduleComponent;

ScheduleComponent* schedule_component_create(void);
void schedule_component_destroy(ScheduleComponent* comp);
const char* schedule_component_get_activity(const ScheduleComponent* comp, const char* time_of_day);
void schedule_component_set_activity(ScheduleComponent* comp, const char* time_of_day, const char* activity);
cJSON* schedule_component_to_json(const ScheduleComponent* comp);
ScheduleComponent* schedule_component_from_json(cJSON* json);

// ============================================================================
// Occupation Component
// ============================================================================
#define MAX_OCCUPATION_NAME 32
#define MAX_WORKPLACE_NAME 64

typedef struct {
    Component base;
    char occupation[MAX_OCCUPATION_NAME];
    char workplace[MAX_WORKPLACE_NAME];
    int skill_level;
} OccupationComponent;

OccupationComponent* occupation_component_create(const char* occupation, const char* workplace, int skill_level);
void occupation_component_destroy(OccupationComponent* comp);
cJSON* occupation_component_to_json(const OccupationComponent* comp);
OccupationComponent* occupation_component_from_json(cJSON* json);

// ============================================================================
// Memory Component
// ============================================================================
#define MAX_MEMORIES 50
#define MAX_MEMORY_TEXT 256

typedef struct {
    char text[MAX_MEMORY_TEXT];
    int day;
    char time_of_day[MAX_TIME_NAME];
} Memory;

typedef struct {
    Component base;
    Memory memories[MAX_MEMORIES];
    int memory_count;
    int max_memories;
} MemoryComponent;

MemoryComponent* memory_component_create(int max_memories);
void memory_component_destroy(MemoryComponent* comp);
void memory_component_add(MemoryComponent* comp, const char* text, int day, const char* time_of_day);
cJSON* memory_component_to_json(const MemoryComponent* comp);
MemoryComponent* memory_component_from_json(cJSON* json);

// ============================================================================
// Goal Component
// ============================================================================
#define MAX_GOALS 10
#define MAX_GOAL_TEXT 128

typedef struct {
    Component base;
    char current_goal[MAX_GOAL_TEXT];
    char goals[MAX_GOALS][MAX_GOAL_TEXT];
    int goal_count;
} GoalComponent;

GoalComponent* goal_component_create(void);
void goal_component_destroy(GoalComponent* comp);
void goal_component_set_current(GoalComponent* comp, const char* goal);
void goal_component_add_goal(GoalComponent* comp, const char* goal);
void goal_component_complete(GoalComponent* comp, const char* goal);
cJSON* goal_component_to_json(const GoalComponent* comp);
GoalComponent* goal_component_from_json(cJSON* json);

// ============================================================================
// Component Factory
// ============================================================================
Component* component_from_json(cJSON* json);
void component_destroy(Component* comp);
cJSON* component_to_json(const Component* comp);

// Helper to get component type name
const char* component_type_to_string(ComponentType type);

#endif // COMPONENT_H
