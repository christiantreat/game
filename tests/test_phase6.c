/**
 * test_phase6.c
 * Tests for Phase 6: Time & Agriculture Systems
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/core/agriculture.h"
#include "../src/core/game_state.h"

#define TEST_ASSERT(condition, message) \
    do { if (!(condition)) { printf("  ❌ FAILED: %s\n", message); return false; } } while(0)
#define TEST_PASS(message) printf("  ✓ %s\n", message)

bool test_crop_type_creation() {
    printf("\nTest: Crop Type Creation\n");
    
    CropType* wheat = create_wheat_crop_type();
    TEST_ASSERT(wheat != NULL, "Wheat crop type created");
    TEST_ASSERT(strcmp(wheat->name, "Wheat") == 0, "Name is Wheat");
    TEST_ASSERT(wheat->days_to_mature == 8, "Matures in 8 days");
    TEST_ASSERT(wheat->preferred_season == SEASON_SPRING, "Prefers spring");
    TEST_PASS("Wheat crop type works");
    
    CropType* potato = create_potato_crop_type();
    TEST_ASSERT(potato->can_grow_any_season, "Potato grows any season");
    TEST_PASS("Potato crop type works");
    
    crop_type_destroy(wheat);
    crop_type_destroy(potato);
    
    printf("  ✓ All crop type tests passed\n");
    return true;
}

bool test_crop_growth() {
    printf("\nTest: Crop Growth\n");
    
    GameState* state = game_state_create();
    state->season = SEASON_SPRING;
    state->current_weather = WEATHER_SUNNY;
    
    CropType* wheat = create_wheat_crop_type();
    Crop* crop = crop_create(1, "Wheat", 100, 0, 0, 999);
    
    TEST_ASSERT(crop->stage == CROP_STAGE_SEED, "Starts as seed");
    TEST_ASSERT(crop->health == 100, "Starts healthy");
    TEST_PASS("Crop creation works");
    
    // Simulate growth
    for (int day = 0; day < 3; day++) {
        crop_water(crop);
        crop_update(crop, wheat, state);
    }
    
    TEST_ASSERT(crop->stage == CROP_STAGE_SPROUT, "Sprouted after 3 days");
    TEST_ASSERT(crop->water_level > 0, "Has water");
    TEST_PASS("Crop grows and changes stages");
    
    // Continue to maturity
    for (int day = 0; day < 10; day++) {
        crop_water(crop);
        crop_update(crop, wheat, state);
    }
    
    TEST_ASSERT(crop_is_ready_to_harvest(crop), "Ready to harvest");
    TEST_ASSERT(crop->predicted_yield > 0, "Has yield");
    TEST_PASS("Crop reaches maturity");
    
    crop_destroy(crop);
    crop_type_destroy(wheat);
    game_state_destroy(state);
    
    printf("  ✓ All growth tests passed\n");
    return true;
}

bool test_crop_death() {
    printf("\nTest: Crop Death\n");
    
    GameState* state = game_state_create();
    state->current_weather = WEATHER_DROUGHT;
    
    CropType* wheat = create_wheat_crop_type();
    Crop* crop = crop_create(1, "Wheat", 100, 0, 0, 999);
    
    // Don't water - let it die
    for (int day = 0; day < 10; day++) {
        crop_update(crop, wheat, state);
    }
    
    TEST_ASSERT(crop_is_withered(crop), "Crop withered from drought");
    TEST_PASS("Crop death works");
    
    crop_destroy(crop);
    crop_type_destroy(wheat);
    game_state_destroy(state);
    
    printf("  ✓ All death tests passed\n");
    return true;
}

bool test_field_manager() {
    printf("\nTest: Field Manager\n");
    
    FieldManager* field = field_manager_create(100, 5, 5);
    TEST_ASSERT(field != NULL, "Field created");
    TEST_ASSERT(field->max_plots == 25, "Has 25 plots");
    TEST_PASS("Field creation works");
    
    // Plant crops
    int crop1 = field_manager_plant_crop(field, "Wheat", 0, 0, 999);
    int crop2 = field_manager_plant_crop(field, "Corn", 1, 0, 999);
    TEST_ASSERT(crop1 > 0 && crop2 > 0, "Crops planted");
    TEST_ASSERT(field->crop_count == 2, "2 crops in field");
    TEST_PASS("Planting works");
    
    // Try to plant on occupied plot
    int crop3 = field_manager_plant_crop(field, "Wheat", 0, 0, 999);
    TEST_ASSERT(crop3 == -1, "Cannot plant on occupied plot");
    TEST_PASS("Plot occupation check works");
    
    // Get crop
    Crop* c = field_manager_get_crop(field, crop1);
    TEST_ASSERT(c != NULL, "Got crop by ID");
    TEST_ASSERT(strcmp(c->crop_type_name, "Wheat") == 0, "Correct crop");
    TEST_PASS("Crop retrieval works");
    
    // Water all
    int watered = field_manager_water_all(field);
    TEST_ASSERT(watered == 2, "Watered 2 crops");
    TEST_PASS("Watering works");
    
    field_manager_destroy(field);
    
    printf("  ✓ All field manager tests passed\n");
    return true;
}

bool test_agriculture_manager() {
    printf("\nTest: Agriculture Manager\n");
    
    AgricultureManager* ag = agriculture_manager_create();
    TEST_ASSERT(ag != NULL, "Manager created");
    
    load_default_crop_types(ag);
    TEST_ASSERT(ag->crop_type_count == 5, "5 crop types loaded");
    TEST_PASS("Default crop types loaded");
    
    CropType* wheat = agriculture_manager_get_crop_type(ag, "Wheat");
    TEST_ASSERT(wheat != NULL, "Got wheat type");
    TEST_PASS("Crop type lookup works");
    
    // Register field
    bool registered = agriculture_manager_register_field(ag, 100, 5, 5);
    TEST_ASSERT(registered, "Field registered");
    TEST_PASS("Field registration works");
    
    // Plant crop
    int crop_id = agriculture_manager_plant_crop(ag, 100, "Wheat", 0, 0, 999);
    TEST_ASSERT(crop_id > 0, "Crop planted");
    TEST_PASS("Planting through manager works");
    
    agriculture_manager_destroy(ag);
    
    printf("  ✓ All agriculture manager tests passed\n");
    return true;
}

bool test_time_progression() {
    printf("\nTest: Time Progression\n");
    
    GameState* state = game_state_create();
    AgricultureManager* ag = agriculture_manager_create();
    load_default_crop_types(ag);
    agriculture_manager_register_field(ag, 100, 5, 5);
    
    int initial_day = state->day_count;
    TimeOfDay initial_time = state->time_of_day;
    
    // Advance time period
    time_advance_period(state, ag);
    TEST_ASSERT(state->time_of_day != initial_time, "Time advanced");
    TEST_PASS("Time period advancement works");
    
    // Advance to next day
    time_advance_day(state, ag);
    TEST_ASSERT(state->day_count == initial_day + 1, "Day advanced");
    TEST_ASSERT(state->time_of_day == TIME_MORNING, "Reset to morning");
    TEST_PASS("Day advancement works");
    
    // Plant and advance days
    agriculture_manager_plant_crop(ag, 100, "Wheat", 0, 0, 999);
    
    for (int i = 0; i < 10; i++) {
        time_advance_day(state, ag);
    }
    
    FieldManager* field = agriculture_manager_get_field(ag, 100);
    Crop* crop = field->crops[0];
    TEST_ASSERT(crop->days_planted >= 10, "Crop aged properly");
    TEST_PASS("Crops update with time");
    
    agriculture_manager_destroy(ag);
    game_state_destroy(state);
    
    printf("  ✓ All time progression tests passed\n");
    return true;
}

bool test_full_harvest_cycle() {
    printf("\nTest: Full Harvest Cycle\n");
    
    GameState* state = game_state_create();
    state->season = SEASON_SPRING;
    state->current_weather = WEATHER_SUNNY;
    
    AgricultureManager* ag = agriculture_manager_create();
    load_default_crop_types(ag);
    agriculture_manager_register_field(ag, 100, 10, 10);
    
    // Plant wheat
    int crop_id = agriculture_manager_plant_crop(ag, 100, "Wheat", 0, 0, 999);
    TEST_ASSERT(crop_id > 0, "Crop planted");
    
    // Water and advance time to maturity
    FieldManager* field = agriculture_manager_get_field(ag, 100);
    for (int day = 0; day < 10; day++) {
        field_manager_water_all(field);
        time_advance_day(state, ag);
    }
    
    // Check if ready
    Crop* ready_crops[10];
    int ready_count = field_manager_get_ready_crops(field, ready_crops, 10);
    TEST_ASSERT(ready_count > 0, "Crops ready to harvest");
    TEST_PASS("Crops mature successfully");
    
    // Harvest
    int yield = agriculture_manager_harvest_crop(ag, 100, crop_id);
    TEST_ASSERT(yield > 0, "Got yield from harvest");
    TEST_ASSERT(field->crop_count == 0, "Crop removed after harvest");
    TEST_PASS("Harvesting works");
    
    agriculture_manager_destroy(ag);
    game_state_destroy(state);
    
    printf("  ✓ All harvest cycle tests passed\n");
    return true;
}

int main() {
    printf("\n");
    printf("============================================================\n");
    printf("PHASE 6: Time & Agriculture Systems Tests\n");
    printf("============================================================\n");
    
    int passed = 0, failed = 0;
    
    if (test_crop_type_creation()) passed++; else failed++;
    if (test_crop_growth()) passed++; else failed++;
    if (test_crop_death()) passed++; else failed++;
    if (test_field_manager()) passed++; else failed++;
    if (test_agriculture_manager()) passed++; else failed++;
    if (test_time_progression()) passed++; else failed++;
    if (test_full_harvest_cycle()) passed++; else failed++;
    
    printf("\n");
    printf("============================================================\n");
    printf("RESULTS: %d passed, %d failed\n", passed, failed);
    printf("============================================================\n");
    
    if (failed == 0) {
        printf("\n🎉 Phase 6: Time & Agriculture Systems - COMPLETE! 🎉\n\n");
        printf("Success Criteria Met:\n");
        printf("✓ Crop types can be created and configured\n");
        printf("✓ Crops grow through stages (Seed -> Sprout -> Growing -> Mature)\n");
        printf("✓ Crops can wither and die from neglect\n");
        printf("✓ Field managers track crops and plots\n");
        printf("✓ Agriculture manager coordinates all farming\n");
        printf("✓ Time progression works (periods, days, seasons)\n");
        printf("✓ Full plant-grow-harvest cycle works\n");
        printf("\n");
    }
    
    return failed == 0 ? 0 : 1;
}
