"""
Game State System for Transparent Game Engine
Central state object holding all game data with save/load capability.
"""

from typing import Dict, Any, Optional
from datetime import datetime
import json
from pathlib import Path

from src.core.entity import EntityManager


class TimeOfDay:
    """Enumeration of time periods in a day"""
    MORNING = "morning"
    AFTERNOON = "afternoon"
    EVENING = "evening"
    NIGHT = "night"

    @staticmethod
    def get_all():
        return [TimeOfDay.MORNING, TimeOfDay.AFTERNOON, TimeOfDay.EVENING, TimeOfDay.NIGHT]

    @staticmethod
    def get_next(current: str) -> str:
        """Get the next time of day"""
        times = TimeOfDay.get_all()
        try:
            index = times.index(current)
            return times[(index + 1) % len(times)]
        except ValueError:
            return TimeOfDay.MORNING


class Season:
    """Enumeration of seasons"""
    SPRING = "spring"
    SUMMER = "summer"
    FALL = "fall"
    WINTER = "winter"

    @staticmethod
    def get_all():
        return [Season.SPRING, Season.SUMMER, Season.FALL, Season.WINTER]

    @staticmethod
    def get_next(current: str) -> str:
        """Get the next season"""
        seasons = Season.get_all()
        try:
            index = seasons.index(current)
            return seasons[(index + 1) % len(seasons)]
        except ValueError:
            return Season.SPRING


class Weather:
    """Enumeration of weather types"""
    SUNNY = "sunny"
    RAINY = "rainy"
    CLOUDY = "cloudy"
    STORMY = "stormy"
    DROUGHT = "drought"

    @staticmethod
    def get_all():
        return [Weather.SUNNY, Weather.RAINY, Weather.CLOUDY, Weather.STORMY, Weather.DROUGHT]


class GameState:
    """
    Central game state holding all game data.
    Manages time, weather, entities, and provides serialization.
    """

    def __init__(self):
        # Entity Management
        self.entity_manager = EntityManager()

        # Time Management
        self.day_count = 1
        self.time_of_day = TimeOfDay.MORNING
        self.season = Season.SPRING
        self.year = 1

        # Weather
        self.current_weather = Weather.SUNNY

        # Player reference (set when player entity is created)
        self.player_id: Optional[int] = None

        # Game metadata
        self.game_name = "Farming Village"
        self.created_at = datetime.now().isoformat()
        self.last_saved = None

    def advance_time(self):
        """
        Advance time to next period of day.
        Handles day/season/year transitions.
        """
        self.time_of_day = TimeOfDay.get_next(self.time_of_day)

        # If we wrapped around to morning, advance day
        if self.time_of_day == TimeOfDay.MORNING:
            self.day_count += 1

            # Every 28 days, advance season
            if self.day_count % 28 == 0:
                self.season = Season.get_next(self.season)

                # If we wrapped to spring, advance year
                if self.season == Season.SPRING:
                    self.year += 1

    def get_current_time_description(self) -> str:
        """Get a human-readable description of current time."""
        return f"Year {self.year}, {self.season.capitalize()}, Day {self.day_count % 28 or 28}, {self.time_of_day.capitalize()}"

    def set_player(self, entity_id: int):
        """Set the player entity ID."""
        if self.entity_manager.get_entity(entity_id):
            self.player_id = entity_id

    def get_player(self):
        """Get the player entity."""
        if self.player_id is not None:
            return self.entity_manager.get_entity(self.player_id)
        return None

    def to_dict(self) -> Dict[str, Any]:
        """Serialize entire game state to dictionary."""
        return {
            "metadata": {
                "game_name": self.game_name,
                "created_at": self.created_at,
                "last_saved": datetime.now().isoformat(),
                "version": "1.0"
            },
            "time": {
                "day_count": self.day_count,
                "time_of_day": self.time_of_day,
                "season": self.season,
                "year": self.year
            },
            "weather": {
                "current_weather": self.current_weather
            },
            "player": {
                "player_id": self.player_id
            },
            "entities": self.entity_manager.to_dict()
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'GameState':
        """Deserialize game state from dictionary."""
        state = cls()

        # Restore metadata
        metadata = data.get("metadata", {})
        state.game_name = metadata.get("game_name", "Farming Village")
        state.created_at = metadata.get("created_at", datetime.now().isoformat())
        state.last_saved = metadata.get("last_saved")

        # Restore time
        time_data = data.get("time", {})
        state.day_count = time_data.get("day_count", 1)
        state.time_of_day = time_data.get("time_of_day", TimeOfDay.MORNING)
        state.season = time_data.get("season", Season.SPRING)
        state.year = time_data.get("year", 1)

        # Restore weather
        weather_data = data.get("weather", {})
        state.current_weather = weather_data.get("current_weather", Weather.SUNNY)

        # Restore player
        player_data = data.get("player", {})
        state.player_id = player_data.get("player_id")

        # Restore entities
        entities_data = data.get("entities", {})
        state.entity_manager = EntityManager.from_dict(entities_data)

        return state

    def save_to_file(self, filepath: str):
        """Save game state to JSON file."""
        path = Path(filepath)
        path.parent.mkdir(parents=True, exist_ok=True)

        data = self.to_dict()
        with open(filepath, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)

        self.last_saved = data["metadata"]["last_saved"]
        print(f"Game saved to {filepath}")

    @classmethod
    def load_from_file(cls, filepath: str) -> 'GameState':
        """Load game state from JSON file."""
        with open(filepath, 'r', encoding='utf-8') as f:
            data = json.load(f)

        state = cls.from_dict(data)
        print(f"Game loaded from {filepath}")
        return state

    def get_statistics(self) -> Dict[str, Any]:
        """Get current game statistics for display."""
        return {
            "time": self.get_current_time_description(),
            "weather": self.current_weather.capitalize(),
            "total_entities": self.entity_manager.count_entities(),
            "villagers": self.entity_manager.count_entities_by_type("Villager"),
            "crops": self.entity_manager.count_entities_by_type("Crop"),
            "player_id": self.player_id
        }

    def __repr__(self):
        stats = self.get_statistics()
        return f"GameState({stats})"
