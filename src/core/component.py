"""
Component System for Transparent Game Engine
Components are data containers that can be attached to entities.
"""

from typing import Any, Dict, Optional, List
from enum import Enum
from dataclasses import dataclass, field, asdict
import json


class ComponentType(Enum):
    """Enumeration of all component types in the system"""
    POSITION = "position"
    HEALTH = "health"
    INVENTORY = "inventory"
    AI = "ai"
    SENSORY = "sensory"
    MEMORY = "memory"
    GOAL = "goal"
    RELATIONSHIP = "relationship"
    SCHEDULE = "schedule"
    OCCUPATION = "occupation"
    NEEDS = "needs"
    CURRENCY = "currency"


class Component:
    """
    Base Component class - holds data that can be attached to entities.
    All components should inherit from this class.
    """

    def __init__(self, component_type: ComponentType):
        self.component_type = component_type
        self.entity_id: Optional[int] = None  # Set when attached to an entity

    def to_dict(self) -> Dict[str, Any]:
        """Serialize component to dictionary for saving"""
        return {
            "type": self.component_type.value,
            "entity_id": self.entity_id
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'Component':
        """Deserialize component from dictionary"""
        raise NotImplementedError("Subclasses must implement from_dict")


@dataclass
class PositionComponent(Component):
    """Component for entity location in the game world"""
    location: str = "Unknown"  # Location name (e.g., "YourFarm", "VillageSquare")
    x: float = 0.0  # Position within location (for future 2D upgrade)
    y: float = 0.0

    def __post_init__(self):
        super().__init__(ComponentType.POSITION)

    def to_dict(self) -> Dict[str, Any]:
        base = super().to_dict()
        base.update({
            "location": self.location,
            "x": self.x,
            "y": self.y
        })
        return base

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'PositionComponent':
        return cls(
            location=data.get("location", "Unknown"),
            x=data.get("x", 0.0),
            y=data.get("y", 0.0)
        )


@dataclass
class HealthComponent(Component):
    """Component for entity health and vitality"""
    current: int = 100
    maximum: int = 100

    def __post_init__(self):
        super().__init__(ComponentType.HEALTH)

    def is_alive(self) -> bool:
        return self.current > 0

    def damage(self, amount: int):
        """Apply damage to health"""
        self.current = max(0, self.current - amount)

    def heal(self, amount: int):
        """Restore health"""
        self.current = min(self.maximum, self.current + amount)

    def to_dict(self) -> Dict[str, Any]:
        base = super().to_dict()
        base.update({
            "current": self.current,
            "maximum": self.maximum
        })
        return base

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'HealthComponent':
        return cls(
            current=data.get("current", 100),
            maximum=data.get("maximum", 100)
        )


@dataclass
class InventoryComponent(Component):
    """Component for storing items and currency"""
    items: Dict[str, int] = field(default_factory=dict)  # item_type -> quantity
    capacity: int = 20

    def __post_init__(self):
        super().__init__(ComponentType.INVENTORY)

    def add_item(self, item_type: str, quantity: int = 1) -> bool:
        """Add items to inventory. Returns True if successful."""
        current_count = sum(self.items.values())
        if current_count + quantity <= self.capacity:
            self.items[item_type] = self.items.get(item_type, 0) + quantity
            return True
        return False

    def remove_item(self, item_type: str, quantity: int = 1) -> bool:
        """Remove items from inventory. Returns True if successful."""
        if item_type in self.items and self.items[item_type] >= quantity:
            self.items[item_type] -= quantity
            if self.items[item_type] == 0:
                del self.items[item_type]
            return True
        return False

    def has_item(self, item_type: str, quantity: int = 1) -> bool:
        """Check if inventory contains specified quantity of item"""
        return self.items.get(item_type, 0) >= quantity

    def get_count(self, item_type: str) -> int:
        """Get count of specific item"""
        return self.items.get(item_type, 0)

    def to_dict(self) -> Dict[str, Any]:
        base = super().to_dict()
        base.update({
            "items": self.items,
            "capacity": self.capacity
        })
        return base

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'InventoryComponent':
        return cls(
            items=data.get("items", {}),
            capacity=data.get("capacity", 20)
        )


@dataclass
class CurrencyComponent(Component):
    """Component for tracking money/gold"""
    amount: int = 0

    def __post_init__(self):
        super().__init__(ComponentType.CURRENCY)

    def add(self, value: int):
        """Add currency"""
        self.amount += value

    def remove(self, value: int) -> bool:
        """Remove currency. Returns True if successful."""
        if self.amount >= value:
            self.amount -= value
            return True
        return False

    def has(self, value: int) -> bool:
        """Check if has enough currency"""
        return self.amount >= value

    def to_dict(self) -> Dict[str, Any]:
        base = super().to_dict()
        base.update({"amount": self.amount})
        return base

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'CurrencyComponent':
        return cls(amount=data.get("amount", 0))


@dataclass
class RelationshipComponent(Component):
    """Component for tracking relationships with other entities"""
    relationships: Dict[int, int] = field(default_factory=dict)  # entity_id -> value (-100 to +100)

    def __post_init__(self):
        super().__init__(ComponentType.RELATIONSHIP)

    def get_relationship(self, entity_id: int) -> int:
        """Get relationship value with entity (-100 to +100, 0 = neutral)"""
        return self.relationships.get(entity_id, 0)

    def modify_relationship(self, entity_id: int, delta: int):
        """Modify relationship value"""
        current = self.relationships.get(entity_id, 0)
        self.relationships[entity_id] = max(-100, min(100, current + delta))

    def set_relationship(self, entity_id: int, value: int):
        """Set relationship value directly"""
        self.relationships[entity_id] = max(-100, min(100, value))

    def get_relationship_level(self, entity_id: int) -> str:
        """Get text description of relationship"""
        value = self.get_relationship(entity_id)
        if value < -50:
            return "enemy"
        elif value < -10:
            return "dislike"
        elif value < 10:
            return "neutral"
        elif value < 50:
            return "friendly"
        elif value < 75:
            return "friend"
        else:
            return "close_friend"

    def to_dict(self) -> Dict[str, Any]:
        base = super().to_dict()
        base.update({"relationships": self.relationships})
        return base

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'RelationshipComponent':
        # Convert string keys back to integers (JSON serialization converts int keys to strings)
        relationships = data.get("relationships", {})
        int_relationships = {int(k): v for k, v in relationships.items()}
        return cls(relationships=int_relationships)


@dataclass
class NeedsComponent(Component):
    """Component for tracking villager needs (hunger, energy, social, etc.)"""
    hunger: float = 50.0  # 0 = starving, 100 = full
    energy: float = 100.0  # 0 = exhausted, 100 = fully rested
    social: float = 50.0  # 0 = lonely, 100 = socially fulfilled

    def __post_init__(self):
        super().__init__(ComponentType.NEEDS)

    def decay(self, delta_time: float):
        """Decay needs over time"""
        # Hunger increases (value decreases) over time
        self.hunger = max(0.0, self.hunger - (delta_time * 5.0))
        # Energy decreases over time
        self.energy = max(0.0, self.energy - (delta_time * 3.0))
        # Social decreases over time
        self.social = max(0.0, self.social - (delta_time * 2.0))

    def eat(self, food_value: float):
        """Increase hunger satisfaction"""
        self.hunger = min(100.0, self.hunger + food_value)

    def rest(self, rest_value: float):
        """Increase energy"""
        self.energy = min(100.0, self.energy + rest_value)

    def socialize(self, social_value: float):
        """Increase social satisfaction"""
        self.social = min(100.0, self.social + social_value)

    def get_most_urgent_need(self) -> str:
        """Return the name of the most urgent need"""
        needs = {
            "hunger": 100.0 - self.hunger,  # Invert so higher = more urgent
            "energy": 100.0 - self.energy,
            "social": 100.0 - self.social
        }
        return max(needs, key=needs.get)

    def to_dict(self) -> Dict[str, Any]:
        base = super().to_dict()
        base.update({
            "hunger": self.hunger,
            "energy": self.energy,
            "social": self.social
        })
        return base

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'NeedsComponent':
        return cls(
            hunger=data.get("hunger", 50.0),
            energy=data.get("energy", 100.0),
            social=data.get("social", 50.0)
        )


@dataclass
class ScheduleComponent(Component):
    """Component defining daily schedule for villagers"""
    schedule: Dict[str, str] = field(default_factory=dict)  # time_of_day -> activity

    def __post_init__(self):
        super().__init__(ComponentType.SCHEDULE)

    def get_activity(self, time_of_day: str) -> Optional[str]:
        """Get scheduled activity for time of day"""
        return self.schedule.get(time_of_day)

    def set_activity(self, time_of_day: str, activity: str):
        """Set activity for time of day"""
        self.schedule[time_of_day] = activity

    def to_dict(self) -> Dict[str, Any]:
        base = super().to_dict()
        base.update({"schedule": self.schedule})
        return base

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'ScheduleComponent':
        return cls(schedule=data.get("schedule", {}))


@dataclass
class OccupationComponent(Component):
    """Component defining villager occupation and workplace"""
    occupation: str = "Villager"
    workplace: str = "None"
    skill_level: int = 1

    def __post_init__(self):
        super().__init__(ComponentType.OCCUPATION)

    def to_dict(self) -> Dict[str, Any]:
        base = super().to_dict()
        base.update({
            "occupation": self.occupation,
            "workplace": self.workplace,
            "skill_level": self.skill_level
        })
        return base

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'OccupationComponent':
        return cls(
            occupation=data.get("occupation", "Villager"),
            workplace=data.get("workplace", "None"),
            skill_level=data.get("skill_level", 1)
        )


@dataclass
class MemoryComponent(Component):
    """Component for storing NPC memories of events"""
    memories: List[Dict[str, Any]] = field(default_factory=list)
    max_memories: int = 50

    def __post_init__(self):
        super().__init__(ComponentType.MEMORY)

    def add_memory(self, memory: Dict[str, Any]):
        """Add a memory, removing oldest if at capacity"""
        self.memories.append(memory)
        if len(self.memories) > self.max_memories:
            self.memories.pop(0)  # Remove oldest

    def get_recent_memories(self, count: int = 5) -> List[Dict[str, Any]]:
        """Get most recent memories"""
        return self.memories[-count:]

    def to_dict(self) -> Dict[str, Any]:
        base = super().to_dict()
        base.update({
            "memories": self.memories,
            "max_memories": self.max_memories
        })
        return base

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'MemoryComponent':
        return cls(
            memories=data.get("memories", []),
            max_memories=data.get("max_memories", 50)
        )


@dataclass
class GoalComponent(Component):
    """Component for tracking NPC goals and motivations"""
    current_goal: Optional[str] = None
    goals: List[str] = field(default_factory=list)

    def __post_init__(self):
        super().__init__(ComponentType.GOAL)

    def set_goal(self, goal: str):
        """Set current goal"""
        self.current_goal = goal

    def add_goal(self, goal: str):
        """Add goal to list"""
        if goal not in self.goals:
            self.goals.append(goal)

    def complete_goal(self, goal: str):
        """Remove completed goal"""
        if goal in self.goals:
            self.goals.remove(goal)
        if self.current_goal == goal:
            self.current_goal = None

    def to_dict(self) -> Dict[str, Any]:
        base = super().to_dict()
        base.update({
            "current_goal": self.current_goal,
            "goals": self.goals
        })
        return base

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'GoalComponent':
        return cls(
            current_goal=data.get("current_goal"),
            goals=data.get("goals", [])
        )


# Component Factory for deserialization
COMPONENT_REGISTRY = {
    ComponentType.POSITION.value: PositionComponent,
    ComponentType.HEALTH.value: HealthComponent,
    ComponentType.INVENTORY.value: InventoryComponent,
    ComponentType.CURRENCY.value: CurrencyComponent,
    ComponentType.RELATIONSHIP.value: RelationshipComponent,
    ComponentType.NEEDS.value: NeedsComponent,
    ComponentType.SCHEDULE.value: ScheduleComponent,
    ComponentType.OCCUPATION.value: OccupationComponent,
    ComponentType.MEMORY.value: MemoryComponent,
    ComponentType.GOAL.value: GoalComponent,
}


def create_component_from_dict(data: Dict[str, Any]) -> Optional[Component]:
    """Factory function to create component from dictionary"""
    component_type = data.get("type")
    if component_type in COMPONENT_REGISTRY:
        return COMPONENT_REGISTRY[component_type].from_dict(data)
    return None
