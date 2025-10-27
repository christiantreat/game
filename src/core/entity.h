/**
 * entity.h
 * Entity System for Transparent Game Engine
 *
 * Entities are containers for components with unique IDs.
 * EntityManager handles lifecycle and queries.
 */

#ifndef ENTITY_H
#define ENTITY_H

#include "component.h"
#include <stdbool.h>

#define MAX_ENTITY_NAME 64
#define MAX_ENTITY_TYPE 32
#define MAX_ENTITIES 1000
#define MAX_COMPONENTS_PER_ENTITY 16

// Entity structure
typedef struct {
    int id;
    char name[MAX_ENTITY_NAME];
    char entity_type[MAX_ENTITY_TYPE];  // e.g., "Player", "Villager", "Crop"
    Component* components[MAX_COMPONENTS_PER_ENTITY];
    int component_count;
    bool active;
} Entity;

// Entity Manager structure
typedef struct {
    Entity* entities[MAX_ENTITIES];
    int entity_count;
    int next_id;
} EntityManager;

// ============================================================================
// Entity Functions
// ============================================================================

Entity* entity_create(const char* name, const char* entity_type);
void entity_destroy(Entity* entity);

bool entity_add_component(Entity* entity, Component* component);
bool entity_remove_component(Entity* entity, ComponentType type);
Component* entity_get_component(const Entity* entity, ComponentType type);
bool entity_has_component(const Entity* entity, ComponentType type);
bool entity_has_components(const Entity* entity, const ComponentType* types, int count);

cJSON* entity_to_json(const Entity* entity);
Entity* entity_from_json(cJSON* json);

// ============================================================================
// EntityManager Functions
// ============================================================================

EntityManager* entity_manager_create(void);
void entity_manager_destroy(EntityManager* manager);

Entity* entity_manager_create_entity(EntityManager* manager, const char* name, const char* entity_type);
bool entity_manager_add_entity(EntityManager* manager, Entity* entity);
bool entity_manager_remove_entity(EntityManager* manager, int entity_id);

Entity* entity_manager_get_entity(const EntityManager* manager, int entity_id);
int entity_manager_get_entities_by_type(const EntityManager* manager, const char* entity_type, Entity** out_entities, int max_entities);
int entity_manager_query_entities(const EntityManager* manager, const ComponentType* required_components, int component_count, Entity** out_entities, int max_entities);
int entity_manager_get_all_entities(const EntityManager* manager, Entity** out_entities, int max_entities);

int entity_manager_count(const EntityManager* manager);
int entity_manager_count_by_type(const EntityManager* manager, const char* entity_type);

void entity_manager_clear(EntityManager* manager);

cJSON* entity_manager_to_json(const EntityManager* manager);
EntityManager* entity_manager_from_json(cJSON* json);

// ============================================================================
// Helper Functions for Creating Common Entity Archetypes
// ============================================================================

Entity* create_player_entity(EntityManager* manager, const char* name);
Entity* create_villager_entity(EntityManager* manager, const char* name, const char* occupation, const char* location);
Entity* create_crop_entity(EntityManager* manager, const char* crop_type, const char* location, float x, float y);

#endif // ENTITY_H
