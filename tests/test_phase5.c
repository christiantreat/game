/**
 * test_phase5.c
 * Tests for Phase 5: Game World & Locations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../src/core/world.h"
#include "../src/core/component.h"
#include "../src/core/entity.h"
#include "../src/core/game_state.h"
#include "../lib/cJSON.h"

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
// Test: Location Creation and Properties
// ============================================================================

bool test_location_creation() {
    printf("\nTest: Location Creation\n");

    // Create location
    Location* loc = location_create(1, "Test Farm", LOCATION_FIELD, 10.0f, 20.0f);
    TEST_ASSERT(loc != NULL, "Location created");
    TEST_ASSERT(loc->id == 1, "ID correct");
    TEST_ASSERT(strcmp(loc->name, "Test Farm") == 0, "Name correct");
    TEST_ASSERT(loc->type == LOCATION_FIELD, "Type correct");
    TEST_ASSERT(loc->x == 10.0f && loc->y == 20.0f, "Position correct");
    TEST_ASSERT(!loc->indoor, "Field is outdoor");
    TEST_ASSERT(loc->can_farm, "Field can farm");
    TEST_PASS("Location properties set correctly");

    // Create indoor location
    Location* shop = location_create(2, "General Store", LOCATION_SHOP, 0.0f, 0.0f);
    TEST_ASSERT(shop->indoor, "Shop is indoor");
    TEST_ASSERT(shop->protected_from_weather, "Shop is protected from weather");
    TEST_ASSERT(shop->can_shop, "Shop can shop");
    TEST_PASS("Indoor location properties correct");

    location_destroy(loc);
    location_destroy(shop);

    printf("  ✓ All location creation tests passed\n");
    return true;
}

// ============================================================================
// Test: Location Connections
// ============================================================================

bool test_location_connections() {
    printf("\nTest: Location Connections\n");

    Location* loc1 = location_create(1, "Place A", LOCATION_OUTDOOR, 0.0f, 0.0f);
    Location* loc2 = location_create(2, "Place B", LOCATION_OUTDOOR, 10.0f, 0.0f);
    Location* loc3 = location_create(3, "Place C", LOCATION_OUTDOOR, 20.0f, 0.0f);

    // Add connections
    bool connected = location_add_connection(loc1, 2, 10.0f, "East road");
    TEST_ASSERT(connected, "Connection added");
    TEST_ASSERT(loc1->connection_count == 1, "Connection count correct");
    TEST_PASS("Basic connection works");

    // Check if connected
    TEST_ASSERT(location_is_connected(loc1, 2), "Is connected to loc2");
    TEST_ASSERT(!location_is_connected(loc1, 3), "Not connected to loc3");
    TEST_PASS("Connection check works");

    // Get distance
    float distance = location_get_connection_distance(loc1, 2);
    TEST_ASSERT(distance == 10.0f, "Distance correct");
    TEST_PASS("Distance retrieval works");

    // Block connection
    location_set_connection_blocked(loc1, 2, true);
    TEST_ASSERT(!location_is_connected(loc1, 2), "Blocked connection not accessible");
    TEST_PASS("Connection blocking works");

    // Unblock
    location_set_connection_blocked(loc1, 2, false);
    TEST_ASSERT(location_is_connected(loc1, 2), "Unblocked connection accessible");
    TEST_PASS("Connection unblocking works");

    // Calculate world distance
    float world_dist = location_distance_to(loc1, loc2);
    TEST_ASSERT(fabs(world_dist - 10.0f) < 0.1f, "World distance correct");
    TEST_PASS("World distance calculation works");

    location_destroy(loc1);
    location_destroy(loc2);
    location_destroy(loc3);

    printf("  ✓ All connection tests passed\n");
    return true;
}

// ============================================================================
// Test: Entity Management in Locations
// ============================================================================

bool test_location_entities() {
    printf("\nTest: Entity Management\n");

    Location* loc = location_create(1, "Town Square", LOCATION_VILLAGE_CENTER, 0.0f, 0.0f);
    loc->capacity = 3;  // Small capacity for testing

    // Add entities
    TEST_ASSERT(location_add_entity(loc, 101), "Entity 101 added");
    TEST_ASSERT(location_add_entity(loc, 102), "Entity 102 added");
    TEST_ASSERT(location_get_entity_count(loc) == 2, "Entity count correct");
    TEST_PASS("Entity addition works");

    // Check if entity is present
    TEST_ASSERT(location_has_entity(loc, 101), "Has entity 101");
    TEST_ASSERT(!location_has_entity(loc, 999), "Doesn't have entity 999");
    TEST_PASS("Entity presence check works");

    // Fill to capacity
    location_add_entity(loc, 103);
    TEST_ASSERT(location_is_full(loc), "Location is full");
    TEST_ASSERT(!location_add_entity(loc, 104), "Cannot add beyond capacity");
    TEST_PASS("Capacity limits work");

    // Remove entity
    TEST_ASSERT(location_remove_entity(loc, 102), "Entity 102 removed");
    TEST_ASSERT(!location_is_full(loc), "No longer full");
    TEST_ASSERT(location_get_entity_count(loc) == 2, "Count decreased");
    TEST_PASS("Entity removal works");

    location_destroy(loc);

    printf("  ✓ All entity management tests passed\n");
    return true;
}

// ============================================================================
// Test: World Creation and Management
// ============================================================================

bool test_world_creation() {
    printf("\nTest: World Creation\n");

    World* world = world_create("Test World", 100.0f, 100.0f);
    TEST_ASSERT(world != NULL, "World created");
    TEST_ASSERT(strcmp(world->world_name, "Test World") == 0, "World name correct");
    TEST_ASSERT(world->world_width == 100.0f, "World width correct");
    TEST_ASSERT(world->location_count == 0, "Starts empty");
    TEST_PASS("World initialization works");

    // Add locations
    int id1 = world_add_location(world, "Farm", LOCATION_FIELD, 10.0f, 10.0f);
    int id2 = world_add_location(world, "Shop", LOCATION_SHOP, 20.0f, 20.0f);
    TEST_ASSERT(id1 > 0 && id2 > 0, "Locations added");
    TEST_ASSERT(world->location_count == 2, "Location count correct");
    TEST_PASS("Location addition works");

    // Get location by ID
    Location* farm = world_get_location(world, id1);
    TEST_ASSERT(farm != NULL, "Got location by ID");
    TEST_ASSERT(strcmp(farm->name, "Farm") == 0, "Retrieved correct location");
    TEST_PASS("Location retrieval by ID works");

    // Get location by name
    Location* shop = world_get_location_by_name(world, "Shop");
    TEST_ASSERT(shop != NULL, "Got location by name");
    TEST_ASSERT(shop->id == id2, "Retrieved correct location");
    TEST_PASS("Location retrieval by name works");

    // Get location by position
    Location* at_farm = world_get_location_at(world, 12.0f, 12.0f);
    TEST_ASSERT(at_farm != NULL, "Got location at position");
    TEST_ASSERT(at_farm->id == id1, "Found correct location");
    TEST_PASS("Location retrieval by position works");

    world_destroy(world);

    printf("  ✓ All world creation tests passed\n");
    return true;
}

// ============================================================================
// Test: World Connections and Pathfinding
// ============================================================================

bool test_world_pathfinding() {
    printf("\nTest: Pathfinding\n");

    World* world = world_create("Path World", 200.0f, 200.0f);

    // Create a simple graph:
    // A -- B -- C
    //  \       /
    //   \--D--/

    int id_a = world_add_location(world, "A", LOCATION_OUTDOOR, 0.0f, 0.0f);
    int id_b = world_add_location(world, "B", LOCATION_OUTDOOR, 10.0f, 0.0f);
    int id_c = world_add_location(world, "C", LOCATION_OUTDOOR, 20.0f, 0.0f);
    int id_d = world_add_location(world, "D", LOCATION_OUTDOOR, 10.0f, 10.0f);

    // Connect locations
    world_connect_locations(world, id_a, id_b, 10.0f, "Road AB");
    world_connect_locations(world, id_b, id_c, 10.0f, "Road BC");
    world_connect_locations(world, id_a, id_d, 15.0f, "Road AD");
    world_connect_locations(world, id_d, id_c, 15.0f, "Road DC");

    TEST_PASS("Test world created with 4 locations");

    // Test direct path A -> B
    int path[MAX_PATH_LENGTH];
    int path_len = world_find_path(world, id_a, id_b, path, MAX_PATH_LENGTH);
    TEST_ASSERT(path_len == 2, "Direct path has 2 nodes");
    TEST_ASSERT(path[0] == id_a && path[1] == id_b, "Path is correct");
    TEST_PASS("Direct path found");

    // Test longer path A -> C (should go through B)
    path_len = world_find_path(world, id_a, id_c, path, MAX_PATH_LENGTH);
    TEST_ASSERT(path_len == 3, "Path A->C has 3 nodes");
    TEST_ASSERT(path[0] == id_a, "Path starts at A");
    TEST_ASSERT(path[2] == id_c, "Path ends at C");
    TEST_PASS("Multi-hop path found");

    // Test path distance
    float distance = world_get_path_distance(world, path, path_len);
    TEST_ASSERT(distance == 20.0f, "Path distance correct (10+10)");
    TEST_PASS("Path distance calculation works");

    // Block path and test alternate route
    Location* loc_b = world_get_location(world, id_b);
    location_set_connection_blocked(loc_b, id_c, true);

    path_len = world_find_path(world, id_a, id_c, path, MAX_PATH_LENGTH);
    TEST_ASSERT(path_len == 3, "Alternate path found");
    TEST_ASSERT(path[1] == id_d, "Uses alternate route through D");
    TEST_PASS("Pathfinding avoids blocked connections");

    world_destroy(world);

    printf("  ✓ All pathfinding tests passed\n");
    return true;
}

// ============================================================================
// Test: Entity Movement Between Locations
// ============================================================================

bool test_entity_movement() {
    printf("\nTest: Entity Movement\n");

    World* world = world_create("Movement World", 100.0f, 100.0f);

    int id_farm = world_add_location(world, "Farm", LOCATION_FIELD, 0.0f, 0.0f);
    int id_shop = world_add_location(world, "Shop", LOCATION_SHOP, 20.0f, 0.0f);

    world_connect_locations(world, id_farm, id_shop, 20.0f, "Road");

    // Place entity at farm
    Location* farm = world_get_location(world, id_farm);
    location_add_entity(farm, 501);

    TEST_ASSERT(location_has_entity(farm, 501), "Entity at farm");
    TEST_PASS("Entity placed at location");

    // Move entity to shop
    bool moved = world_move_entity(world, 501, id_farm, id_shop);
    TEST_ASSERT(moved, "Entity moved");
    TEST_ASSERT(!location_has_entity(farm, 501), "Entity left farm");

    Location* shop = world_get_location(world, id_shop);
    TEST_ASSERT(location_has_entity(shop, 501), "Entity arrived at shop");
    TEST_PASS("Entity movement works");

    // Get entity's current location
    Location* entity_loc = world_get_entity_location(world, 501);
    TEST_ASSERT(entity_loc != NULL, "Found entity location");
    TEST_ASSERT(entity_loc->id == id_shop, "Entity at shop");
    TEST_PASS("Entity location lookup works");

    // Get all entities at location
    int entities[MAX_ENTITIES_PER_LOCATION];
    int count = world_get_entities_at_location(world, id_shop, entities, MAX_ENTITIES_PER_LOCATION);
    TEST_ASSERT(count == 1, "One entity at shop");
    TEST_ASSERT(entities[0] == 501, "Correct entity ID");
    TEST_PASS("Entity enumeration works");

    world_destroy(world);

    printf("  ✓ All entity movement tests passed\n");
    return true;
}

// ============================================================================
// Test: Location Type Filtering
// ============================================================================

bool test_location_filtering() {
    printf("\nTest: Location Filtering\n");

    World* world = world_create("Filter World", 100.0f, 100.0f);

    world_add_location(world, "Farm 1", LOCATION_FIELD, 0.0f, 0.0f);
    world_add_location(world, "Farm 2", LOCATION_FIELD, 10.0f, 0.0f);
    world_add_location(world, "Shop", LOCATION_SHOP, 20.0f, 0.0f);
    world_add_location(world, "House", LOCATION_HOME, 30.0f, 0.0f);

    // Get all fields
    Location* fields[10];
    int count = world_get_locations_by_type(world, LOCATION_FIELD, fields, 10);
    TEST_ASSERT(count == 2, "Found 2 fields");
    TEST_PASS("Location type filtering works");

    // Find nearest field
    Location* nearest = world_find_nearest_location(world, 8.0f, 0.0f, LOCATION_FIELD);
    TEST_ASSERT(nearest != NULL, "Found nearest field");
    TEST_ASSERT(strcmp(nearest->name, "Farm 2") == 0, "Farm 2 is nearest");
    TEST_PASS("Nearest location search works");

    world_destroy(world);

    printf("  ✓ All filtering tests passed\n");
    return true;
}

// ============================================================================
// Test: Pre-built Farming Village World
// ============================================================================

bool test_farming_village() {
    printf("\nTest: Farming Village World\n");

    World* world = create_farming_village_world();
    TEST_ASSERT(world != NULL, "Farming village created");
    TEST_ASSERT(world->location_count > 0, "Has locations");
    TEST_PASS("Village creation works");

    // Check for village center
    Location* center = world_get_location_by_name(world, "Village Square");
    TEST_ASSERT(center != NULL, "Village center exists");
    TEST_ASSERT(center->type == LOCATION_VILLAGE_CENTER, "Correct type");
    TEST_PASS("Village center present");

    // Check for shop
    Location* shop = world_get_location_by_name(world, "General Store");
    TEST_ASSERT(shop != NULL, "Shop exists");
    TEST_ASSERT(shop->can_shop, "Shop can shop");
    TEST_PASS("Shop present and configured");

    // Check for fields
    Location* fields[10];
    int field_count = world_get_locations_by_type(world, LOCATION_FIELD, fields, 10);
    TEST_ASSERT(field_count >= 2, "Has farm fields");
    TEST_PASS("Farm fields present");

    // Check for homes
    Location* homes[10];
    int home_count = world_get_locations_by_type(world, LOCATION_HOME, homes, 10);
    TEST_ASSERT(home_count >= 3, "Has homes");
    TEST_PASS("Residential area present");

    // Check connections from village center
    TEST_ASSERT(center->connection_count > 0, "Village center is connected");
    TEST_PASS("Locations are interconnected");

    // Print world for visual inspection
    printf("\n  Village World Layout:\n");
    for (int i = 0; i < world->location_count; i++) {
        Location* loc = world->locations[i];
        printf("    - %s (%s) at (%.0f, %.0f) with %d connections\n",
               loc->name, location_type_to_string(loc->type),
               loc->x, loc->y, loc->connection_count);
    }

    world_destroy(world);

    printf("  ✓ All farming village tests passed\n");
    return true;
}

// ============================================================================
// Test: Serialization
// ============================================================================

bool test_world_serialization() {
    printf("\nTest: World Serialization\n");

    // Create world
    World* world = world_create("Save World", 100.0f, 100.0f);
    int id1 = world_add_location(world, "Farm", LOCATION_FIELD, 10.0f, 20.0f);
    int id2 = world_add_location(world, "Shop", LOCATION_SHOP, 30.0f, 40.0f);
    world_connect_locations(world, id1, id2, 25.0f, "Main road");

    // Add entity to location
    Location* farm = world_get_location(world, id1);
    location_add_entity(farm, 999);

    // Serialize
    cJSON* json = world_to_json(world);
    TEST_ASSERT(json != NULL, "World serialized to JSON");
    TEST_PASS("Serialization works");

    // Deserialize
    World* loaded = world_from_json(json);
    TEST_ASSERT(loaded != NULL, "World deserialized");
    TEST_ASSERT(loaded->location_count == 2, "Location count preserved");
    TEST_ASSERT(strcmp(loaded->world_name, "Save World") == 0, "Name preserved");
    TEST_PASS("Deserialization works");

    // Check location
    Location* loaded_farm = world_get_location_by_name(loaded, "Farm");
    TEST_ASSERT(loaded_farm != NULL, "Farm location exists");
    TEST_ASSERT(loaded_farm->x == 10.0f && loaded_farm->y == 20.0f, "Position preserved");
    TEST_ASSERT(loaded_farm->connection_count == 1, "Connections preserved");
    TEST_PASS("Location data preserved");

    cJSON_Delete(json);
    world_destroy(world);
    world_destroy(loaded);

    printf("  ✓ All serialization tests passed\n");
    return true;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    printf("\n");
    printf("============================================================\n");
    printf("PHASE 5: Game World & Locations Tests\n");
    printf("============================================================\n");

    int passed = 0;
    int failed = 0;

    // Run tests
    if (test_location_creation()) passed++; else failed++;
    if (test_location_connections()) passed++; else failed++;
    if (test_location_entities()) passed++; else failed++;
    if (test_world_creation()) passed++; else failed++;
    if (test_world_pathfinding()) passed++; else failed++;
    if (test_entity_movement()) passed++; else failed++;
    if (test_location_filtering()) passed++; else failed++;
    if (test_farming_village()) passed++; else failed++;
    if (test_world_serialization()) passed++; else failed++;

    // Summary
    printf("\n");
    printf("============================================================\n");
    printf("RESULTS: %d passed, %d failed\n", passed, failed);
    printf("============================================================\n");

    if (failed == 0) {
        printf("\n🎉 Phase 5: Game World & Locations - COMPLETE! 🎉\n\n");
        printf("Success Criteria Met:\n");
        printf("✓ Location creation and property management\n");
        printf("✓ Location connections and blocking\n");
        printf("✓ Entity management in locations\n");
        printf("✓ World creation and location management\n");
        printf("✓ Pathfinding with BFS algorithm\n");
        printf("✓ Entity movement between locations\n");
        printf("✓ Location type filtering and search\n");
        printf("✓ Pre-built farming village world\n");
        printf("✓ World serialization and deserialization\n");
        printf("\n");
    }

    return failed == 0 ? 0 : 1;
}
