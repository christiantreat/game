"""
Core systems for the game engine.
"""

from src.core.component import (
    Component,
    ComponentType,
    PositionComponent,
    HealthComponent,
    InventoryComponent,
    CurrencyComponent,
    RelationshipComponent,
    NeedsComponent,
    ScheduleComponent,
    OccupationComponent,
    MemoryComponent,
    GoalComponent,
)

from src.core.entity import (
    Entity,
    EntityManager,
    create_player_entity,
    create_villager_entity,
    create_crop_entity,
)

from src.core.game_state import (
    GameState,
    TimeOfDay,
    Season,
    Weather,
)

__all__ = [
    # Components
    'Component',
    'ComponentType',
    'PositionComponent',
    'HealthComponent',
    'InventoryComponent',
    'CurrencyComponent',
    'RelationshipComponent',
    'NeedsComponent',
    'ScheduleComponent',
    'OccupationComponent',
    'MemoryComponent',
    'GoalComponent',
    # Entities
    'Entity',
    'EntityManager',
    'create_player_entity',
    'create_villager_entity',
    'create_crop_entity',
    # Game State
    'GameState',
    'TimeOfDay',
    'Season',
    'Weather',
]
