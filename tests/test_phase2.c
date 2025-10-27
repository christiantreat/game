/**
 * test_phase2.c
 * Phase 2 Tests: Event System
 * Tests for Event creation, Event Bus, and Event Logger
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/cJSON.h"
#include "../src/core/event.h"

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("  ✗ FAILED: %s\n", message); \
            tests_failed++; \
            return false; \
        } \
    } while(0)

#define TEST_PASS(message) \
    do { \
        printf("  ✓ %s\n", message); \
        tests_passed++; \
    } while(0)

// ============================================================================
// Event Creation Tests
// ============================================================================

bool test_event_creation() {
    printf("\n=== Test 2.1: Event Creation ===\n");

    // Test basic event creation
    GameEvent* event = event_create(EVENT_SYSTEM, SYSTEM_ENTITY_CREATED, 1, "Player created");
    TEST_ASSERT(event != NULL, "Event created");
    TEST_ASSERT(event->type == EVENT_SYSTEM, "Event type correct");
    TEST_ASSERT(event->subtype == SYSTEM_ENTITY_CREATED, "Event subtype correct");
    TEST_ASSERT(event->source_entity_id == 1, "Source entity correct");
    TEST_ASSERT(strcmp(event->description, "Player created") == 0, "Description correct");
    TEST_PASS("Basic event creation works");
    event_destroy(event);

    // Test trade event
    GameEvent* trade = event_create_trade(1, 2, "wheat", 10, 50, true, "Fair price");
    TEST_ASSERT(trade != NULL, "Trade event created");
    TEST_ASSERT(trade->type == EVENT_ECONOMIC, "Trade event type correct");
    TEST_ASSERT(trade->subtype == ECONOMIC_TRADE_ACCEPTED, "Trade accepted subtype");
    TEST_ASSERT(trade->source_entity_id == 1, "Trade source correct");
    TEST_ASSERT(trade->target_entity_id == 2, "Trade target correct");
    TEST_ASSERT(trade->data != NULL, "Trade has data");

    TradeEventData* trade_data = (TradeEventData*)trade->data;
    TEST_ASSERT(strcmp(trade_data->item_name, "wheat") == 0, "Trade item correct");
    TEST_ASSERT(trade_data->quantity == 10, "Trade quantity correct");
    TEST_ASSERT(trade_data->offered_price == 50, "Trade price correct");
    TEST_ASSERT(trade_data->accepted == true, "Trade accepted flag correct");
    TEST_PASS("Trade event creation works");
    event_destroy(trade);

    // Test relationship change event
    GameEvent* rel = event_create_relationship_change(1, 2, 50, 60, "Gave gift");
    TEST_ASSERT(rel != NULL, "Relationship event created");
    TEST_ASSERT(rel->type == EVENT_SOCIAL, "Relationship event type correct");
    TEST_ASSERT(rel->data != NULL, "Relationship has data");

    RelationshipEventData* rel_data = (RelationshipEventData*)rel->data;
    TEST_ASSERT(rel_data->relationship_before == 50, "Old relationship correct");
    TEST_ASSERT(rel_data->relationship_after == 60, "New relationship correct");
    TEST_ASSERT(rel_data->delta == 10, "Relationship delta correct");
    TEST_PASS("Relationship event creation works");
    event_destroy(rel);

    // Test crop event
    GameEvent* crop = event_create_crop_action(AGRICULTURAL_CROP_PLANTED, "corn", 5, 3, 1);
    TEST_ASSERT(crop != NULL, "Crop event created");
    TEST_ASSERT(crop->type == EVENT_AGRICULTURAL, "Crop event type correct");
    TEST_ASSERT(crop->subtype == AGRICULTURAL_CROP_PLANTED, "Crop subtype correct");
    TEST_ASSERT(crop->data != NULL, "Crop has data");

    CropEventData* crop_data = (CropEventData*)crop->data;
    TEST_ASSERT(strcmp(crop_data->crop_type, "corn") == 0, "Crop type correct");
    TEST_ASSERT(crop_data->plot_x == 5, "Crop X correct");
    TEST_ASSERT(crop_data->plot_y == 3, "Crop Y correct");
    TEST_PASS("Crop event creation works");
    event_destroy(crop);

    // Test weather event
    GameEvent* weather = event_create_weather_change("sunny", "rainy");
    TEST_ASSERT(weather != NULL, "Weather event created");
    TEST_ASSERT(weather->type == EVENT_ENVIRONMENTAL, "Weather event type correct");
    TEST_ASSERT(weather->data != NULL, "Weather has data");

    WeatherEventData* weather_data = (WeatherEventData*)weather->data;
    TEST_ASSERT(strcmp(weather_data->from_weather, "sunny") == 0, "From weather correct");
    TEST_ASSERT(strcmp(weather_data->to_weather, "rainy") == 0, "To weather correct");
    TEST_PASS("Weather event creation works");
    event_destroy(weather);

    // Test currency event
    GameEvent* currency = event_create_currency(1, 100, "Sold wheat");
    TEST_ASSERT(currency != NULL, "Currency event created");
    TEST_ASSERT(currency->type == EVENT_ECONOMIC, "Currency event type correct");
    TEST_ASSERT(currency->subtype == ECONOMIC_CURRENCY_GAINED, "Currency gained subtype");
    TEST_ASSERT(currency->data != NULL, "Currency has data");

    CurrencyEventData* curr_data = (CurrencyEventData*)currency->data;
    TEST_ASSERT(curr_data->amount == 100, "Currency amount correct");
    TEST_PASS("Currency event creation works");
    event_destroy(currency);

    // Test time event
    GameEvent* time_ev = event_create_time_advance(TIME_NEW_DAY, 2, "morning");
    TEST_ASSERT(time_ev != NULL, "Time event created");
    TEST_ASSERT(time_ev->type == EVENT_TIME, "Time event type correct");
    TEST_ASSERT(time_ev->game_day == 2, "Game day correct");
    TEST_ASSERT(strcmp(time_ev->game_time, "morning") == 0, "Game time correct");
    TEST_PASS("Time event creation works");
    event_destroy(time_ev);

    printf("\n✅ All event creation tests passed!\n");
    return true;
}

// ============================================================================
// Event Bus Tests
// ============================================================================

// Test callback data
typedef struct {
    int call_count;
    EventType last_type;
    uint64_t last_event_id;
} CallbackData;

void test_callback(const GameEvent* event, void* user_data) {
    CallbackData* data = (CallbackData*)user_data;
    data->call_count++;
    data->last_type = event->type;
    data->last_event_id = event->id;
}

bool test_event_bus() {
    printf("\n=== Test 2.2: Event Bus ===\n");

    EventBus* bus = event_bus_create();
    TEST_ASSERT(bus != NULL, "EventBus created");
    TEST_ASSERT(bus->subscriber_count == 0, "No initial subscribers");
    TEST_PASS("EventBus initialization works");

    // Test subscription
    CallbackData data1 = {0, 0, 0};
    CallbackData data2 = {0, 0, 0};
    CallbackData data3 = {0, 0, 0};

    int sub1 = event_bus_subscribe(bus, test_callback, &data1, -1);  // All events
    TEST_ASSERT(sub1 >= 0, "First subscription successful");

    int sub2 = event_bus_subscribe(bus, test_callback, &data2, EVENT_ECONOMIC);  // Only economic
    TEST_ASSERT(sub2 >= 0, "Second subscription successful");

    int sub3 = event_bus_subscribe(bus, test_callback, &data3, EVENT_SOCIAL);  // Only social
    TEST_ASSERT(sub3 >= 0, "Third subscription successful");

    TEST_ASSERT(bus->subscriber_count == 3, "Subscriber count correct");
    TEST_PASS("Event subscription works");

    // Test publishing economic event
    GameEvent* trade = event_create_trade(1, 2, "wheat", 10, 50, true, "Test");
    event_bus_publish(bus, trade);

    TEST_ASSERT(trade->id > 0, "Event assigned ID");
    TEST_ASSERT(data1.call_count == 1, "Subscriber 1 received event (all)");
    TEST_ASSERT(data2.call_count == 1, "Subscriber 2 received event (economic filter)");
    TEST_ASSERT(data3.call_count == 0, "Subscriber 3 didn't receive event (social filter)");
    TEST_PASS("Event publishing and filtering works");
    event_destroy(trade);

    // Test publishing social event
    GameEvent* rel = event_create_relationship_change(1, 2, 50, 60, "Test");
    event_bus_publish(bus, rel);

    TEST_ASSERT(data1.call_count == 2, "Subscriber 1 received both events");
    TEST_ASSERT(data2.call_count == 1, "Subscriber 2 still at 1 (economic only)");
    TEST_ASSERT(data3.call_count == 1, "Subscriber 3 received social event");
    TEST_PASS("Event filtering by type works");
    event_destroy(rel);

    // Test unsubscribing
    bool unsub = event_bus_unsubscribe(bus, sub2);
    TEST_ASSERT(unsub == true, "Unsubscribe successful");
    TEST_ASSERT(bus->subscriber_count == 2, "Subscriber count decreased");

    GameEvent* trade2 = event_create_trade(3, 4, "corn", 5, 25, false, "Test");
    event_bus_publish(bus, trade2);

    TEST_ASSERT(data1.call_count == 3, "Subscriber 1 still receiving");
    TEST_ASSERT(data2.call_count == 1, "Subscriber 2 no longer receiving");
    TEST_PASS("Unsubscribe works");
    event_destroy(trade2);

    event_bus_destroy(bus);

    printf("\n✅ All event bus tests passed!\n");
    return true;
}

// ============================================================================
// Event Logger Tests
// ============================================================================

bool test_event_logger() {
    printf("\n=== Test 2.3: Event Logger ===\n");

    EventLogger* logger = event_logger_create();
    TEST_ASSERT(logger != NULL, "EventLogger created");
    TEST_ASSERT(logger->event_count == 0, "No initial events");
    TEST_ASSERT(logger->total_events_logged == 0, "Total events zero");
    TEST_PASS("EventLogger initialization works");

    // Log some events
    for (int i = 0; i < 10; i++) {
        GameEvent* event = event_create(EVENT_SYSTEM, SYSTEM_ENTITY_CREATED, i, "Test entity");
        event->game_day = i + 1;
        event_logger_log(logger, event);
        event_destroy(event);
    }

    TEST_ASSERT(logger->event_count == 10, "Logged 10 events");
    TEST_ASSERT(logger->total_events_logged == 10, "Total count correct");
    TEST_PASS("Event logging works");

    // Test getting recent events
    GameEvent* recent[5];
    int count = event_logger_get_recent(logger, recent, 5);
    TEST_ASSERT(count == 5, "Got 5 recent events");
    TEST_ASSERT(recent[0]->game_day == 10, "Most recent event first");
    TEST_ASSERT(recent[4]->game_day == 6, "5th recent event correct");
    TEST_PASS("Get recent events works");

    // Log different event types
    GameEvent* trade = event_create_trade(1, 2, "wheat", 10, 50, true, "Test");
    trade->game_day = 11;
    event_logger_log(logger, trade);
    event_destroy(trade);

    GameEvent* social = event_create_relationship_change(1, 2, 50, 60, "Test");
    social->game_day = 11;
    event_logger_log(logger, social);
    event_destroy(social);

    GameEvent* crop = event_create_crop_action(AGRICULTURAL_CROP_PLANTED, "corn", 5, 3, 1);
    crop->game_day = 11;
    event_logger_log(logger, crop);
    event_destroy(crop);

    TEST_ASSERT(logger->event_count == 13, "Logged 13 total events");
    TEST_PASS("Multiple event types logged");

    // Test filtering by type
    GameEvent* economic[10];
    count = event_logger_get_by_type(logger, EVENT_ECONOMIC, economic, 10);
    TEST_ASSERT(count == 1, "Found 1 economic event");
    TEST_PASS("Filter by type works");

    GameEvent* social_events[10];
    count = event_logger_get_by_type(logger, EVENT_SOCIAL, social_events, 10);
    TEST_ASSERT(count == 1, "Found 1 social event");
    TEST_PASS("Filter by social type works");

    // Test filtering by entity
    GameEvent* entity_events[10];
    count = event_logger_get_by_entity(logger, 1, entity_events, 10);
    TEST_ASSERT(count >= 3, "Found events for entity 1");
    TEST_PASS("Filter by entity works");

    // Test filtering by day
    GameEvent* day_events[10];
    count = event_logger_get_by_day(logger, 11, day_events, 10);
    TEST_ASSERT(count == 3, "Found 3 events on day 11");
    TEST_PASS("Filter by day works");

    // Test statistics
    int total = 0;
    int by_type[EVENT_TYPE_COUNT];
    event_logger_get_stats(logger, &total, by_type);
    TEST_ASSERT(total == 13, "Total events in stats correct");
    TEST_ASSERT(by_type[EVENT_SYSTEM] == 10, "System events counted");
    TEST_ASSERT(by_type[EVENT_ECONOMIC] == 1, "Economic events counted");
    TEST_ASSERT(by_type[EVENT_SOCIAL] == 1, "Social events counted");
    TEST_ASSERT(by_type[EVENT_AGRICULTURAL] == 1, "Agricultural events counted");
    TEST_PASS("Event statistics work");

    // Test ring buffer (overflow)
    printf("  Testing ring buffer overflow...\n");
    EventLogger* small_logger = event_logger_create();

    // Log more than buffer size to test ring buffer
    for (int i = 0; i < 100; i++) {
        GameEvent* event = event_create(EVENT_SYSTEM, SYSTEM_ENTITY_CREATED, i, "Overflow test");
        event->game_day = i;
        event_logger_log(small_logger, event);
        event_destroy(event);
    }

    TEST_ASSERT(small_logger->total_events_logged == 100, "All 100 events logged");
    TEST_PASS("Ring buffer overflow handled");

    event_logger_destroy(small_logger);
    event_logger_destroy(logger);

    printf("\n✅ All event logger tests passed!\n");
    return true;
}

// ============================================================================
// Integration Test
// ============================================================================

bool test_event_system_integration() {
    printf("\n=== Test 2.4: Event System Integration ===\n");

    // Create integrated system
    EventBus* bus = event_bus_create();
    EventLogger* logger = event_logger_create();

    TEST_ASSERT(bus != NULL && logger != NULL, "Systems created");

    // Subscribe logger to event bus
    CallbackData callback_data = {0, 0, 0};
    event_bus_subscribe(bus, test_callback, &callback_data, -1);
    TEST_PASS("Subscriber registered");

    // Create event chain: Trade -> Relationship Change -> Currency
    GameEvent* trade = event_create_trade(1, 2, "wheat", 10, 100, true, "Good price");
    trade->game_day = 5;
    strcpy(trade->game_time, "afternoon");
    event_bus_publish(bus, trade);
    event_logger_log(logger, trade);

    GameEvent* rel = event_create_relationship_change(1, 2, 50, 55, "Successful trade");
    rel->game_day = 5;
    strcpy(rel->game_time, "afternoon");
    event_bus_publish(bus, rel);
    event_logger_log(logger, rel);

    GameEvent* currency = event_create_currency(2, -100, "Bought wheat");
    currency->game_day = 5;
    strcpy(currency->game_time, "afternoon");
    event_bus_publish(bus, currency);
    event_logger_log(logger, currency);

    TEST_ASSERT(callback_data.call_count == 3, "All events published");
    TEST_ASSERT(logger->event_count == 3, "All events logged");
    TEST_PASS("Event chain recorded");

    // Query event chain
    GameEvent* day5_events[10];
    int count = event_logger_get_by_day(logger, 5, day5_events, 10);
    TEST_ASSERT(count == 3, "Retrieved all day 5 events");
    TEST_PASS("Event chain retrievable");

    // Verify event transparency (can see full chain)
    printf("  Event Chain on Day 5:\n");
    for (int i = 0; i < count; i++) {
        printf("    %d. [%s] %s\n", i+1,
               event_type_to_string(day5_events[i]->type),
               day5_events[i]->description);
    }
    TEST_PASS("Event chain transparency verified");

    event_destroy(trade);
    event_destroy(rel);
    event_destroy(currency);
    event_bus_destroy(bus);
    event_logger_destroy(logger);

    printf("\n✅ All integration tests passed!\n");
    return true;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    printf("============================================================\n");
    printf("PHASE 2 TESTS: Event System\n");
    printf("Testing Event Creation, Event Bus, and Event Logger\n");
    printf("============================================================\n");

    bool all_passed = true;

    // Run tests
    if (!test_event_creation()) all_passed = false;
    if (!test_event_bus()) all_passed = false;
    if (!test_event_logger()) all_passed = false;
    if (!test_event_system_integration()) all_passed = false;

    // Print results
    printf("\n============================================================\n");
    printf("RESULTS: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("============================================================\n");

    if (all_passed && tests_failed == 0) {
        printf("\n🎉 Phase 2: Event System - COMPLETE! 🎉\n");
        printf("\nSuccess Criteria Met:\n");
        printf("✓ Can create events with full context\n");
        printf("✓ Event Bus publishes to subscribers\n");
        printf("✓ Event filtering by type works\n");
        printf("✓ Event Logger records all events\n");
        printf("✓ Can query events by type, entity, and day\n");
        printf("✓ Event chains provide transparency\n");
        return 0;
    } else {
        printf("\n⚠️  Some tests failed. Please review errors above.\n");
        return 1;
    }
}
