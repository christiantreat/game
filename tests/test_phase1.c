/**
 * test_phase1.c
 * Phase 1 Tests: Core Foundation
 * Tests for Component System, Entity System, and Game State
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../lib/cJSON.h"
#include "../src/core/component.h"
#include "../src/core/entity.h"
#include "../src/core/game_state.h"

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
// Component Tests
// ============================================================================

bool test_component_creation() {
    printf("\n=== Test 1.1: Component Creation ===\n");

    // Test Position Component
    PositionComponent* pos = position_component_create("VillageSquare", 10.5f, 20.3f);
    TEST_ASSERT(pos != NULL, "Position component created");
    TEST_ASSERT(strcmp(pos->location, "VillageSquare") == 0, "Position location correct");
    TEST_ASSERT(pos->x == 10.5f && pos->y == 20.3f, "Position coordinates correct");
    TEST_PASS("PositionComponent works");
    position_component_destroy(pos);

    // Test Health Component
    HealthComponent* health = health_component_create(80, 100);
    TEST_ASSERT(health != NULL, "Health component created");
    TEST_ASSERT(health_component_is_alive(health), "Health component is alive");
    health_component_damage(health, 30);
    TEST_ASSERT(health->current == 50, "Health damage works");
    health_component_heal(health, 20);
    TEST_ASSERT(health->current == 70, "Health heal works");
    TEST_PASS("HealthComponent works");
    health_component_destroy(health);

    // Test Inventory Component
    InventoryComponent* inv = inventory_component_create(10);
    TEST_ASSERT(inv != NULL, "Inventory component created");
    TEST_ASSERT(inventory_component_add_item(inv, "wheat", 5), "Add item works");
    TEST_ASSERT(inventory_component_has_item(inv, "wheat", 5), "Has item works");
    TEST_ASSERT(inventory_component_get_count(inv, "wheat") == 5, "Get count works");
    TEST_ASSERT(inventory_component_remove_item(inv, "wheat", 2), "Remove item works");
    TEST_ASSERT(inventory_component_get_count(inv, "wheat") == 3, "Item count correct after removal");
    TEST_PASS("InventoryComponent works");
    inventory_component_destroy(inv);

    // Test Currency Component
    CurrencyComponent* currency = currency_component_create(100);
    TEST_ASSERT(currency != NULL, "Currency component created");
    TEST_ASSERT(currency_component_has(currency, 50), "Has currency works");
    TEST_ASSERT(currency_component_remove(currency, 30), "Remove currency works");
    TEST_ASSERT(currency->amount == 70, "Currency amount correct");
    currency_component_add(currency, 50);
    TEST_ASSERT(currency->amount == 120, "Add currency works");
    TEST_PASS("CurrencyComponent works");
    currency_component_destroy(currency);

    // Test Relationship Component
    RelationshipComponent* rel = relationship_component_create();
    TEST_ASSERT(rel != NULL, "Relationship component created");
    relationship_component_set(rel, 1, 60);
    TEST_ASSERT(relationship_component_get(rel, 1) == 60, "Set relationship works");
    TEST_ASSERT(strcmp(relationship_component_get_level(rel, 1), "friend") == 0, "Relationship level correct");
    relationship_component_modify(rel, 1, 20);
    TEST_ASSERT(relationship_component_get(rel, 1) == 80, "Modify relationship works");
    TEST_ASSERT(strcmp(relationship_component_get_level(rel, 1), "close_friend") == 0, "Relationship level updated");
    TEST_PASS("RelationshipComponent works");
    relationship_component_destroy(rel);

    // Test Needs Component
    NeedsComponent* needs = needs_component_create();
    TEST_ASSERT(needs != NULL, "Needs component created");
    needs->hunger = 30.0f;  // Low hunger (hungry)
    needs->energy = 20.0f;  // Low energy (tired)
    TEST_ASSERT(strcmp(needs_component_get_most_urgent(needs), "energy") == 0, "Most urgent need correct");
    needs_component_eat(needs, 40.0f);
    TEST_ASSERT(needs->hunger == 70.0f, "Eat works");
    TEST_PASS("NeedsComponent works");
    needs_component_destroy(needs);

    // Test Schedule Component
    ScheduleComponent* schedule = schedule_component_create();
    TEST_ASSERT(schedule != NULL, "Schedule component created");
    schedule_component_set_activity(schedule, "morning", "work");
    schedule_component_set_activity(schedule, "evening", "socialize");
    TEST_ASSERT(strcmp(schedule_component_get_activity(schedule, "morning"), "work") == 0, "Schedule get works");
    TEST_ASSERT(strcmp(schedule_component_get_activity(schedule, "evening"), "socialize") == 0, "Schedule set works");
    TEST_PASS("ScheduleComponent works");
    schedule_component_destroy(schedule);

    // Test Occupation Component
    OccupationComponent* occ = occupation_component_create("Blacksmith", "Forge", 5);
    TEST_ASSERT(occ != NULL, "Occupation component created");
    TEST_ASSERT(strcmp(occ->occupation, "Blacksmith") == 0, "Occupation correct");
    TEST_ASSERT(strcmp(occ->workplace, "Forge") == 0, "Workplace correct");
    TEST_ASSERT(occ->skill_level == 5, "Skill level correct");
    TEST_PASS("OccupationComponent works");
    occupation_component_destroy(occ);

    printf("\n✅ All component tests passed!\n");
    return true;
}

// ============================================================================
// Entity System Tests
// ============================================================================

bool test_entity_system() {
    printf("\n=== Test 1.2: Entity System ===\n");

    EntityManager* manager = entity_manager_create();
    TEST_ASSERT(manager != NULL, "EntityManager created");

    // Create player entity
    Entity* player = create_player_entity(manager, "TestPlayer");
    TEST_ASSERT(player != NULL, "Player entity created");
    TEST_ASSERT(strcmp(player->name, "TestPlayer") == 0, "Player name correct");
    TEST_ASSERT(strcmp(player->entity_type, "Player") == 0, "Player type correct");
    TEST_ASSERT(entity_has_component(player, COMPONENT_POSITION), "Player has position");
    TEST_ASSERT(entity_has_component(player, COMPONENT_HEALTH), "Player has health");
    TEST_ASSERT(entity_has_component(player, COMPONENT_INVENTORY), "Player has inventory");
    printf("  ✓ Created player: Entity(%d, '%s', type=%s, components=%d)\n",
           player->id, player->name, player->entity_type, player->component_count);

    // Create villager entity
    Entity* villager = create_villager_entity(manager, "Marcus", "Merchant", "VillageSquare");
    TEST_ASSERT(villager != NULL, "Villager entity created");
    TEST_ASSERT(strcmp(villager->name, "Marcus") == 0, "Villager name correct");
    TEST_ASSERT(entity_has_component(villager, COMPONENT_OCCUPATION), "Villager has occupation");

    OccupationComponent* occ = (OccupationComponent*)entity_get_component(villager, COMPONENT_OCCUPATION);
    TEST_ASSERT(occ != NULL, "Get occupation component works");
    TEST_ASSERT(strcmp(occ->occupation, "Merchant") == 0, "Occupation correct");
    printf("  ✓ Created villager: Entity(%d, '%s', type=%s, components=%d)\n",
           villager->id, villager->name, villager->entity_type, villager->component_count);

    // Create crop entity
    Entity* crop = create_crop_entity(manager, "Wheat", "YourFarm", 5.0f, 3.0f);
    TEST_ASSERT(crop != NULL, "Crop entity created");
    TEST_ASSERT(strcmp(crop->name, "Wheat") == 0, "Crop name correct");
    TEST_ASSERT(strcmp(crop->entity_type, "Crop") == 0, "Crop type correct");

    PositionComponent* pos = (PositionComponent*)entity_get_component(crop, COMPONENT_POSITION);
    TEST_ASSERT(pos != NULL, "Crop has position");
    TEST_ASSERT(strcmp(pos->location, "YourFarm") == 0, "Crop location correct");
    TEST_ASSERT(pos->x == 5.0f, "Crop x position correct");
    printf("  ✓ Created crop: Entity(%d, '%s', type=%s, components=%d)\n",
           crop->id, crop->name, crop->entity_type, crop->component_count);

    // Test entity lookup
    TEST_ASSERT(entity_manager_count(manager) == 3, "Entity count correct");
    TEST_ASSERT(entity_manager_get_entity(manager, player->id) == player, "Get entity by ID works");
    printf("  ✓ Entity lookup works, total entities: %d\n", entity_manager_count(manager));

    // Test query by component
    Entity* entities_with_position[MAX_ENTITIES];
    ComponentType position_type = COMPONENT_POSITION;
    int count = entity_manager_query_entities(manager, &position_type, 1, entities_with_position, MAX_ENTITIES);
    TEST_ASSERT(count == 3, "Query by component works");
    printf("  ✓ Query by component works: found %d entities with position\n", count);

    Entity* entities_with_needs[MAX_ENTITIES];
    ComponentType needs_type = COMPONENT_NEEDS;
    count = entity_manager_query_entities(manager, &needs_type, 1, entities_with_needs, MAX_ENTITIES);
    TEST_ASSERT(count == 1, "Query finds only entities with needs");
    printf("  ✓ Query found %d entities with needs (villager only)\n", count);

    // Test entity removal
    TEST_ASSERT(entity_manager_remove_entity(manager, crop->id), "Remove entity works");
    TEST_ASSERT(entity_manager_count(manager) == 2, "Entity count after removal correct");
    TEST_ASSERT(entity_manager_get_entity(manager, crop->id) == NULL, "Removed entity not found");
    TEST_PASS("Entity removal works");

    entity_manager_destroy(manager);

    printf("\n✅ All entity system tests passed!\n");
    return true;
}

// ============================================================================
// Game State Tests
// ============================================================================

bool test_game_state() {
    printf("\n=== Test 1.3: Game State System ===\n");

    // Create game state
    GameState* state = game_state_create();
    TEST_ASSERT(state != NULL, "GameState created");
    TEST_ASSERT(state->day_count == 1, "Initial day count correct");
    TEST_ASSERT(state->season == SEASON_SPRING, "Initial season correct");
    TEST_ASSERT(state->time_of_day == TIME_MORNING, "Initial time of day correct");
    TEST_PASS("GameState initialized");

    // Create entities
    Entity* player = create_player_entity(state->entity_manager, "Hero");
    TEST_ASSERT(player != NULL, "Player created");
    game_state_set_player(state, player->id);
    TEST_ASSERT(game_state_get_player(state) == player, "Set/get player works");
    printf("  ✓ Player set: %s\n", player->name);

    Entity* villager1 = create_villager_entity(state->entity_manager, "Sarah", "Baker", "VillageSquare");
    Entity* villager2 = create_villager_entity(state->entity_manager, "Tom", "Farmer", "VillageSquare");
    TEST_ASSERT(villager1 != NULL && villager2 != NULL, "Villagers created");
    printf("  ✓ Created 2 villagers\n");

    // Modify player state
    InventoryComponent* inv = (InventoryComponent*)entity_get_component(player, COMPONENT_INVENTORY);
    TEST_ASSERT(inv != NULL, "Player inventory exists");
    inventory_component_add_item(inv, "wheat", 10);
    inventory_component_add_item(inv, "corn", 5);

    CurrencyComponent* currency = (CurrencyComponent*)entity_get_component(player, COMPONENT_CURRENCY);
    TEST_ASSERT(currency != NULL, "Player currency exists");
    currency_component_add(currency, 250);

    // Set relationships
    RelationshipComponent* rel = (RelationshipComponent*)entity_get_component(player, COMPONENT_RELATIONSHIP);
    TEST_ASSERT(rel != NULL, "Player relationships exist");
    relationship_component_set(rel, villager1->id, 60);
    relationship_component_set(rel, villager2->id, 30);
    TEST_PASS("Modified player state");

    // Test time advancement
    const char* initial_time = game_state_get_time_description(state);
    printf("  ✓ Time: %s\n", initial_time);

    game_state_advance_time(state);
    TEST_ASSERT(state->time_of_day == TIME_AFTERNOON, "Time advanced to afternoon");

    game_state_advance_time(state);
    game_state_advance_time(state);
    game_state_advance_time(state);  // Should wrap to next day
    TEST_ASSERT(state->day_count == 2, "Day count advanced");
    TEST_ASSERT(state->time_of_day == TIME_MORNING, "Time wrapped to morning");

    printf("  ✓ Time advancement works: %s -> %s\n", initial_time, game_state_get_time_description(state));

    // Test serialization
    printf("\n--- Testing Serialization ---\n");
    cJSON* state_json = game_state_to_json(state);
    TEST_ASSERT(state_json != NULL, "to_json works");
    TEST_PASS("Serialization to JSON works");

    // Save to file
    const char* temp_file = "/tmp/test_game_save.json";
    TEST_ASSERT(game_state_save_to_file(state, temp_file), "Save to file works");

    // Load from file
    GameState* loaded_state = game_state_load_from_file(temp_file);
    TEST_ASSERT(loaded_state != NULL, "Load from file works");
    TEST_PASS("Loaded from file");

    // Verify loaded state
    TEST_ASSERT(loaded_state->day_count == state->day_count, "Day count preserved");
    TEST_ASSERT(loaded_state->season == state->season, "Season preserved");
    TEST_ASSERT(entity_manager_count(loaded_state->entity_manager) == 3, "Entity count preserved");
    TEST_PASS("Loaded state matches original");

    // Verify player data
    Entity* loaded_player = game_state_get_player(loaded_state);
    TEST_ASSERT(loaded_player != NULL, "Player loaded");
    TEST_ASSERT(strcmp(loaded_player->name, "Hero") == 0, "Player name preserved");

    InventoryComponent* loaded_inv = (InventoryComponent*)entity_get_component(loaded_player, COMPONENT_INVENTORY);
    TEST_ASSERT(loaded_inv != NULL, "Player inventory loaded");
    TEST_ASSERT(inventory_component_get_count(loaded_inv, "wheat") == 10, "Wheat count preserved");
    TEST_ASSERT(inventory_component_get_count(loaded_inv, "corn") == 5, "Corn count preserved");

    CurrencyComponent* loaded_currency = (CurrencyComponent*)entity_get_component(loaded_player, COMPONENT_CURRENCY);
    TEST_ASSERT(loaded_currency != NULL, "Player currency loaded");
    TEST_ASSERT(loaded_currency->amount == 350, "Currency amount preserved (100 + 250)");

    RelationshipComponent* loaded_rel = (RelationshipComponent*)entity_get_component(loaded_player, COMPONENT_RELATIONSHIP);
    TEST_ASSERT(loaded_rel != NULL, "Player relationships loaded");
    TEST_ASSERT(relationship_component_get(loaded_rel, villager1->id) == 60, "Relationship with Sarah preserved");
    TEST_PASS("Player data integrity verified");

    // Verify villagers
    Entity* villagers[MAX_ENTITIES];
    int villager_count = entity_manager_get_entities_by_type(loaded_state->entity_manager, "Villager", villagers, MAX_ENTITIES);
    TEST_ASSERT(villager_count == 2, "Villager count correct");

    Entity* baker = NULL;
    for (int i = 0; i < villager_count; i++) {
        if (strcmp(villagers[i]->name, "Sarah") == 0) {
            baker = villagers[i];
            break;
        }
    }
    TEST_ASSERT(baker != NULL, "Baker villager found");

    OccupationComponent* baker_occ = (OccupationComponent*)entity_get_component(baker, COMPONENT_OCCUPATION);
    TEST_ASSERT(baker_occ != NULL, "Baker occupation loaded");
    TEST_ASSERT(strcmp(baker_occ->occupation, "Baker") == 0, "Baker occupation preserved");
    TEST_PASS("Villager data integrity verified");

    // Cleanup
    cJSON_Delete(state_json);
    game_state_destroy(state);
    game_state_destroy(loaded_state);

    printf("\n✅ All game state tests passed!\n");
    return true;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    printf("============================================================\n");
    printf("PHASE 1 TESTS: Core Foundation\n");
    printf("Testing Component System, Entity System, and Game State\n");
    printf("============================================================\n");

    bool all_passed = true;

    // Run tests
    if (!test_component_creation()) all_passed = false;
    if (!test_entity_system()) all_passed = false;
    if (!test_game_state()) all_passed = false;

    // Print results
    printf("\n============================================================\n");
    printf("RESULTS: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("============================================================\n");

    if (all_passed && tests_failed == 0) {
        printf("\n🎉 Phase 1: Core Foundation - COMPLETE! 🎉\n");
        printf("\nSuccess Criteria Met:\n");
        printf("✓ Can create entities with components\n");
        printf("✓ Can maintain game state\n");
        printf("✓ Can serialize to JSON\n");
        printf("✓ Can load back from JSON\n");
        printf("✓ Data integrity verified\n");
        return 0;
    } else {
        printf("\n⚠️  Some tests failed. Please review errors above.\n");
        return 1;
    }
}
