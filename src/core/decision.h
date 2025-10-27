/**
 * decision.h
 * Decision System for Transparent Game Engine
 *
 * Provides decision-making infrastructure with full transparency.
 * Every decision is logged with complete context and reasoning chain.
 */

#ifndef DECISION_H
#define DECISION_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "component.h"
#include "entity.h"
#include "game_state.h"
#include "event.h"

// Forward declarations
typedef struct cJSON cJSON;

#define MAX_NEARBY_ENTITIES 20
#define MAX_RECENT_EVENTS 10
#define MAX_DECISION_REASONING 1024
#define MAX_DECISION_OPTIONS 10
#define MAX_DECISION_LOG_SIZE 1000

// ============================================================================
// Decision Context - All information available to AI when making decisions
// ============================================================================

/**
 * DecisionContext holds complete state information available to an entity
 * when making a decision. This ensures transparency by capturing exactly
 * what information the AI had access to.
 */
typedef struct {
    // Entity Information
    int entity_id;
    char entity_name[MAX_ENTITY_NAME];
    char entity_type[MAX_ENTITY_TYPE];

    // World State
    int day_count;
    TimeOfDay time_of_day;
    Season season;
    int year;
    Weather weather;
    char weather_description[64];

    // Position & Location
    float position_x;
    float position_y;
    char location[64];

    // Needs (0-100, higher = more satisfied)
    float hunger;      // 0 = starving, 100 = full
    float energy;      // 0 = exhausted, 100 = well-rested
    float social;      // 0 = lonely, 100 = socially satisfied
    bool has_needs;    // Whether entity has needs component

    // Health
    int health_current;
    int health_max;
    bool has_health;

    // Resources
    int currency;
    bool has_currency;
    int inventory_item_count;
    int inventory_capacity;
    bool has_inventory;

    // Occupation & Skills
    char occupation[64];
    int skill_level;
    bool has_occupation;

    // Goals
    char current_goal[256];
    bool has_goal;

    // Relationships (nearby or relevant entities)
    int nearby_entity_ids[MAX_NEARBY_ENTITIES];
    char nearby_entity_names[MAX_NEARBY_ENTITIES][MAX_ENTITY_NAME];
    int nearby_relationship_values[MAX_NEARBY_ENTITIES];  // -100 to 100
    int nearby_entity_count;
    bool has_relationships;

    // Schedule Information
    char current_activity[128];
    int schedule_hour_start;
    int schedule_hour_end;
    bool has_schedule;

    // Recent Events (for context)
    int recent_event_count;
    uint64_t recent_event_ids[MAX_RECENT_EVENTS];  // Just IDs to avoid deep copy

    // Memory (recent significant events this entity remembers)
    int memory_count;
    bool has_memory;

} DecisionContext;

// Create decision context from game state and entity
DecisionContext* decision_context_create(const GameState* game_state,
                                         const Entity* entity,
                                         const EventLogger* event_logger);

// Create decision context with nearby entities filter
DecisionContext* decision_context_create_with_nearby(const GameState* game_state,
                                                     const Entity* entity,
                                                     const EventLogger* event_logger,
                                                     float nearby_radius);

// Destroy decision context
void decision_context_destroy(DecisionContext* context);

// Serialize to JSON (for logging and transparency)
cJSON* decision_context_to_json(const DecisionContext* context);

// Print decision context (for debugging)
void decision_context_print(const DecisionContext* context);

// ============================================================================
// Decision Options - Possible actions AI can choose from
// ============================================================================

typedef enum {
    DECISION_ACTION_MOVE,
    DECISION_ACTION_TALK,
    DECISION_ACTION_TRADE,
    DECISION_ACTION_WORK,
    DECISION_ACTION_REST,
    DECISION_ACTION_EAT,
    DECISION_ACTION_PLANT,
    DECISION_ACTION_HARVEST,
    DECISION_ACTION_WATER,
    DECISION_ACTION_GIVE_GIFT,
    DECISION_ACTION_WAIT,
    DECISION_ACTION_NONE,
    DECISION_ACTION_COUNT
} DecisionAction;

typedef struct {
    DecisionAction action;
    char description[256];
    float utility;          // Expected value of this action
    float cost;            // Resource cost
    float success_chance;  // Probability of success (0-1)
    int target_entity_id;  // Target entity (-1 if none)
    int target_item_id;    // Target item (-1 if none)
    float target_x;        // Target position
    float target_y;
} DecisionOption;

// ============================================================================
// Decision Record - Logged decision with full reasoning chain
// ============================================================================

typedef struct {
    uint64_t id;                                    // Unique decision ID
    time_t timestamp;                               // When decision was made
    int game_day;                                   // Game day
    char game_time[32];                             // Game time of day

    int entity_id;                                  // Who made the decision
    char entity_name[MAX_ENTITY_NAME];

    DecisionContext* context;                       // Full context at decision time

    DecisionOption options[MAX_DECISION_OPTIONS];   // All options considered
    int option_count;

    int chosen_option_index;                        // Which option was chosen
    DecisionAction chosen_action;

    char reasoning[MAX_DECISION_REASONING];         // Why this option was chosen

    // Evaluation (filled in after action completes)
    bool executed;
    bool succeeded;
    float actual_utility;  // Actual outcome
    char outcome_description[256];

} DecisionRecord;

// Create decision record
DecisionRecord* decision_record_create(const DecisionContext* context,
                                       const DecisionOption* options,
                                       int option_count,
                                       int chosen_index,
                                       const char* reasoning);

// Destroy decision record
void decision_record_destroy(DecisionRecord* record);

// Update decision record with execution results
void decision_record_set_outcome(DecisionRecord* record,
                                bool succeeded,
                                float actual_utility,
                                const char* outcome_description);

// Serialize to JSON
cJSON* decision_record_to_json(const DecisionRecord* record);

// Print decision record (for debugging and transparency UI)
void decision_record_print(const DecisionRecord* record);

const char* decision_action_to_string(DecisionAction action);

// ============================================================================
// Decision Logger - Logs all decisions for transparency
// ============================================================================

typedef struct {
    DecisionRecord* decisions[MAX_DECISION_LOG_SIZE];
    int decision_count;
    int head;  // Ring buffer head
    int tail;  // Ring buffer tail
    bool full;
    uint64_t next_decision_id;

    // Statistics
    int decisions_by_action[DECISION_ACTION_COUNT];
    int total_decisions;
    int successful_decisions;
    int failed_decisions;

} DecisionLogger;

// Create decision logger
DecisionLogger* decision_logger_create(void);

// Destroy decision logger
void decision_logger_destroy(DecisionLogger* logger);

// Log a decision
void decision_logger_log(DecisionLogger* logger, DecisionRecord* record);

// Query decisions
int decision_logger_get_recent(const DecisionLogger* logger,
                               DecisionRecord** out_decisions,
                               int max_decisions);

int decision_logger_get_by_entity(const DecisionLogger* logger,
                                  int entity_id,
                                  DecisionRecord** out_decisions,
                                  int max_decisions);

int decision_logger_get_by_day(const DecisionLogger* logger,
                               int day,
                               DecisionRecord** out_decisions,
                               int max_decisions);

int decision_logger_get_by_action(const DecisionLogger* logger,
                                  DecisionAction action,
                                  DecisionRecord** out_decisions,
                                  int max_decisions);

// Get statistics
void decision_logger_get_stats(const DecisionLogger* logger,
                              int* total_decisions,
                              int* successful,
                              int* failed,
                              int* by_action);

// Clear log
void decision_logger_clear(DecisionLogger* logger);

// Serialize to JSON
cJSON* decision_logger_to_json(const DecisionLogger* logger);

#endif // DECISION_H
