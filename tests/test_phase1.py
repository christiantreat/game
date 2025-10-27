"""
Phase 1 Tests: Core Foundation
Tests for Component System, Entity System, and Game State
"""

import sys
import os
import json
import tempfile

# Add src to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from src.core import (
    # Components
    ComponentType,
    PositionComponent,
    HealthComponent,
    InventoryComponent,
    CurrencyComponent,
    RelationshipComponent,
    NeedsComponent,
    ScheduleComponent,
    OccupationComponent,
    # Entities
    Entity,
    EntityManager,
    create_player_entity,
    create_villager_entity,
    create_crop_entity,
    # Game State
    GameState,
    TimeOfDay,
    Season,
    Weather,
)


def test_component_creation():
    """Test 1.1: Create components and verify data storage"""
    print("\n=== Test 1.1: Component Creation ===")

    # Test Position Component
    pos = PositionComponent(location="VillageSquare", x=10.5, y=20.3)
    assert pos.location == "VillageSquare"
    assert pos.x == 10.5
    assert pos.y == 20.3
    print("✓ PositionComponent works")

    # Test Health Component
    health = HealthComponent(current=80, maximum=100)
    assert health.is_alive()
    health.damage(30)
    assert health.current == 50
    health.heal(20)
    assert health.current == 70
    print("✓ HealthComponent works")

    # Test Inventory Component
    inv = InventoryComponent(capacity=10)
    assert inv.add_item("wheat", 5)
    assert inv.has_item("wheat", 5)
    assert inv.get_count("wheat") == 5
    assert inv.remove_item("wheat", 2)
    assert inv.get_count("wheat") == 3
    print("✓ InventoryComponent works")

    # Test Currency Component
    currency = CurrencyComponent(amount=100)
    assert currency.has(50)
    assert currency.remove(30)
    assert currency.amount == 70
    currency.add(50)
    assert currency.amount == 120
    print("✓ CurrencyComponent works")

    # Test Relationship Component
    rel = RelationshipComponent()
    rel.set_relationship(entity_id=1, value=60)
    assert rel.get_relationship(1) == 60
    assert rel.get_relationship_level(1) == "friend"
    rel.modify_relationship(1, 20)
    assert rel.get_relationship(1) == 80
    assert rel.get_relationship_level(1) == "close_friend"
    print("✓ RelationshipComponent works")

    # Test Needs Component
    needs = NeedsComponent()
    needs.hunger = 30.0  # Low hunger (hungry)
    needs.energy = 20.0  # Low energy (tired)
    assert needs.get_most_urgent_need() == "energy"
    needs.eat(40)
    assert needs.hunger == 70.0
    print("✓ NeedsComponent works")

    # Test Schedule Component
    schedule = ScheduleComponent()
    schedule.set_activity("morning", "work")
    schedule.set_activity("evening", "socialize")
    assert schedule.get_activity("morning") == "work"
    assert schedule.get_activity("evening") == "socialize"
    print("✓ ScheduleComponent works")

    # Test Occupation Component
    occ = OccupationComponent(occupation="Blacksmith", workplace="Forge", skill_level=5)
    assert occ.occupation == "Blacksmith"
    assert occ.workplace == "Forge"
    assert occ.skill_level == 5
    print("✓ OccupationComponent works")

    print("\n✅ All component tests passed!")
    return True


def test_entity_system():
    """Test 1.2: Entity creation, component attachment/removal, entity lookup"""
    print("\n=== Test 1.2: Entity System ===")

    manager = EntityManager()

    # Create player entity
    player = create_player_entity(manager, "TestPlayer")
    assert player.name == "TestPlayer"
    assert player.entity_type == "Player"
    assert player.has_component(ComponentType.POSITION)
    assert player.has_component(ComponentType.HEALTH)
    assert player.has_component(ComponentType.INVENTORY)
    print(f"✓ Created player: {player}")

    # Create villager entity
    villager = create_villager_entity(manager, "Marcus", "Merchant")
    assert villager.name == "Marcus"
    assert villager.has_component(ComponentType.OCCUPATION)
    occ = villager.get_component(ComponentType.OCCUPATION)
    assert occ.occupation == "Merchant"
    print(f"✓ Created villager: {villager}")

    # Create crop entity
    crop = create_crop_entity(manager, "Wheat", "YourFarm", x=5.0, y=3.0)
    assert crop.name == "Wheat"
    assert crop.entity_type == "Crop"
    pos = crop.get_component(ComponentType.POSITION)
    assert pos.location == "YourFarm"
    assert pos.x == 5.0
    print(f"✓ Created crop: {crop}")

    # Test entity lookup
    assert manager.count_entities() == 3
    assert manager.get_entity(player.id) == player
    print(f"✓ Entity lookup works, total entities: {manager.count_entities()}")

    # Test query by component
    entities_with_position = manager.query_entities(ComponentType.POSITION)
    assert len(entities_with_position) == 3  # All have position
    print(f"✓ Query by component works: found {len(entities_with_position)} entities with position")

    entities_with_needs = manager.query_entities(ComponentType.NEEDS)
    assert len(entities_with_needs) == 1  # Only villager
    print(f"✓ Query found {len(entities_with_needs)} entities with needs (villager only)")

    # Test component removal
    villager.remove_component(ComponentType.SCHEDULE)
    assert not villager.has_component(ComponentType.SCHEDULE)
    print("✓ Component removal works")

    # Test entity removal
    manager.remove_entity(crop.id)
    assert manager.count_entities() == 2
    assert manager.get_entity(crop.id) is None
    print("✓ Entity removal works")

    print("\n✅ All entity system tests passed!")
    return True


def test_game_state():
    """Test 1.3: Game state creation, serialization, and loading"""
    print("\n=== Test 1.3: Game State System ===")

    # Create game state
    state = GameState()
    assert state.day_count == 1
    assert state.season == Season.SPRING
    assert state.time_of_day == TimeOfDay.MORNING
    print("✓ GameState initialized")

    # Create entities
    player = create_player_entity(state.entity_manager, "Hero")
    state.set_player(player.id)
    assert state.get_player() == player
    print(f"✓ Player set: {player.name}")

    villager1 = create_villager_entity(state.entity_manager, "Sarah", "Baker")
    villager2 = create_villager_entity(state.entity_manager, "Tom", "Farmer")
    print(f"✓ Created 2 villagers")

    # Modify some state
    inv = player.get_component(ComponentType.INVENTORY)
    inv.add_item("wheat", 10)
    inv.add_item("corn", 5)

    currency = player.get_component(ComponentType.CURRENCY)
    currency.add(250)

    # Set relationships
    rel = player.get_component(ComponentType.RELATIONSHIP)
    rel.set_relationship(villager1.id, 60)  # Friend
    rel.set_relationship(villager2.id, 30)  # Friendly
    print("✓ Modified player state")

    # Advance time
    initial_time = state.get_current_time_description()
    state.advance_time()
    assert state.time_of_day == TimeOfDay.AFTERNOON
    state.advance_time()
    state.advance_time()
    state.advance_time()  # Should wrap to next day
    assert state.day_count == 2
    assert state.time_of_day == TimeOfDay.MORNING
    print(f"✓ Time advancement works: {initial_time} -> {state.get_current_time_description()}")

    # Test statistics
    stats = state.get_statistics()
    print(f"✓ Statistics: {stats}")
    assert stats["total_entities"] == 3
    assert stats["villagers"] == 2

    # Test serialization
    print("\n--- Testing Serialization ---")
    state_dict = state.to_dict()
    assert "metadata" in state_dict
    assert "entities" in state_dict
    assert "time" in state_dict
    print("✓ to_dict() works")

    # Save to file
    with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as f:
        temp_file = f.name

    try:
        state.save_to_file(temp_file)
        print(f"✓ Saved to {temp_file}")

        # Load from file
        loaded_state = GameState.load_from_file(temp_file)
        print("✓ Loaded from file")

        # Verify loaded state
        assert loaded_state.day_count == state.day_count
        assert loaded_state.season == state.season
        assert loaded_state.entity_manager.count_entities() == 3
        print("✓ Loaded state matches original")

        # Verify player data
        loaded_player = loaded_state.get_player()
        assert loaded_player is not None
        assert loaded_player.name == "Hero"

        loaded_inv = loaded_player.get_component(ComponentType.INVENTORY)
        assert loaded_inv.get_count("wheat") == 10
        assert loaded_inv.get_count("corn") == 5

        loaded_currency = loaded_player.get_component(ComponentType.CURRENCY)
        assert loaded_currency.amount == 350  # 100 + 250

        loaded_rel = loaded_player.get_component(ComponentType.RELATIONSHIP)
        assert loaded_rel.get_relationship(villager1.id) == 60
        print("✓ Player data integrity verified")

        # Verify villagers
        villagers = loaded_state.entity_manager.get_entities_by_type("Villager")
        assert len(villagers) == 2
        baker = next(v for v in villagers if v.name == "Sarah")
        baker_occ = baker.get_component(ComponentType.OCCUPATION)
        assert baker_occ.occupation == "Baker"
        print("✓ Villager data integrity verified")

    finally:
        # Cleanup
        os.unlink(temp_file)

    print("\n✅ All game state tests passed!")
    return True


def run_all_tests():
    """Run all Phase 1 tests"""
    print("=" * 60)
    print("PHASE 1 TESTS: Core Foundation")
    print("Testing Component System, Entity System, and Game State")
    print("=" * 60)

    tests = [
        ("Component Creation", test_component_creation),
        ("Entity System", test_entity_system),
        ("Game State & Serialization", test_game_state),
    ]

    passed = 0
    failed = 0

    for test_name, test_func in tests:
        try:
            if test_func():
                passed += 1
        except Exception as e:
            print(f"\n❌ Test '{test_name}' failed with error:")
            print(f"   {type(e).__name__}: {e}")
            import traceback
            traceback.print_exc()
            failed += 1

    print("\n" + "=" * 60)
    print(f"RESULTS: {passed} passed, {failed} failed")
    print("=" * 60)

    if failed == 0:
        print("\n🎉 Phase 1: Core Foundation - COMPLETE! 🎉")
        print("\nSuccess Criteria Met:")
        print("✓ Can create entities with components")
        print("✓ Can maintain game state")
        print("✓ Can serialize to JSON")
        print("✓ Can load back from JSON")
        print("✓ Data integrity verified")
        return True
    else:
        print("\n⚠️  Some tests failed. Please review errors above.")
        return False


if __name__ == "__main__":
    success = run_all_tests()
    sys.exit(0 if success else 1)
