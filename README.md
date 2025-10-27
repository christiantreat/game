# Transparent Text-Based Adventure Engine - Farming Village Edition

A bottom-up, layered game engine designed for complete AI transparency. This project builds a peaceful farming simulation with a village economy and social systems where all AI decision-making is fully inspectable and logged.

## Philosophy

- **Transparency First**: Every AI decision is logged with full reasoning chains
- **Bottom-Up Development**: Build foundation first, add complexity incrementally
- **Data-Driven Design**: Game content defined in external files for easy modification
- **Peaceful Focus**: Cooperative gameplay emphasizing farming, trading, and relationships
- **2D-Ready Architecture**: Game logic separated from presentation layer for future graphical upgrade

## Project Status

### ✅ Phase 1: Core Foundation (COMPLETE)

**Goal**: Establish basic data structures and game state management

**Implemented**:
- ✅ Component System with 10 component types
  - Position, Health, Inventory, Currency
  - Relationship, Needs, Schedule, Occupation
  - Memory, Goal
- ✅ Entity System with component management
  - Entity creation and lifecycle
  - Component attachment/removal
  - Entity queries by component type
- ✅ Game State Management
  - Time system (day/night, seasons, years)
  - Weather tracking
  - Complete JSON serialization
  - Save/load functionality with data integrity

**Success Criteria Met**:
- ✓ Can create entities with components
- ✓ Can maintain game state
- ✓ Serialization to JSON verified
- ✓ Load from JSON with data integrity preserved

**Files Created**:
```
src/
  core/
    component.py       - Component system with 10 component types
    entity.py          - Entity system and manager
    game_state.py      - Central game state with time/weather
tests/
  test_phase1.py       - Comprehensive Phase 1 test suite
```

### 🔜 Upcoming Phases

- **Phase 2**: Event System (message-passing for transparency)
- **Phase 3**: Decision System Foundation
- **Phase 4**: Behavior Tree Implementation
- **Phase 5**: Game World & Locations
- **Phase 6**: Time & Agriculture Systems
- **Phase 7**: Economy & Trading System
- **Phase 8**: Social Systems
- **Phase 9**: Game Loop & Turn Management
- **Phase 10**: Transparency UI
- **Phase 11**: Data-Driven Content
- **Phase 12**: Farming Village Demo
- **Phase 13**: Polish & Documentation

## Architecture

### Component System

The engine uses an Entity-Component-System (ECS) architecture:

```python
# Create entities
player = create_player_entity(entity_manager, "Hero")

# Components are data containers
health = HealthComponent(current=100, maximum=100)
inventory = InventoryComponent(capacity=20)

# Attach to entities
player.add_component(health)
player.add_component(inventory)

# Query entities by components
entities_with_health = entity_manager.query_entities(ComponentType.HEALTH)
```

### Game State

Central state object manages all game data:

```python
# Create and manage game state
state = GameState()
player = create_player_entity(state.entity_manager, "Hero")
state.set_player(player.id)

# Time advances naturally
state.advance_time()  # Morning -> Afternoon -> Evening -> Night -> Next Day

# Save/load with full data integrity
state.save_to_file("saves/game.json")
loaded_state = GameState.load_from_file("saves/game.json")
```

## Running Tests

```bash
# Run Phase 1 tests
python tests/test_phase1.py
```

Expected output:
```
============================================================
PHASE 1 TESTS: Core Foundation
Testing Component System, Entity System, and Game State
============================================================

=== Test 1.1: Component Creation ===
✓ All component types work

=== Test 1.2: Entity System ===
✓ Entity creation, component management, queries

=== Test 1.3: Game State System ===
✓ Time management, serialization, data integrity

============================================================
RESULTS: 3 passed, 0 failed
============================================================

🎉 Phase 1: Core Foundation - COMPLETE! 🎉
```

## Component Types

| Component | Purpose | Key Features |
|-----------|---------|--------------|
| Position | Entity location | Location name, x/y coordinates |
| Health | Vitality tracking | Current/max health, damage/heal |
| Inventory | Item storage | Item quantities, capacity limits |
| Currency | Money/gold | Add/remove/check funds |
| Relationship | Social bonds | Relationship values (-100 to +100) |
| Needs | Villager needs | Hunger, energy, social fulfillment |
| Schedule | Daily routines | Time-based activities |
| Occupation | Job/profession | Occupation, workplace, skill level |
| Memory | Event storage | Recent memories, event logs |
| Goal | Motivations | Current and pending goals |

## Development Principles

1. **Test Each Phase**: Don't move forward until current phase is solid
2. **Maintain Transparency**: Add logging and inspection at every step
3. **Data-Driven from Start**: Structure code for external data files
4. **Iterate Quickly**: Keep phases small and completable
5. **Document Decisions**: Note architectural choices for reference

## Future: Transparency Showcase Examples

The system will eventually show AI decisions like:

**Trade Decision**:
```
Merchant Marcus declined your wheat offer:
  Base price: 8 gold
  Your offer: 5 gold (-37.5%)
  Relationship bonus: +10% (50/100 relationship)
  Final threshold: 6.4 gold needed
  Verdict: REJECT - offer too low
Try again with higher price or improve relationship.
```

**Schedule Decision**:
```
Baker Sarah's morning routine:
  [Time: 6 AM, Energy: 85%, Hunger: 60%]
  → Behavior Tree: [Priority: HIGH_HUNGER (60% > 50%)]
  → [Action: EAT_BREAKFAST at home]
  → [Energy: 85% → 100%, Hunger: 60% → 20%]
  → [Next: OPEN_BAKERY at 7 AM]
```

## References

The engine architecture is informed by:
- **Game Engine Architecture** (3rd Edition) by Jason Gregory
- **Game Programming Patterns** by Robert Nystrom
- **C++ Game Development Cookbook**

See `engine_architecture.md` and `programming_patterns.md` for detailed reference material.

## License

This is a learning project for exploring transparent AI systems in games.

---

**Current Version**: Phase 1 Complete
**Last Updated**: 2025-10-27
**Next Phase**: Event System Implementation
