/**
 * event.h
 * Event System for Transparent Game Engine
 *
 * Provides message-passing infrastructure for observable actions.
 * Enables loose coupling and creates audit trail for AI transparency.
 */

#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// Forward declarations
typedef struct cJSON cJSON;

#define MAX_EVENT_DESCRIPTION 256
#define MAX_EVENT_LOCATION 64
#define MAX_EVENT_SUBSCRIBERS 100
#define MAX_EVENT_LOG_SIZE 10000

// Event Types
typedef enum {
    EVENT_ECONOMIC,      // Trading, currency changes, market events
    EVENT_SOCIAL,        // Conversations, relationship changes, interactions
    EVENT_AGRICULTURAL,  // Planting, harvesting, crop growth
    EVENT_ENVIRONMENTAL, // Weather changes, time progression
    EVENT_TIME,          // Time of day changes, season changes
    EVENT_SYSTEM,        // Game state changes, entity creation/destruction
    EVENT_TYPE_COUNT
} EventType;

// Event Subtypes for more specific categorization
typedef enum {
    // Economic subtypes
    ECONOMIC_TRADE_OFFERED,
    ECONOMIC_TRADE_ACCEPTED,
    ECONOMIC_TRADE_DECLINED,
    ECONOMIC_CURRENCY_GAINED,
    ECONOMIC_CURRENCY_SPENT,
    ECONOMIC_PRICE_CHANGED,

    // Social subtypes
    SOCIAL_CONVERSATION_STARTED,
    SOCIAL_CONVERSATION_ENDED,
    SOCIAL_RELATIONSHIP_CHANGED,
    SOCIAL_GIFT_GIVEN,
    SOCIAL_HELP_REQUESTED,
    SOCIAL_HELP_PROVIDED,

    // Agricultural subtypes
    AGRICULTURAL_CROP_PLANTED,
    AGRICULTURAL_CROP_WATERED,
    AGRICULTURAL_CROP_HARVESTED,
    AGRICULTURAL_CROP_WITHERED,
    AGRICULTURAL_CROP_GROWTH_STAGE,

    // Environmental subtypes
    ENVIRONMENTAL_WEATHER_CHANGED,
    ENVIRONMENTAL_TIME_ADVANCED,
    ENVIRONMENTAL_SEASON_CHANGED,
    ENVIRONMENTAL_DAY_STARTED,

    // Time subtypes
    TIME_SUBTYPE_MORNING_STARTED,
    TIME_SUBTYPE_AFTERNOON_STARTED,
    TIME_SUBTYPE_EVENING_STARTED,
    TIME_SUBTYPE_NIGHT_STARTED,
    TIME_SUBTYPE_NEW_DAY,
    TIME_SUBTYPE_NEW_SEASON,
    TIME_SUBTYPE_NEW_YEAR,

    // System subtypes
    SYSTEM_ENTITY_CREATED,
    SYSTEM_ENTITY_DESTROYED,
    SYSTEM_GAME_SAVED,
    SYSTEM_GAME_LOADED,

    EVENT_SUBTYPE_COUNT
} EventSubtype;

// Base Event Structure
typedef struct {
    uint64_t id;                           // Unique event ID
    EventType type;                        // Main event type
    EventSubtype subtype;                  // Specific event subtype
    time_t timestamp;                      // When the event occurred
    int game_day;                          // Game day when event occurred
    char game_time[32];                    // Game time of day
    int source_entity_id;                  // Entity that triggered the event (-1 if none)
    int target_entity_id;                  // Entity affected by the event (-1 if none)
    char location[MAX_EVENT_LOCATION];     // Where the event occurred
    char description[MAX_EVENT_DESCRIPTION]; // Human-readable description
    void* data;                            // Event-specific data (must be freed by handler)
} GameEvent;

// Event-specific data structures

typedef struct {
    int item_id;
    char item_name[64];
    int quantity;
    int offered_price;
    int asking_price;
    bool accepted;
    char reason[128];  // Why trade was accepted/declined
} TradeEventData;

typedef struct {
    int relationship_before;
    int relationship_after;
    int delta;
    char reason[128];  // Why relationship changed
} RelationshipEventData;

typedef struct {
    char crop_type[64];
    int plot_x;
    int plot_y;
    int growth_stage;
    int days_to_maturity;
} CropEventData;

typedef struct {
    char from_weather[32];
    char to_weather[32];
    float temperature;
    float rainfall;
} WeatherEventData;

typedef struct {
    int amount;
    char reason[128];
} CurrencyEventData;

// ============================================================================
// Event Creation Functions
// ============================================================================

GameEvent* event_create(EventType type, EventSubtype subtype, int source_entity_id, const char* description);
void event_destroy(GameEvent* event);

// Specific event creators
GameEvent* event_create_trade(int source_id, int target_id, const char* item_name, int quantity, int price, bool accepted, const char* reason);
GameEvent* event_create_relationship_change(int source_id, int target_id, int old_value, int new_value, const char* reason);
GameEvent* event_create_crop_action(EventSubtype subtype, const char* crop_type, int x, int y, int source_id);
GameEvent* event_create_weather_change(const char* from, const char* to);
GameEvent* event_create_currency(int entity_id, int amount, const char* reason);
GameEvent* event_create_time_advance(EventSubtype subtype, int day, const char* time_of_day);

cJSON* event_to_json(const GameEvent* event);
GameEvent* event_from_json(cJSON* json);

const char* event_type_to_string(EventType type);
const char* event_subtype_to_string(EventSubtype subtype);

// ============================================================================
// Event Bus (Publisher-Subscriber Pattern)
// ============================================================================

typedef void (*EventCallback)(const GameEvent* event, void* user_data);

typedef struct {
    EventCallback callback;
    void* user_data;
    int filter_type;     // Only receive events of this type (-1 for all)
    bool active;
} EventSubscriber;

typedef struct {
    EventSubscriber subscribers[MAX_EVENT_SUBSCRIBERS];
    int subscriber_count;
    uint64_t next_event_id;
} EventBus;

EventBus* event_bus_create(void);
void event_bus_destroy(EventBus* bus);

// Subscribe to events
int event_bus_subscribe(EventBus* bus, EventCallback callback, void* user_data, int filter_type);
bool event_bus_unsubscribe(EventBus* bus, int subscriber_id);

// Publish events
void event_bus_publish(EventBus* bus, GameEvent* event);

// Clear all subscribers
void event_bus_clear(EventBus* bus);

// ============================================================================
// Event Logger (For Transparency and Debugging)
// ============================================================================

typedef struct {
    GameEvent* events[MAX_EVENT_LOG_SIZE];
    int event_count;
    int head;  // Ring buffer head
    int tail;  // Ring buffer tail
    bool full;

    // Statistics
    int events_by_type[EVENT_TYPE_COUNT];
    int total_events_logged;
} EventLogger;

EventLogger* event_logger_create(void);
void event_logger_destroy(EventLogger* logger);

// Log an event
void event_logger_log(EventLogger* logger, const GameEvent* event);

// Query events
int event_logger_get_recent(const EventLogger* logger, GameEvent** out_events, int max_events);
int event_logger_get_by_type(const EventLogger* logger, EventType type, GameEvent** out_events, int max_events);
int event_logger_get_by_entity(const EventLogger* logger, int entity_id, GameEvent** out_events, int max_events);
int event_logger_get_by_day(const EventLogger* logger, int day, GameEvent** out_events, int max_events);

// Get statistics
void event_logger_get_stats(const EventLogger* logger, int* total_events, int* events_by_type);

// Clear log
void event_logger_clear(EventLogger* logger);

// Save/load event log
cJSON* event_logger_to_json(const EventLogger* logger);
EventLogger* event_logger_from_json(cJSON* json);

// Print event (for debugging)
void event_print(const GameEvent* event);

#endif // EVENT_H
