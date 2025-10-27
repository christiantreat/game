# Transparent Text-Based Adventure Engine - Farming Village Edition

A bottom-up, layered game engine written in **pure C** designed for complete AI transparency. This project builds a peaceful farming simulation with a village economy and social systems where all AI decision-making is fully inspectable and logged.

## Philosophy

- **Transparency First**: Every AI decision is logged with full reasoning chains
- **Bottom-Up Development**: Build foundation first, add complexity incrementally
- **Data-Driven Design**: Game content defined in external files for easy modification
- **Peaceful Focus**: Cooperative gameplay emphasizing farming, trading, and relationships
- **2D-Ready Architecture**: Game logic separated from presentation layer for future graphical upgrade
- **Pure C Implementation**: Performance-focused with manual memory management and ECS architecture

## Project Status

### ✅ Phase 1: Core Foundation (COMPLETE)

**Goal**: Establish basic data structures and game state management

**Implemented**:
- ✅ **Component System** (pure C with 10 component types)
  - Position, Health, Inventory, Currency
  - Relationship, Needs, Schedule, Occupation
  - Memory, Goal
  - Full serialization to/from JSON

- ✅ **Entity System** (Entity-Component-System architecture)
  - Entity creation and lifecycle management
  - Component attachment/removal
  - Entity queries by component type
  - Helper functions for common archetypes

- ✅ **Game State Management**
  - Time system (day/night, seasons, years)
  - Weather tracking
  - Complete JSON serialization using cJSON
  - Save/load functionality with data integrity

**Success Criteria Met**:
- ✓ Can create entities with components
- ✓ Can maintain game state
- ✓ Serialization to JSON verified
- ✓ Load from JSON with data integrity preserved
- ✓ All tests passing (16/16)

**Files Created**:
```
src/core/
  component.h          - Component system header (10 types)
  component.c          - Component implementation (~1000 lines)
  entity.h             - Entity system header
  entity.c             - Entity management (~400 lines)
  game_state.h         - Game state header
  game_state.c         - Time/weather/serialization (~400 lines)
lib/
  cJSON.h/cJSON.c      - JSON parsing library
tests/
  test_phase1.c        - Comprehensive test suite
Makefile               - Build system
```

## Building and Running

### Prerequisites

- GCC compiler with C11 support
- Make
- Standard C library

### Build

```bash
# Build all
make

# Build and run tests
make test

# Clean and rebuild
make rebuild
```

### Expected Output

```
🎉 Phase 1: Core Foundation - COMPLETE! 🎉

Success Criteria Met:
✓ Can create entities with components
✓ Can maintain game state
✓ Can serialize to JSON
✓ Can load back from JSON
✓ Data integrity verified
```

## Architecture

### Entity-Component-System (ECS)

Pure C implementation of ECS architecture with manual memory management and performance focus.

## References

The engine architecture is informed by:
- **Game Engine Architecture** (3rd Edition) by Jason Gregory
- **Game Programming Patterns** by Robert Nystrom

See `engine_architecture.md` and `programming_patterns.md` for detailed reference material.

---

**Language**: Pure C (C11)
**Current Version**: Phase 1 Complete
**Test Coverage**: 16/16 passing
**Next Phase**: Event System Implementation
