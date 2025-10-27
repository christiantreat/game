/**
 * world.h
 * Game World and Location System
 *
 * Manages locations, spatial relationships, and navigation.
 */

#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <stdint.h>
#include "game_state.h"
#include "entity.h"

// Forward declarations
typedef struct cJSON cJSON;

#define MAX_LOCATION_NAME 64
#define MAX_LOCATION_DESCRIPTION 256
#define MAX_LOCATION_CONNECTIONS 10
#define MAX_LOCATIONS 100
#define MAX_ENTITIES_PER_LOCATION 50
#define MAX_PATH_LENGTH 20

// ============================================================================
// Location Types
// ============================================================================

typedef enum {
    LOCATION_OUTDOOR,       // Outdoor area
    LOCATION_INDOOR,        // Indoor building
    LOCATION_FIELD,         // Farm field
    LOCATION_SHOP,          // Shop or store
    LOCATION_HOME,          // Residential building
    LOCATION_WORKSHOP,      // Workshop or workplace
    LOCATION_ROAD,          // Road or path
    LOCATION_WATER,         // Water body
    LOCATION_FOREST,        // Forest area
    LOCATION_VILLAGE_CENTER // Town square or center
} LocationType;

const char* location_type_to_string(LocationType type);

// ============================================================================
// Location Connection
// ============================================================================

typedef struct {
    int location_id;        // ID of connected location
    float distance;         // Distance to travel
    bool blocked;           // Is path blocked?
    char description[128];  // Description of connection (e.g., "North door")
} LocationConnection;

// ============================================================================
// Location
// ============================================================================

typedef struct {
    int id;
    char name[MAX_LOCATION_NAME];
    char description[MAX_LOCATION_DESCRIPTION];
    LocationType type;

    // Spatial properties
    float x, y;             // Position in world coordinates
    float width, height;    // Size of location

    // Environmental properties
    bool indoor;            // Is it indoors?
    bool protected_from_weather;  // Does weather affect this location?
    int capacity;           // Max entities that can be here

    // Connections to other locations
    LocationConnection connections[MAX_LOCATION_CONNECTIONS];
    int connection_count;

    // Entities currently at this location
    int entity_ids[MAX_ENTITIES_PER_LOCATION];
    int entity_count;

    // Gameplay properties
    bool can_rest;          // Can entities rest here?
    bool can_work;          // Can entities work here?
    bool can_shop;          // Can entities shop here?
    bool can_farm;          // Can entities farm here?

} Location;

// Create location
Location* location_create(int id, const char* name, LocationType type, float x, float y);

// Destroy location
void location_destroy(Location* location);

// Add connection to another location
bool location_add_connection(Location* location, int target_id, float distance, const char* description);

// Remove connection
bool location_remove_connection(Location* location, int target_id);

// Block/unblock connection
bool location_set_connection_blocked(Location* location, int target_id, bool blocked);

// Check if connected to another location
bool location_is_connected(const Location* location, int target_id);

// Get connection distance
float location_get_connection_distance(const Location* location, int target_id);

// Entity management
bool location_add_entity(Location* location, int entity_id);
bool location_remove_entity(Location* location, int entity_id);
bool location_has_entity(const Location* location, int entity_id);
int location_get_entity_count(const Location* location);

// Check if location is full
bool location_is_full(const Location* location);

// Calculate distance between locations (world coordinates)
float location_distance_to(const Location* from, const Location* to);

// Serialization
cJSON* location_to_json(const Location* location);
Location* location_from_json(cJSON* json);

// ============================================================================
// World Manager
// ============================================================================

typedef struct {
    Location* locations[MAX_LOCATIONS];
    int location_count;
    int next_location_id;

    char world_name[64];
    float world_width;
    float world_height;

} World;

// Create world
World* world_create(const char* name, float width, float height);

// Destroy world
void world_destroy(World* world);

// Add location to world
int world_add_location(World* world, const char* name, LocationType type, float x, float y);

// Remove location from world
bool world_remove_location(World* world, int location_id);

// Get location by ID
Location* world_get_location(World* world, int location_id);

// Get location by name
Location* world_get_location_by_name(World* world, const char* name);

// Get location by position
Location* world_get_location_at(World* world, float x, float y);

// Get all locations of a specific type
int world_get_locations_by_type(const World* world, LocationType type,
                                Location** out_locations, int max_locations);

// Connect two locations
bool world_connect_locations(World* world, int location_a, int location_b,
                             float distance, const char* description);

// Find path between locations (BFS)
int world_find_path(const World* world, int start_id, int end_id,
                    int* out_path, int max_path_length);

// Get distance along path
float world_get_path_distance(const World* world, const int* path, int path_length);

// Get all entities at a location
int world_get_entities_at_location(const World* world, int location_id,
                                   int* out_entity_ids, int max_entities);

// Find nearest location of a type
Location* world_find_nearest_location(const World* world, float x, float y, LocationType type);

// Update entity position (moves entity between locations)
bool world_move_entity(World* world, int entity_id, int from_location_id, int to_location_id);

// Get entity's current location
Location* world_get_entity_location(const World* world, int entity_id);

// Serialization
cJSON* world_to_json(const World* world);
World* world_from_json(cJSON* json);

// Debug printing
void world_print(const World* world);
void world_print_location(const Location* location);
void world_print_connections(const World* world, int location_id);

// ============================================================================
// World Builder (for creating common world layouts)
// ============================================================================

// Create a simple farming village world
World* create_farming_village_world(void);

// Add common locations to a world
void add_village_center(World* world, float x, float y);
void add_farm_area(World* world, float x, float y);
void add_shop_area(World* world, float x, float y);
void add_residential_area(World* world, float x, float y, int house_count);

#endif // WORLD_H
