/**
 * world.c
 * Game World and Location Implementation
 */

#include "world.h"
#include "../lib/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// ============================================================================
// Location Type Helpers
// ============================================================================

const char* location_type_to_string(LocationType type) {
    switch (type) {
        case LOCATION_OUTDOOR: return "Outdoor";
        case LOCATION_INDOOR: return "Indoor";
        case LOCATION_FIELD: return "Field";
        case LOCATION_SHOP: return "Shop";
        case LOCATION_HOME: return "Home";
        case LOCATION_WORKSHOP: return "Workshop";
        case LOCATION_ROAD: return "Road";
        case LOCATION_WATER: return "Water";
        case LOCATION_FOREST: return "Forest";
        case LOCATION_VILLAGE_CENTER: return "Village Center";
        default: return "Unknown";
    }
}

// ============================================================================
// Location Implementation
// ============================================================================

Location* location_create(int id, const char* name, LocationType type, float x, float y) {
    if (!name) return NULL;

    Location* location = calloc(1, sizeof(Location));
    if (!location) return NULL;

    location->id = id;
    strncpy(location->name, name, MAX_LOCATION_NAME - 1);
    location->type = type;
    location->x = x;
    location->y = y;
    location->width = 10.0f;   // Default size
    location->height = 10.0f;
    location->indoor = (type == LOCATION_INDOOR || type == LOCATION_SHOP ||
                       type == LOCATION_HOME || type == LOCATION_WORKSHOP);
    location->protected_from_weather = location->indoor;
    location->capacity = 10;   // Default capacity
    location->connection_count = 0;
    location->entity_count = 0;

    // Set default capabilities based on type
    location->can_rest = (type == LOCATION_HOME || type == LOCATION_INDOOR);
    location->can_work = (type == LOCATION_WORKSHOP || type == LOCATION_SHOP);
    location->can_shop = (type == LOCATION_SHOP);
    location->can_farm = (type == LOCATION_FIELD);

    location->description[0] = '\0';

    return location;
}

void location_destroy(Location* location) {
    if (location) {
        free(location);
    }
}

bool location_add_connection(Location* location, int target_id, float distance, const char* description) {
    if (!location || location->connection_count >= MAX_LOCATION_CONNECTIONS) {
        return false;
    }

    // Check if already connected
    for (int i = 0; i < location->connection_count; i++) {
        if (location->connections[i].location_id == target_id) {
            return false;  // Already connected
        }
    }

    LocationConnection* conn = &location->connections[location->connection_count];
    conn->location_id = target_id;
    conn->distance = distance;
    conn->blocked = false;

    if (description) {
        strncpy(conn->description, description, 127);
        conn->description[127] = '\0';
    } else {
        conn->description[0] = '\0';
    }

    location->connection_count++;
    return true;
}

bool location_remove_connection(Location* location, int target_id) {
    if (!location) return false;

    for (int i = 0; i < location->connection_count; i++) {
        if (location->connections[i].location_id == target_id) {
            // Shift remaining connections down
            for (int j = i; j < location->connection_count - 1; j++) {
                location->connections[j] = location->connections[j + 1];
            }
            location->connection_count--;
            return true;
        }
    }

    return false;
}

bool location_set_connection_blocked(Location* location, int target_id, bool blocked) {
    if (!location) return false;

    for (int i = 0; i < location->connection_count; i++) {
        if (location->connections[i].location_id == target_id) {
            location->connections[i].blocked = blocked;
            return true;
        }
    }

    return false;
}

bool location_is_connected(const Location* location, int target_id) {
    if (!location) return false;

    for (int i = 0; i < location->connection_count; i++) {
        if (location->connections[i].location_id == target_id &&
            !location->connections[i].blocked) {
            return true;
        }
    }

    return false;
}

float location_get_connection_distance(const Location* location, int target_id) {
    if (!location) return -1.0f;

    for (int i = 0; i < location->connection_count; i++) {
        if (location->connections[i].location_id == target_id) {
            return location->connections[i].distance;
        }
    }

    return -1.0f;  // Not connected
}

bool location_add_entity(Location* location, int entity_id) {
    if (!location || location->entity_count >= MAX_ENTITIES_PER_LOCATION ||
        location->entity_count >= location->capacity) {
        return false;
    }

    // Check if already here
    for (int i = 0; i < location->entity_count; i++) {
        if (location->entity_ids[i] == entity_id) {
            return false;
        }
    }

    location->entity_ids[location->entity_count++] = entity_id;
    return true;
}

bool location_remove_entity(Location* location, int entity_id) {
    if (!location) return false;

    for (int i = 0; i < location->entity_count; i++) {
        if (location->entity_ids[i] == entity_id) {
            // Shift remaining entities down
            for (int j = i; j < location->entity_count - 1; j++) {
                location->entity_ids[j] = location->entity_ids[j + 1];
            }
            location->entity_count--;
            return true;
        }
    }

    return false;
}

bool location_has_entity(const Location* location, int entity_id) {
    if (!location) return false;

    for (int i = 0; i < location->entity_count; i++) {
        if (location->entity_ids[i] == entity_id) {
            return true;
        }
    }

    return false;
}

int location_get_entity_count(const Location* location) {
    return location ? location->entity_count : 0;
}

bool location_is_full(const Location* location) {
    if (!location) return true;
    return location->entity_count >= location->capacity;
}

float location_distance_to(const Location* from, const Location* to) {
    if (!from || !to) return -1.0f;

    float dx = to->x - from->x;
    float dy = to->y - from->y;
    return sqrtf(dx * dx + dy * dy);
}

cJSON* location_to_json(const Location* location) {
    if (!location) return NULL;

    cJSON* json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "id", location->id);
    cJSON_AddStringToObject(json, "name", location->name);
    cJSON_AddStringToObject(json, "description", location->description);
    cJSON_AddNumberToObject(json, "type", location->type);
    cJSON_AddNumberToObject(json, "x", location->x);
    cJSON_AddNumberToObject(json, "y", location->y);
    cJSON_AddNumberToObject(json, "width", location->width);
    cJSON_AddNumberToObject(json, "height", location->height);
    cJSON_AddBoolToObject(json, "indoor", location->indoor);
    cJSON_AddNumberToObject(json, "capacity", location->capacity);

    // Connections
    cJSON* connections = cJSON_CreateArray();
    for (int i = 0; i < location->connection_count; i++) {
        cJSON* conn = cJSON_CreateObject();
        cJSON_AddNumberToObject(conn, "location_id", location->connections[i].location_id);
        cJSON_AddNumberToObject(conn, "distance", location->connections[i].distance);
        cJSON_AddBoolToObject(conn, "blocked", location->connections[i].blocked);
        cJSON_AddStringToObject(conn, "description", location->connections[i].description);
        cJSON_AddItemToArray(connections, conn);
    }
    cJSON_AddItemToObject(json, "connections", connections);

    return json;
}

Location* location_from_json(cJSON* json) {
    if (!json) return NULL;

    int id = cJSON_GetObjectItem(json, "id")->valueint;
    const char* name = cJSON_GetObjectItem(json, "name")->valuestring;
    LocationType type = cJSON_GetObjectItem(json, "type")->valueint;
    float x = (float)cJSON_GetObjectItem(json, "x")->valuedouble;
    float y = (float)cJSON_GetObjectItem(json, "y")->valuedouble;

    Location* location = location_create(id, name, type, x, y);
    if (!location) return NULL;

    // Load other properties
    cJSON* desc = cJSON_GetObjectItem(json, "description");
    if (desc) strncpy(location->description, desc->valuestring, MAX_LOCATION_DESCRIPTION - 1);

    cJSON* width = cJSON_GetObjectItem(json, "width");
    if (width) location->width = (float)width->valuedouble;

    cJSON* height = cJSON_GetObjectItem(json, "height");
    if (height) location->height = (float)height->valuedouble;

    // Load connections
    cJSON* connections = cJSON_GetObjectItem(json, "connections");
    if (connections) {
        cJSON* conn = NULL;
        cJSON_ArrayForEach(conn, connections) {
            int target_id = cJSON_GetObjectItem(conn, "location_id")->valueint;
            float distance = (float)cJSON_GetObjectItem(conn, "distance")->valuedouble;
            const char* desc = cJSON_GetObjectItem(conn, "description")->valuestring;
            location_add_connection(location, target_id, distance, desc);

            cJSON* blocked = cJSON_GetObjectItem(conn, "blocked");
            if (blocked && blocked->valueint) {
                location_set_connection_blocked(location, target_id, true);
            }
        }
    }

    return location;
}

// ============================================================================
// World Implementation
// ============================================================================

World* world_create(const char* name, float width, float height) {
    World* world = calloc(1, sizeof(World));
    if (!world) return NULL;

    if (name) {
        strncpy(world->world_name, name, 63);
    } else {
        strcpy(world->world_name, "Unnamed World");
    }

    world->world_width = width;
    world->world_height = height;
    world->location_count = 0;
    world->next_location_id = 1;

    return world;
}

void world_destroy(World* world) {
    if (!world) return;

    for (int i = 0; i < world->location_count; i++) {
        location_destroy(world->locations[i]);
    }

    free(world);
}

int world_add_location(World* world, const char* name, LocationType type, float x, float y) {
    if (!world || world->location_count >= MAX_LOCATIONS) {
        return -1;
    }

    int id = world->next_location_id++;
    Location* location = location_create(id, name, type, x, y);
    if (!location) return -1;

    world->locations[world->location_count++] = location;
    return id;
}

bool world_remove_location(World* world, int location_id) {
    if (!world) return false;

    for (int i = 0; i < world->location_count; i++) {
        if (world->locations[i]->id == location_id) {
            location_destroy(world->locations[i]);

            // Shift remaining locations down
            for (int j = i; j < world->location_count - 1; j++) {
                world->locations[j] = world->locations[j + 1];
            }

            world->location_count--;

            // Remove connections to this location from other locations
            for (int j = 0; j < world->location_count; j++) {
                location_remove_connection(world->locations[j], location_id);
            }

            return true;
        }
    }

    return false;
}

Location* world_get_location(World* world, int location_id) {
    if (!world) return NULL;

    for (int i = 0; i < world->location_count; i++) {
        if (world->locations[i]->id == location_id) {
            return world->locations[i];
        }
    }

    return NULL;
}

Location* world_get_location_by_name(World* world, const char* name) {
    if (!world || !name) return NULL;

    for (int i = 0; i < world->location_count; i++) {
        if (strcmp(world->locations[i]->name, name) == 0) {
            return world->locations[i];
        }
    }

    return NULL;
}

Location* world_get_location_at(World* world, float x, float y) {
    if (!world) return NULL;

    // Find location containing this point
    for (int i = 0; i < world->location_count; i++) {
        Location* loc = world->locations[i];
        if (x >= loc->x && x < loc->x + loc->width &&
            y >= loc->y && y < loc->y + loc->height) {
            return loc;
        }
    }

    return NULL;
}

int world_get_locations_by_type(const World* world, LocationType type,
                                Location** out_locations, int max_locations) {
    if (!world || !out_locations) return 0;

    int count = 0;
    for (int i = 0; i < world->location_count && count < max_locations; i++) {
        if (world->locations[i]->type == type) {
            out_locations[count++] = world->locations[i];
        }
    }

    return count;
}

bool world_connect_locations(World* world, int location_a, int location_b,
                             float distance, const char* description) {
    if (!world) return false;

    Location* loc_a = world_get_location(world, location_a);
    Location* loc_b = world_get_location(world, location_b);

    if (!loc_a || !loc_b) return false;

    // Add bidirectional connection
    bool success_a = location_add_connection(loc_a, location_b, distance, description);
    bool success_b = location_add_connection(loc_b, location_a, distance, description);

    return success_a && success_b;
}

// BFS pathfinding
int world_find_path(const World* world, int start_id, int end_id,
                    int* out_path, int max_path_length) {
    if (!world || !out_path || max_path_length < 2) return 0;

    // Simple BFS implementation
    int queue[MAX_LOCATIONS];
    int parent[MAX_LOCATIONS];
    bool visited[MAX_LOCATIONS] = {false};

    for (int i = 0; i < MAX_LOCATIONS; i++) {
        parent[i] = -1;
    }

    int queue_start = 0;
    int queue_end = 0;

    queue[queue_end++] = start_id;
    visited[start_id] = true;

    while (queue_start < queue_end) {
        int current_id = queue[queue_start++];

        if (current_id == end_id) {
            // Found path - reconstruct it
            int path_length = 0;
            int temp_path[MAX_PATH_LENGTH];
            int node = end_id;

            while (node != -1 && path_length < MAX_PATH_LENGTH) {
                temp_path[path_length++] = node;
                node = parent[node];
            }

            // Reverse path (it's backwards)
            int result_length = (path_length < max_path_length) ? path_length : max_path_length;
            for (int i = 0; i < result_length; i++) {
                out_path[i] = temp_path[path_length - 1 - i];
            }

            return result_length;
        }

        // Explore neighbors
        Location* current_loc = world_get_location((World*)world, current_id);
        if (!current_loc) continue;

        for (int i = 0; i < current_loc->connection_count; i++) {
            int neighbor_id = current_loc->connections[i].location_id;

            if (!current_loc->connections[i].blocked && !visited[neighbor_id]) {
                visited[neighbor_id] = true;
                parent[neighbor_id] = current_id;
                queue[queue_end++] = neighbor_id;
            }
        }
    }

    return 0;  // No path found
}

float world_get_path_distance(const World* world, const int* path, int path_length) {
    if (!world || !path || path_length < 2) return 0.0f;

    float total_distance = 0.0f;

    for (int i = 0; i < path_length - 1; i++) {
        Location* loc = world_get_location((World*)world, path[i]);
        if (loc) {
            float dist = location_get_connection_distance(loc, path[i + 1]);
            if (dist >= 0.0f) {
                total_distance += dist;
            }
        }
    }

    return total_distance;
}

int world_get_entities_at_location(const World* world, int location_id,
                                   int* out_entity_ids, int max_entities) {
    if (!world || !out_entity_ids) return 0;

    Location* loc = world_get_location((World*)world, location_id);
    if (!loc) return 0;

    int count = (loc->entity_count < max_entities) ? loc->entity_count : max_entities;
    for (int i = 0; i < count; i++) {
        out_entity_ids[i] = loc->entity_ids[i];
    }

    return count;
}

Location* world_find_nearest_location(const World* world, float x, float y, LocationType type) {
    if (!world) return NULL;

    Location* nearest = NULL;
    float min_distance = INFINITY;

    for (int i = 0; i < world->location_count; i++) {
        Location* loc = world->locations[i];
        if (loc->type == type) {
            float dx = loc->x - x;
            float dy = loc->y - y;
            float distance = sqrtf(dx * dx + dy * dy);

            if (distance < min_distance) {
                min_distance = distance;
                nearest = loc;
            }
        }
    }

    return nearest;
}

bool world_move_entity(World* world, int entity_id, int from_location_id, int to_location_id) {
    if (!world) return false;

    Location* from_loc = world_get_location(world, from_location_id);
    Location* to_loc = world_get_location(world, to_location_id);

    if (!to_loc) return false;

    // Check capacity
    if (location_is_full(to_loc)) return false;

    // Remove from old location (if specified)
    if (from_loc) {
        location_remove_entity(from_loc, entity_id);
    }

    // Add to new location
    return location_add_entity(to_loc, entity_id);
}

Location* world_get_entity_location(const World* world, int entity_id) {
    if (!world) return NULL;

    for (int i = 0; i < world->location_count; i++) {
        if (location_has_entity(world->locations[i], entity_id)) {
            return world->locations[i];
        }
    }

    return NULL;
}

cJSON* world_to_json(const World* world) {
    if (!world) return NULL;

    cJSON* json = cJSON_CreateObject();

    cJSON_AddStringToObject(json, "world_name", world->world_name);
    cJSON_AddNumberToObject(json, "world_width", world->world_width);
    cJSON_AddNumberToObject(json, "world_height", world->world_height);

    cJSON* locations = cJSON_CreateArray();
    for (int i = 0; i < world->location_count; i++) {
        cJSON_AddItemToArray(locations, location_to_json(world->locations[i]));
    }
    cJSON_AddItemToObject(json, "locations", locations);

    return json;
}

World* world_from_json(cJSON* json) {
    if (!json) return NULL;

    const char* name = cJSON_GetObjectItem(json, "world_name")->valuestring;
    float width = (float)cJSON_GetObjectItem(json, "world_width")->valuedouble;
    float height = (float)cJSON_GetObjectItem(json, "world_height")->valuedouble;

    World* world = world_create(name, width, height);
    if (!world) return NULL;

    cJSON* locations = cJSON_GetObjectItem(json, "locations");
    if (locations) {
        cJSON* loc = NULL;
        cJSON_ArrayForEach(loc, locations) {
            Location* location = location_from_json(loc);
            if (location) {
                world->locations[world->location_count++] = location;
                if (location->id >= world->next_location_id) {
                    world->next_location_id = location->id + 1;
                }
            }
        }
    }

    return world;
}

void world_print(const World* world) {
    if (!world) return;

    printf("\n=== World: %s ===\n", world->world_name);
    printf("Size: %.1f x %.1f\n", world->world_width, world->world_height);
    printf("Locations: %d\n", world->location_count);

    for (int i = 0; i < world->location_count; i++) {
        world_print_location(world->locations[i]);
    }

    printf("==================\n\n");
}

void world_print_location(const Location* location) {
    if (!location) return;

    printf("\n  Location #%d: %s\n", location->id, location->name);
    printf("    Type: %s\n", location_type_to_string(location->type));
    printf("    Position: (%.1f, %.1f)\n", location->x, location->y);
    printf("    Indoor: %s\n", location->indoor ? "Yes" : "No");
    printf("    Entities: %d/%d\n", location->entity_count, location->capacity);
    printf("    Connections: %d\n", location->connection_count);
}

void world_print_connections(const World* world, int location_id) {
    if (!world) return;

    Location* loc = world_get_location((World*)world, location_id);
    if (!loc) return;

    printf("\n  Connections from %s:\n", loc->name);
    for (int i = 0; i < loc->connection_count; i++) {
        LocationConnection* conn = &loc->connections[i];
        Location* target = world_get_location((World*)world, conn->location_id);
        if (target) {
            printf("    -> %s (distance: %.1f)%s\n",
                   target->name, conn->distance,
                   conn->blocked ? " [BLOCKED]" : "");
        }
    }
}

// ============================================================================
// World Builders
// ============================================================================

World* create_farming_village_world(void) {
    World* world = world_create("Farming Village", 200.0f, 200.0f);
    if (!world) return NULL;

    // Add locations
    add_village_center(world, 100.0f, 100.0f);
    add_farm_area(world, 50.0f, 150.0f);
    add_shop_area(world, 150.0f, 100.0f);
    add_residential_area(world, 100.0f, 50.0f, 3);

    return world;
}

void add_village_center(World* world, float x, float y) {
    if (!world) return;

    int center_id = world_add_location(world, "Village Square", LOCATION_VILLAGE_CENTER, x, y);
    Location* center = world_get_location(world, center_id);
    if (center) {
        center->width = 30.0f;
        center->height = 30.0f;
        center->capacity = 50;
        strcpy(center->description, "The heart of the village where everyone gathers");
    }
}

void add_farm_area(World* world, float x, float y) {
    if (!world) return;

    // Add fields
    int field1_id = world_add_location(world, "West Field", LOCATION_FIELD, x, y);
    int field2_id = world_add_location(world, "East Field", LOCATION_FIELD, x + 30, y);
    int barn_id = world_add_location(world, "Barn", LOCATION_WORKSHOP, x + 15, y + 20);

    // Connect fields to barn
    world_connect_locations(world, field1_id, barn_id, 15.0f, "Path to barn");
    world_connect_locations(world, field2_id, barn_id, 15.0f, "Path to barn");

    // Connect to village center (if it exists)
    Location* center = world_get_location_by_name(world, "Village Square");
    if (center) {
        world_connect_locations(world, field1_id, center->id, 20.0f, "Road to village");
    }
}

void add_shop_area(World* world, float x, float y) {
    if (!world) return;

    int shop_id = world_add_location(world, "General Store", LOCATION_SHOP, x, y);
    Location* shop = world_get_location(world, shop_id);
    if (shop) {
        shop->width = 15.0f;
        shop->height = 15.0f;
        shop->can_shop = true;
        strcpy(shop->description, "A general store selling goods and supplies");
    }

    // Connect to village center
    Location* center = world_get_location_by_name(world, "Village Square");
    if (center) {
        world_connect_locations(world, shop_id, center->id, 10.0f, "Main street");
    }
}

void add_residential_area(World* world, float x, float y, int house_count) {
    if (!world) return;

    for (int i = 0; i < house_count; i++) {
        char house_name[64];
        snprintf(house_name, 64, "House %d", i + 1);

        float house_x = x + (i * 20.0f);
        int house_id = world_add_location(world, house_name, LOCATION_HOME, house_x, y);

        Location* house = world_get_location(world, house_id);
        if (house) {
            house->width = 15.0f;
            house->height = 15.0f;
            house->capacity = 5;
            house->can_rest = true;
        }

        // Connect to village center
        Location* center = world_get_location_by_name(world, "Village Square");
        if (center) {
            world_connect_locations(world, house_id, center->id, 15.0f, "Residential street");
        }
    }
}
