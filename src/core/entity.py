"""
Entity System for Transparent Game Engine
Entities are containers for components with unique IDs.
"""

from typing import Dict, List, Optional, Type, Any
from src.core.component import Component, ComponentType


class Entity:
    """
    Entity class - a container for components.
    Entities are defined by their components, not by inheritance.
    """

    _next_id = 1  # Class variable for generating unique IDs

    def __init__(self, name: str = "Unnamed Entity", entity_type: str = "Generic"):
        self.id = Entity._next_id
        Entity._next_id += 1

        self.name = name
        self.entity_type = entity_type  # e.g., "Player", "Villager", "Crop", "Item"
        self.components: Dict[ComponentType, Component] = {}
        self.active = True  # Whether entity is active in the world

    def add_component(self, component: Component) -> bool:
        """
        Add a component to this entity.
        Returns True if successful, False if component type already exists.
        """
        if component.component_type in self.components:
            return False

        component.entity_id = self.id
        self.components[component.component_type] = component
        return True

    def remove_component(self, component_type: ComponentType) -> bool:
        """
        Remove a component from this entity.
        Returns True if successful, False if component doesn't exist.
        """
        if component_type in self.components:
            del self.components[component_type]
            return True
        return False

    def get_component(self, component_type: ComponentType) -> Optional[Component]:
        """Get a component by type. Returns None if not found."""
        return self.components.get(component_type)

    def has_component(self, component_type: ComponentType) -> bool:
        """Check if entity has a component of given type."""
        return component_type in self.components

    def has_components(self, *component_types: ComponentType) -> bool:
        """Check if entity has all specified component types."""
        return all(ct in self.components for ct in component_types)

    def get_components(self) -> List[Component]:
        """Get all components attached to this entity."""
        return list(self.components.values())

    def to_dict(self) -> Dict[str, Any]:
        """Serialize entity to dictionary for saving."""
        return {
            "id": self.id,
            "name": self.name,
            "entity_type": self.entity_type,
            "active": self.active,
            "components": [comp.to_dict() for comp in self.components.values()]
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'Entity':
        """Deserialize entity from dictionary."""
        from src.core.component import create_component_from_dict

        entity = cls(
            name=data.get("name", "Unnamed Entity"),
            entity_type=data.get("entity_type", "Generic")
        )
        entity.id = data.get("id", entity.id)
        entity.active = data.get("active", True)

        # Restore components
        for comp_data in data.get("components", []):
            component = create_component_from_dict(comp_data)
            if component:
                entity.add_component(component)

        return entity

    def __repr__(self):
        return f"Entity({self.id}, '{self.name}', type={self.entity_type}, components={len(self.components)})"


class EntityManager:
    """
    Manages all entities in the game world.
    Provides lookup, query, and lifecycle management.
    """

    def __init__(self):
        self.entities: Dict[int, Entity] = {}  # entity_id -> Entity
        self.entities_by_type: Dict[str, List[int]] = {}  # entity_type -> [entity_ids]

    def create_entity(self, name: str = "Unnamed Entity", entity_type: str = "Generic") -> Entity:
        """Create and register a new entity."""
        entity = Entity(name=name, entity_type=entity_type)
        self.add_entity(entity)
        return entity

    def add_entity(self, entity: Entity) -> bool:
        """
        Add an existing entity to the manager.
        Returns True if successful, False if entity ID already exists.
        """
        if entity.id in self.entities:
            return False

        self.entities[entity.id] = entity

        # Track by type
        if entity.entity_type not in self.entities_by_type:
            self.entities_by_type[entity.entity_type] = []
        self.entities_by_type[entity.entity_type].append(entity.id)

        return True

    def remove_entity(self, entity_id: int) -> bool:
        """
        Remove entity from the manager.
        Returns True if successful, False if entity doesn't exist.
        """
        if entity_id not in self.entities:
            return False

        entity = self.entities[entity_id]

        # Remove from type tracking
        if entity.entity_type in self.entities_by_type:
            if entity_id in self.entities_by_type[entity.entity_type]:
                self.entities_by_type[entity.entity_type].remove(entity_id)

        # Remove entity
        del self.entities[entity_id]
        return True

    def get_entity(self, entity_id: int) -> Optional[Entity]:
        """Get entity by ID. Returns None if not found."""
        return self.entities.get(entity_id)

    def get_entities_by_type(self, entity_type: str) -> List[Entity]:
        """Get all entities of a specific type."""
        entity_ids = self.entities_by_type.get(entity_type, [])
        return [self.entities[eid] for eid in entity_ids if eid in self.entities]

    def get_all_entities(self) -> List[Entity]:
        """Get all entities."""
        return list(self.entities.values())

    def get_active_entities(self) -> List[Entity]:
        """Get all active entities."""
        return [entity for entity in self.entities.values() if entity.active]

    def query_entities(self, *required_components: ComponentType) -> List[Entity]:
        """
        Query entities that have all specified components.
        Example: query_entities(ComponentType.POSITION, ComponentType.HEALTH)
        """
        results = []
        for entity in self.entities.values():
            if entity.active and entity.has_components(*required_components):
                results.append(entity)
        return results

    def count_entities(self) -> int:
        """Get total number of entities."""
        return len(self.entities)

    def count_entities_by_type(self, entity_type: str) -> int:
        """Get count of entities of specific type."""
        return len(self.entities_by_type.get(entity_type, []))

    def clear(self):
        """Remove all entities."""
        self.entities.clear()
        self.entities_by_type.clear()

    def to_dict(self) -> Dict[str, Any]:
        """Serialize all entities to dictionary."""
        return {
            "entities": [entity.to_dict() for entity in self.entities.values()]
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'EntityManager':
        """Deserialize entity manager from dictionary."""
        manager = cls()

        for entity_data in data.get("entities", []):
            entity = Entity.from_dict(entity_data)
            manager.add_entity(entity)

            # Update next_id to avoid collisions
            if entity.id >= Entity._next_id:
                Entity._next_id = entity.id + 1

        return manager


# Helper functions for creating common entity archetypes

def create_player_entity(entity_manager: EntityManager, name: str = "Player") -> Entity:
    """Create a player entity with common components."""
    from src.core.component import (
        PositionComponent, HealthComponent, InventoryComponent,
        CurrencyComponent, RelationshipComponent
    )

    player = entity_manager.create_entity(name=name, entity_type="Player")
    player.add_component(PositionComponent(location="YourFarm"))
    player.add_component(HealthComponent(current=100, maximum=100))
    player.add_component(InventoryComponent(capacity=20))
    player.add_component(CurrencyComponent(amount=100))
    player.add_component(RelationshipComponent())

    return player


def create_villager_entity(
    entity_manager: EntityManager,
    name: str,
    occupation: str,
    location: str = "VillageSquare"
) -> Entity:
    """Create a villager entity with common components."""
    from src.core.component import (
        PositionComponent, HealthComponent, InventoryComponent,
        CurrencyComponent, RelationshipComponent, NeedsComponent,
        ScheduleComponent, OccupationComponent, MemoryComponent, GoalComponent
    )

    villager = entity_manager.create_entity(name=name, entity_type="Villager")
    villager.add_component(PositionComponent(location=location))
    villager.add_component(HealthComponent(current=100, maximum=100))
    villager.add_component(InventoryComponent(capacity=15))
    villager.add_component(CurrencyComponent(amount=50))
    villager.add_component(RelationshipComponent())
    villager.add_component(NeedsComponent())
    villager.add_component(ScheduleComponent())
    villager.add_component(OccupationComponent(occupation=occupation))
    villager.add_component(MemoryComponent())
    villager.add_component(GoalComponent())

    return villager


def create_crop_entity(
    entity_manager: EntityManager,
    crop_type: str,
    location: str,
    x: float = 0.0,
    y: float = 0.0
) -> Entity:
    """Create a crop entity."""
    from src.core.component import PositionComponent

    crop = entity_manager.create_entity(name=crop_type, entity_type="Crop")
    crop.add_component(PositionComponent(location=location, x=x, y=y))

    return crop
