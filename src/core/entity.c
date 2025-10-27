/**
 * entity.c
 * Entity System Implementation
 */

#include "entity.h"
#include "../../lib/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// Entity Functions
// ============================================================================

Entity* entity_create(const char* name, const char* entity_type) {
    Entity* entity = (Entity*)malloc(sizeof(Entity));
    if (!entity) return NULL;

    entity->id = -1;  // Will be set by manager
    strncpy(entity->name, name, MAX_ENTITY_NAME - 1);
    entity->name[MAX_ENTITY_NAME - 1] = '\0';
    strncpy(entity->entity_type, entity_type, MAX_ENTITY_TYPE - 1);
    entity->entity_type[MAX_ENTITY_TYPE - 1] = '\0';
    entity->component_count = 0;
    entity->active = true;

    for (int i = 0; i < MAX_COMPONENTS_PER_ENTITY; i++) {
        entity->components[i] = NULL;
    }

    return entity;
}

void entity_destroy(Entity* entity) {
    if (!entity) return;

    // Destroy all components
    for (int i = 0; i < entity->component_count; i++) {
        if (entity->components[i]) {
            component_destroy(entity->components[i]);
        }
    }

    free(entity);
}

bool entity_add_component(Entity* entity, Component* component) {
    if (!entity || !component) return false;

    // Check if component type already exists
    if (entity_get_component(entity, component->type)) {
        return false;
    }

    // Check if we have space
    if (entity->component_count >= MAX_COMPONENTS_PER_ENTITY) {
        return false;
    }

    // Add component
    component->entity_id = entity->id;
    entity->components[entity->component_count] = component;
    entity->component_count++;

    return true;
}

bool entity_remove_component(Entity* entity, ComponentType type) {
    if (!entity) return false;

    for (int i = 0; i < entity->component_count; i++) {
        if (entity->components[i] && entity->components[i]->type == type) {
            // Destroy component
            component_destroy(entity->components[i]);

            // Shift remaining components
            for (int j = i; j < entity->component_count - 1; j++) {
                entity->components[j] = entity->components[j + 1];
            }
            entity->components[entity->component_count - 1] = NULL;
            entity->component_count--;

            return true;
        }
    }

    return false;
}

Component* entity_get_component(const Entity* entity, ComponentType type) {
    if (!entity) return NULL;

    for (int i = 0; i < entity->component_count; i++) {
        if (entity->components[i] && entity->components[i]->type == type) {
            return entity->components[i];
        }
    }

    return NULL;
}

bool entity_has_component(const Entity* entity, ComponentType type) {
    return entity_get_component(entity, type) != NULL;
}

bool entity_has_components(const Entity* entity, const ComponentType* types, int count) {
    if (!entity || !types) return false;

    for (int i = 0; i < count; i++) {
        if (!entity_has_component(entity, types[i])) {
            return false;
        }
    }

    return true;
}

cJSON* entity_to_json(const Entity* entity) {
    if (!entity) return NULL;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "id", entity->id);
    cJSON_AddStringToObject(json, "name", entity->name);
    cJSON_AddStringToObject(json, "entity_type", entity->entity_type);
    cJSON_AddBoolToObject(json, "active", entity->active);

    cJSON* components = cJSON_CreateArray();
    for (int i = 0; i < entity->component_count; i++) {
        if (entity->components[i]) {
            cJSON* comp_json = component_to_json(entity->components[i]);
            if (comp_json) {
                cJSON_AddItemToArray(components, comp_json);
            }
        }
    }
    cJSON_AddItemToObject(json, "components", components);

    return json;
}

Entity* entity_from_json(cJSON* json) {
    if (!json) return NULL;

    cJSON* name = cJSON_GetObjectItem(json, "name");
    cJSON* entity_type = cJSON_GetObjectItem(json, "entity_type");

    Entity* entity = entity_create(
        name ? name->valuestring : "Unnamed",
        entity_type ? entity_type->valuestring : "Generic"
    );

    if (!entity) return NULL;

    cJSON* id = cJSON_GetObjectItem(json, "id");
    if (id) entity->id = id->valueint;

    cJSON* active = cJSON_GetObjectItem(json, "active");
    if (active) entity->active = cJSON_IsTrue(active);

    // Restore components
    cJSON* components = cJSON_GetObjectItem(json, "components");
    if (components) {
        cJSON* comp = NULL;
        cJSON_ArrayForEach(comp, components) {
            Component* component = component_from_json(comp);
            if (component) {
                entity_add_component(entity, component);
            }
        }
    }

    return entity;
}

// ============================================================================
// EntityManager Functions
// ============================================================================

EntityManager* entity_manager_create(void) {
    EntityManager* manager = (EntityManager*)malloc(sizeof(EntityManager));
    if (!manager) return NULL;

    manager->entity_count = 0;
    manager->next_id = 1;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        manager->entities[i] = NULL;
    }

    return manager;
}

void entity_manager_destroy(EntityManager* manager) {
    if (!manager) return;

    // Destroy all entities
    for (int i = 0; i < manager->entity_count; i++) {
        if (manager->entities[i]) {
            entity_destroy(manager->entities[i]);
        }
    }

    free(manager);
}

Entity* entity_manager_create_entity(EntityManager* manager, const char* name, const char* entity_type) {
    if (!manager) return NULL;

    Entity* entity = entity_create(name, entity_type);
    if (!entity) return NULL;

    if (!entity_manager_add_entity(manager, entity)) {
        entity_destroy(entity);
        return NULL;
    }

    return entity;
}

bool entity_manager_add_entity(EntityManager* manager, Entity* entity) {
    if (!manager || !entity) return false;

    if (manager->entity_count >= MAX_ENTITIES) {
        return false;
    }

    // Assign ID
    entity->id = manager->next_id++;

    // Update component entity IDs
    for (int i = 0; i < entity->component_count; i++) {
        if (entity->components[i]) {
            entity->components[i]->entity_id = entity->id;
        }
    }

    // Add to manager
    manager->entities[manager->entity_count] = entity;
    manager->entity_count++;

    return true;
}

bool entity_manager_remove_entity(EntityManager* manager, int entity_id) {
    if (!manager) return false;

    for (int i = 0; i < manager->entity_count; i++) {
        if (manager->entities[i] && manager->entities[i]->id == entity_id) {
            // Destroy entity
            entity_destroy(manager->entities[i]);

            // Shift remaining entities
            for (int j = i; j < manager->entity_count - 1; j++) {
                manager->entities[j] = manager->entities[j + 1];
            }
            manager->entities[manager->entity_count - 1] = NULL;
            manager->entity_count--;

            return true;
        }
    }

    return false;
}

Entity* entity_manager_get_entity(const EntityManager* manager, int entity_id) {
    if (!manager) return NULL;

    for (int i = 0; i < manager->entity_count; i++) {
        if (manager->entities[i] && manager->entities[i]->id == entity_id) {
            return manager->entities[i];
        }
    }

    return NULL;
}

int entity_manager_get_entities_by_type(const EntityManager* manager, const char* entity_type, Entity** out_entities, int max_entities) {
    if (!manager || !out_entities) return 0;

    int count = 0;
    for (int i = 0; i < manager->entity_count && count < max_entities; i++) {
        if (manager->entities[i] && strcmp(manager->entities[i]->entity_type, entity_type) == 0) {
            out_entities[count++] = manager->entities[i];
        }
    }

    return count;
}

int entity_manager_query_entities(const EntityManager* manager, const ComponentType* required_components, int component_count, Entity** out_entities, int max_entities) {
    if (!manager || !out_entities) return 0;

    int count = 0;
    for (int i = 0; i < manager->entity_count && count < max_entities; i++) {
        Entity* entity = manager->entities[i];
        if (entity && entity->active && entity_has_components(entity, required_components, component_count)) {
            out_entities[count++] = entity;
        }
    }

    return count;
}

int entity_manager_get_all_entities(const EntityManager* manager, Entity** out_entities, int max_entities) {
    if (!manager || !out_entities) return 0;

    int count = 0;
    for (int i = 0; i < manager->entity_count && count < max_entities; i++) {
        if (manager->entities[i]) {
            out_entities[count++] = manager->entities[i];
        }
    }

    return count;
}

int entity_manager_count(const EntityManager* manager) {
    return manager ? manager->entity_count : 0;
}

int entity_manager_count_by_type(const EntityManager* manager, const char* entity_type) {
    if (!manager) return 0;

    int count = 0;
    for (int i = 0; i < manager->entity_count; i++) {
        if (manager->entities[i] && strcmp(manager->entities[i]->entity_type, entity_type) == 0) {
            count++;
        }
    }

    return count;
}

void entity_manager_clear(EntityManager* manager) {
    if (!manager) return;

    for (int i = 0; i < manager->entity_count; i++) {
        if (manager->entities[i]) {
            entity_destroy(manager->entities[i]);
            manager->entities[i] = NULL;
        }
    }

    manager->entity_count = 0;
}

cJSON* entity_manager_to_json(const EntityManager* manager) {
    if (!manager) return NULL;

    cJSON* json = cJSON_CreateObject();

    cJSON* entities = cJSON_CreateArray();
    for (int i = 0; i < manager->entity_count; i++) {
        if (manager->entities[i]) {
            cJSON* entity_json = entity_to_json(manager->entities[i]);
            if (entity_json) {
                cJSON_AddItemToArray(entities, entity_json);
            }
        }
    }
    cJSON_AddItemToObject(json, "entities", entities);

    return json;
}

EntityManager* entity_manager_from_json(cJSON* json) {
    if (!json) return NULL;

    EntityManager* manager = entity_manager_create();
    if (!manager) return NULL;

    cJSON* entities = cJSON_GetObjectItem(json, "entities");
    if (entities) {
        cJSON* entity_json = NULL;
        cJSON_ArrayForEach(entity_json, entities) {
            Entity* entity = entity_from_json(entity_json);
            if (entity) {
                entity_manager_add_entity(manager, entity);

                // Update next_id to avoid collisions
                if (entity->id >= manager->next_id) {
                    manager->next_id = entity->id + 1;
                }
            }
        }
    }

    return manager;
}

// ============================================================================
// Helper Functions
// ============================================================================

Entity* create_player_entity(EntityManager* manager, const char* name) {
    Entity* player = entity_manager_create_entity(manager, name, "Player");
    if (!player) return NULL;

    // Add components
    entity_add_component(player, (Component*)position_component_create("YourFarm", 0, 0));
    entity_add_component(player, (Component*)health_component_create(100, 100));
    entity_add_component(player, (Component*)inventory_component_create(20));
    entity_add_component(player, (Component*)currency_component_create(100));
    entity_add_component(player, (Component*)relationship_component_create());

    return player;
}

Entity* create_villager_entity(EntityManager* manager, const char* name, const char* occupation, const char* location) {
    Entity* villager = entity_manager_create_entity(manager, name, "Villager");
    if (!villager) return NULL;

    // Add components
    entity_add_component(villager, (Component*)position_component_create(location, 0, 0));
    entity_add_component(villager, (Component*)health_component_create(100, 100));
    entity_add_component(villager, (Component*)inventory_component_create(15));
    entity_add_component(villager, (Component*)currency_component_create(50));
    entity_add_component(villager, (Component*)relationship_component_create());
    entity_add_component(villager, (Component*)needs_component_create());
    entity_add_component(villager, (Component*)schedule_component_create());
    entity_add_component(villager, (Component*)occupation_component_create(occupation, "WorkPlace", 1));
    entity_add_component(villager, (Component*)memory_component_create(50));
    entity_add_component(villager, (Component*)goal_component_create());

    return villager;
}

Entity* create_crop_entity(EntityManager* manager, const char* crop_type, const char* location, float x, float y) {
    Entity* crop = entity_manager_create_entity(manager, crop_type, "Crop");
    if (!crop) return NULL;

    entity_add_component(crop, (Component*)position_component_create(location, x, y));

    return crop;
}
