/**
 * event.c
 * Event System Implementation
 */

#include "event.h"
#include "../../lib/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// Event Creation
// ============================================================================

GameEvent* event_create(EventType type, EventSubtype subtype, int source_entity_id, const char* description) {
    GameEvent* event = (GameEvent*)malloc(sizeof(GameEvent));
    if (!event) return NULL;

    event->id = 0;  // Will be set by event bus
    event->type = type;
    event->subtype = subtype;
    event->timestamp = time(NULL);
    event->game_day = 0;
    event->game_time[0] = '\0';
    event->source_entity_id = source_entity_id;
    event->target_entity_id = -1;
    event->location[0] = '\0';
    event->data = NULL;

    if (description) {
        strncpy(event->description, description, MAX_EVENT_DESCRIPTION - 1);
        event->description[MAX_EVENT_DESCRIPTION - 1] = '\0';
    } else {
        event->description[0] = '\0';
    }

    return event;
}

void event_destroy(GameEvent* event) {
    if (!event) return;

    // Free event-specific data if it exists
    if (event->data) {
        free(event->data);
    }

    free(event);
}

GameEvent* event_create_trade(int source_id, int target_id, const char* item_name,
                               int quantity, int price, bool accepted, const char* reason) {
    GameEvent* event = event_create(
        EVENT_ECONOMIC,
        accepted ? ECONOMIC_TRADE_ACCEPTED : ECONOMIC_TRADE_DECLINED,
        source_id,
        NULL
    );
    if (!event) return NULL;

    event->target_entity_id = target_id;

    // Create description
    snprintf(event->description, MAX_EVENT_DESCRIPTION,
             "Trade %s: %d %s for %d gold. %s",
             accepted ? "accepted" : "declined",
             quantity, item_name, price, reason);

    // Allocate trade-specific data
    TradeEventData* data = (TradeEventData*)malloc(sizeof(TradeEventData));
    if (data) {
        data->item_id = 0;  // Could be set if we have item IDs
        strncpy(data->item_name, item_name, sizeof(data->item_name) - 1);
        data->item_name[sizeof(data->item_name) - 1] = '\0';
        data->quantity = quantity;
        data->offered_price = price;
        data->asking_price = price;
        data->accepted = accepted;
        strncpy(data->reason, reason, sizeof(data->reason) - 1);
        data->reason[sizeof(data->reason) - 1] = '\0';
        event->data = data;
    }

    return event;
}

GameEvent* event_create_relationship_change(int source_id, int target_id,
                                             int old_value, int new_value, const char* reason) {
    GameEvent* event = event_create(
        EVENT_SOCIAL,
        SOCIAL_RELATIONSHIP_CHANGED,
        source_id,
        NULL
    );
    if (!event) return NULL;

    event->target_entity_id = target_id;

    int delta = new_value - old_value;
    snprintf(event->description, MAX_EVENT_DESCRIPTION,
             "Relationship changed: %d -> %d (%+d). %s",
             old_value, new_value, delta, reason);

    RelationshipEventData* data = (RelationshipEventData*)malloc(sizeof(RelationshipEventData));
    if (data) {
        data->relationship_before = old_value;
        data->relationship_after = new_value;
        data->delta = delta;
        strncpy(data->reason, reason, sizeof(data->reason) - 1);
        data->reason[sizeof(data->reason) - 1] = '\0';
        event->data = data;
    }

    return event;
}

GameEvent* event_create_crop_action(EventSubtype subtype, const char* crop_type,
                                     int x, int y, int source_id) {
    GameEvent* event = event_create(EVENT_AGRICULTURAL, subtype, source_id, NULL);
    if (!event) return NULL;

    const char* action = "acted on";
    if (subtype == AGRICULTURAL_CROP_PLANTED) action = "planted";
    else if (subtype == AGRICULTURAL_CROP_WATERED) action = "watered";
    else if (subtype == AGRICULTURAL_CROP_HARVESTED) action = "harvested";
    else if (subtype == AGRICULTURAL_CROP_WITHERED) action = "withered";

    snprintf(event->description, MAX_EVENT_DESCRIPTION,
             "%s %s at (%d, %d)", action, crop_type, x, y);

    CropEventData* data = (CropEventData*)malloc(sizeof(CropEventData));
    if (data) {
        strncpy(data->crop_type, crop_type, sizeof(data->crop_type) - 1);
        data->crop_type[sizeof(data->crop_type) - 1] = '\0';
        data->plot_x = x;
        data->plot_y = y;
        data->growth_stage = 0;
        data->days_to_maturity = 0;
        event->data = data;
    }

    return event;
}

GameEvent* event_create_weather_change(const char* from, const char* to) {
    GameEvent* event = event_create(EVENT_ENVIRONMENTAL, ENVIRONMENTAL_WEATHER_CHANGED, -1, NULL);
    if (!event) return NULL;

    snprintf(event->description, MAX_EVENT_DESCRIPTION,
             "Weather changed from %s to %s", from, to);

    WeatherEventData* data = (WeatherEventData*)malloc(sizeof(WeatherEventData));
    if (data) {
        strncpy(data->from_weather, from, sizeof(data->from_weather) - 1);
        data->from_weather[sizeof(data->from_weather) - 1] = '\0';
        strncpy(data->to_weather, to, sizeof(data->to_weather) - 1);
        data->to_weather[sizeof(data->to_weather) - 1] = '\0';
        data->temperature = 0.0f;
        data->rainfall = 0.0f;
        event->data = data;
    }

    return event;
}

GameEvent* event_create_currency(int entity_id, int amount, const char* reason) {
    GameEvent* event = event_create(
        EVENT_ECONOMIC,
        amount >= 0 ? ECONOMIC_CURRENCY_GAINED : ECONOMIC_CURRENCY_SPENT,
        entity_id,
        NULL
    );
    if (!event) return NULL;

    snprintf(event->description, MAX_EVENT_DESCRIPTION,
             "%s %d gold. %s",
             amount >= 0 ? "Gained" : "Spent",
             abs(amount), reason);

    CurrencyEventData* data = (CurrencyEventData*)malloc(sizeof(CurrencyEventData));
    if (data) {
        data->amount = amount;
        strncpy(data->reason, reason, sizeof(data->reason) - 1);
        data->reason[sizeof(data->reason) - 1] = '\0';
        event->data = data;
    }

    return event;
}

GameEvent* event_create_time_advance(EventSubtype subtype, int day, const char* time_of_day) {
    GameEvent* event = event_create(EVENT_TIME, subtype, -1, NULL);
    if (!event) return NULL;

    event->game_day = day;
    strncpy(event->game_time, time_of_day, sizeof(event->game_time) - 1);
    event->game_time[sizeof(event->game_time) - 1] = '\0';

    snprintf(event->description, MAX_EVENT_DESCRIPTION,
             "Time advanced to day %d, %s", day, time_of_day);

    return event;
}

// ============================================================================
// Event Type String Conversion
// ============================================================================

const char* event_type_to_string(EventType type) {
    switch (type) {
        case EVENT_ECONOMIC: return "Economic";
        case EVENT_SOCIAL: return "Social";
        case EVENT_AGRICULTURAL: return "Agricultural";
        case EVENT_ENVIRONMENTAL: return "Environmental";
        case EVENT_TIME: return "Time";
        case EVENT_SYSTEM: return "System";
        default: return "Unknown";
    }
}

const char* event_subtype_to_string(EventSubtype subtype) {
    switch (subtype) {
        case ECONOMIC_TRADE_OFFERED: return "TradeOffered";
        case ECONOMIC_TRADE_ACCEPTED: return "TradeAccepted";
        case ECONOMIC_TRADE_DECLINED: return "TradeDeclined";
        case ECONOMIC_CURRENCY_GAINED: return "CurrencyGained";
        case ECONOMIC_CURRENCY_SPENT: return "CurrencySpent";
        case SOCIAL_RELATIONSHIP_CHANGED: return "RelationshipChanged";
        case SOCIAL_GIFT_GIVEN: return "GiftGiven";
        case AGRICULTURAL_CROP_PLANTED: return "CropPlanted";
        case AGRICULTURAL_CROP_HARVESTED: return "CropHarvested";
        case ENVIRONMENTAL_WEATHER_CHANGED: return "WeatherChanged";
        case TIME_SUBTYPE_NEW_DAY: return "NewDay";
        case SYSTEM_ENTITY_CREATED: return "EntityCreated";
        default: return "Unknown";
    }
}

// ============================================================================
// Event Serialization
// ============================================================================

cJSON* event_to_json(const GameEvent* event) {
    if (!event) return NULL;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "id", (double)event->id);
    cJSON_AddStringToObject(json, "type", event_type_to_string(event->type));
    cJSON_AddStringToObject(json, "subtype", event_subtype_to_string(event->subtype));
    cJSON_AddNumberToObject(json, "timestamp", (double)event->timestamp);
    cJSON_AddNumberToObject(json, "game_day", event->game_day);
    cJSON_AddStringToObject(json, "game_time", event->game_time);
    cJSON_AddNumberToObject(json, "source_entity_id", event->source_entity_id);
    cJSON_AddNumberToObject(json, "target_entity_id", event->target_entity_id);
    cJSON_AddStringToObject(json, "location", event->location);
    cJSON_AddStringToObject(json, "description", event->description);

    // TODO: Serialize event-specific data based on type
    // For now, just store the basic event info

    return json;
}

GameEvent* event_from_json(cJSON* json) {
    if (!json) return NULL;

    GameEvent* event = (GameEvent*)malloc(sizeof(GameEvent));
    if (!event) return NULL;

    cJSON* id = cJSON_GetObjectItem(json, "id");
    if (id) event->id = (uint64_t)id->valuedouble;

    // Note: Type string parsing would go here
    event->type = EVENT_SYSTEM;
    event->subtype = 0;

    cJSON* timestamp = cJSON_GetObjectItem(json, "timestamp");
    if (timestamp) event->timestamp = (time_t)timestamp->valuedouble;

    cJSON* game_day = cJSON_GetObjectItem(json, "game_day");
    if (game_day) event->game_day = game_day->valueint;

    cJSON* game_time = cJSON_GetObjectItem(json, "game_time");
    if (game_time && cJSON_IsString(game_time)) {
        strncpy(event->game_time, game_time->valuestring, sizeof(event->game_time) - 1);
    }

    cJSON* source = cJSON_GetObjectItem(json, "source_entity_id");
    if (source) event->source_entity_id = source->valueint;

    cJSON* target = cJSON_GetObjectItem(json, "target_entity_id");
    if (target) event->target_entity_id = target->valueint;

    cJSON* location = cJSON_GetObjectItem(json, "location");
    if (location && cJSON_IsString(location)) {
        strncpy(event->location, location->valuestring, sizeof(event->location) - 1);
    }

    cJSON* description = cJSON_GetObjectItem(json, "description");
    if (description && cJSON_IsString(description)) {
        strncpy(event->description, description->valuestring, sizeof(event->description) - 1);
    }

    event->data = NULL;

    return event;
}

// ============================================================================
// Event Bus Implementation
// ============================================================================

EventBus* event_bus_create(void) {
    EventBus* bus = (EventBus*)malloc(sizeof(EventBus));
    if (!bus) return NULL;

    bus->subscriber_count = 0;
    bus->next_event_id = 1;

    for (int i = 0; i < MAX_EVENT_SUBSCRIBERS; i++) {
        bus->subscribers[i].callback = NULL;
        bus->subscribers[i].user_data = NULL;
        bus->subscribers[i].filter_type = -1;
        bus->subscribers[i].active = false;
    }

    return bus;
}

void event_bus_destroy(EventBus* bus) {
    free(bus);
}

int event_bus_subscribe(EventBus* bus, EventCallback callback, void* user_data, int filter_type) {
    if (!bus || !callback) return -1;

    // Find free slot
    for (int i = 0; i < MAX_EVENT_SUBSCRIBERS; i++) {
        if (!bus->subscribers[i].active) {
            bus->subscribers[i].callback = callback;
            bus->subscribers[i].user_data = user_data;
            bus->subscribers[i].filter_type = filter_type;
            bus->subscribers[i].active = true;
            bus->subscriber_count++;
            return i;
        }
    }

    return -1;  // No free slots
}

bool event_bus_unsubscribe(EventBus* bus, int subscriber_id) {
    if (!bus || subscriber_id < 0 || subscriber_id >= MAX_EVENT_SUBSCRIBERS) {
        return false;
    }

    if (bus->subscribers[subscriber_id].active) {
        bus->subscribers[subscriber_id].active = false;
        bus->subscriber_count--;
        return true;
    }

    return false;
}

void event_bus_publish(EventBus* bus, GameEvent* event) {
    if (!bus || !event) return;

    // Assign event ID
    event->id = bus->next_event_id++;

    // Notify all subscribers
    for (int i = 0; i < MAX_EVENT_SUBSCRIBERS; i++) {
        EventSubscriber* sub = &bus->subscribers[i];
        if (sub->active && sub->callback) {
            // Check filter (-1 means all events)
            if (sub->filter_type < 0 || sub->filter_type == (int)event->type) {
                sub->callback(event, sub->user_data);
            }
        }
    }
}

void event_bus_clear(EventBus* bus) {
    if (!bus) return;

    for (int i = 0; i < MAX_EVENT_SUBSCRIBERS; i++) {
        bus->subscribers[i].active = false;
    }
    bus->subscriber_count = 0;
}

// ============================================================================
// Event Logger Implementation
// ============================================================================

EventLogger* event_logger_create(void) {
    EventLogger* logger = (EventLogger*)malloc(sizeof(EventLogger));
    if (!logger) return NULL;

    logger->event_count = 0;
    logger->head = 0;
    logger->tail = 0;
    logger->full = false;
    logger->total_events_logged = 0;

    for (int i = 0; i < MAX_EVENT_LOG_SIZE; i++) {
        logger->events[i] = NULL;
    }

    for (int i = 0; i < EVENT_TYPE_COUNT; i++) {
        logger->events_by_type[i] = 0;
    }

    return logger;
}

void event_logger_destroy(EventLogger* logger) {
    if (!logger) return;

    // Free all logged events
    for (int i = 0; i < MAX_EVENT_LOG_SIZE; i++) {
        if (logger->events[i]) {
            event_destroy(logger->events[i]);
        }
    }

    free(logger);
}

void event_logger_log(EventLogger* logger, const GameEvent* event) {
    if (!logger || !event) return;

    // Create a copy of the event
    GameEvent* event_copy = (GameEvent*)malloc(sizeof(GameEvent));
    if (!event_copy) return;

    memcpy(event_copy, event, sizeof(GameEvent));
    event_copy->data = NULL;  // Don't copy data pointer (we'd need deep copy)

    // Free old event if buffer is full
    if (logger->full && logger->events[logger->head]) {
        event_destroy(logger->events[logger->head]);
    }

    // Add to ring buffer
    logger->events[logger->head] = event_copy;
    logger->head = (logger->head + 1) % MAX_EVENT_LOG_SIZE;

    if (logger->full) {
        logger->tail = (logger->tail + 1) % MAX_EVENT_LOG_SIZE;
    }

    if (logger->head == logger->tail) {
        logger->full = true;
    }

    if (!logger->full) {
        logger->event_count++;
    }

    // Update statistics
    logger->total_events_logged++;
    if (event->type >= 0 && event->type < EVENT_TYPE_COUNT) {
        logger->events_by_type[event->type]++;
    }
}

int event_logger_get_recent(const EventLogger* logger, GameEvent** out_events, int max_events) {
    if (!logger || !out_events) return 0;

    int count = 0;
    int items = logger->full ? MAX_EVENT_LOG_SIZE : logger->event_count;

    // Get most recent events (iterate backwards)
    for (int i = 0; i < items && count < max_events; i++) {
        int idx = (logger->head - 1 - i + MAX_EVENT_LOG_SIZE) % MAX_EVENT_LOG_SIZE;
        if (logger->events[idx]) {
            out_events[count++] = logger->events[idx];
        }
    }

    return count;
}

int event_logger_get_by_type(const EventLogger* logger, EventType type,
                              GameEvent** out_events, int max_events) {
    if (!logger || !out_events) return 0;

    int count = 0;
    int items = logger->full ? MAX_EVENT_LOG_SIZE : logger->event_count;

    for (int i = 0; i < items && count < max_events; i++) {
        int idx = (logger->tail + i) % MAX_EVENT_LOG_SIZE;
        if (logger->events[idx] && logger->events[idx]->type == type) {
            out_events[count++] = logger->events[idx];
        }
    }

    return count;
}

int event_logger_get_by_entity(const EventLogger* logger, int entity_id,
                                GameEvent** out_events, int max_events) {
    if (!logger || !out_events) return 0;

    int count = 0;
    int items = logger->full ? MAX_EVENT_LOG_SIZE : logger->event_count;

    for (int i = 0; i < items && count < max_events; i++) {
        int idx = (logger->tail + i) % MAX_EVENT_LOG_SIZE;
        GameEvent* event = logger->events[idx];
        if (event && (event->source_entity_id == entity_id || event->target_entity_id == entity_id)) {
            out_events[count++] = event;
        }
    }

    return count;
}

int event_logger_get_by_day(const EventLogger* logger, int day,
                             GameEvent** out_events, int max_events) {
    if (!logger || !out_events) return 0;

    int count = 0;
    int items = logger->full ? MAX_EVENT_LOG_SIZE : logger->event_count;

    for (int i = 0; i < items && count < max_events; i++) {
        int idx = (logger->tail + i) % MAX_EVENT_LOG_SIZE;
        if (logger->events[idx] && logger->events[idx]->game_day == day) {
            out_events[count++] = logger->events[idx];
        }
    }

    return count;
}

void event_logger_get_stats(const EventLogger* logger, int* total_events, int* events_by_type) {
    if (!logger) return;

    if (total_events) {
        *total_events = logger->total_events_logged;
    }

    if (events_by_type) {
        for (int i = 0; i < EVENT_TYPE_COUNT; i++) {
            events_by_type[i] = logger->events_by_type[i];
        }
    }
}

void event_logger_clear(EventLogger* logger) {
    if (!logger) return;

    for (int i = 0; i < MAX_EVENT_LOG_SIZE; i++) {
        if (logger->events[i]) {
            event_destroy(logger->events[i]);
            logger->events[i] = NULL;
        }
    }

    logger->event_count = 0;
    logger->head = 0;
    logger->tail = 0;
    logger->full = false;
    logger->total_events_logged = 0;

    for (int i = 0; i < EVENT_TYPE_COUNT; i++) {
        logger->events_by_type[i] = 0;
    }
}

cJSON* event_logger_to_json(const EventLogger* logger) {
    if (!logger) return NULL;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "total_events_logged", logger->total_events_logged);
    cJSON_AddNumberToObject(json, "current_event_count", logger->event_count);

    cJSON* events_array = cJSON_CreateArray();
    int items = logger->full ? MAX_EVENT_LOG_SIZE : logger->event_count;

    for (int i = 0; i < items; i++) {
        int idx = (logger->tail + i) % MAX_EVENT_LOG_SIZE;
        if (logger->events[idx]) {
            cJSON* event_json = event_to_json(logger->events[idx]);
            if (event_json) {
                cJSON_AddItemToArray(events_array, event_json);
            }
        }
    }

    cJSON_AddItemToObject(json, "events", events_array);

    return json;
}

EventLogger* event_logger_from_json(cJSON* json) {
    if (!json) return NULL;

    EventLogger* logger = event_logger_create();
    if (!logger) return NULL;

    cJSON* total = cJSON_GetObjectItem(json, "total_events_logged");
    if (total) logger->total_events_logged = total->valueint;

    cJSON* events_array = cJSON_GetObjectItem(json, "events");
    if (events_array && cJSON_IsArray(events_array)) {
        cJSON* event_json = NULL;
        cJSON_ArrayForEach(event_json, events_array) {
            GameEvent* event = event_from_json(event_json);
            if (event) {
                event_logger_log(logger, event);
                event_destroy(event);  // log makes a copy
            }
        }
    }

    return logger;
}

void event_print(const GameEvent* event) {
    if (!event) return;

    printf("Event #%lu: [%s/%s] %s\n",
           event->id,
           event_type_to_string(event->type),
           event_subtype_to_string(event->subtype),
           event->description);

    if (event->source_entity_id >= 0) {
        printf("  Source Entity: %d\n", event->source_entity_id);
    }
    if (event->target_entity_id >= 0) {
        printf("  Target Entity: %d\n", event->target_entity_id);
    }
    if (event->location[0] != '\0') {
        printf("  Location: %s\n", event->location);
    }
    if (event->game_day > 0) {
        printf("  Game Time: Day %d, %s\n", event->game_day, event->game_time);
    }
}
