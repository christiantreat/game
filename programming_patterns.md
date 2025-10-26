# Game Programming Patterns - Condensed Guide

**Source:** Game Programming Patterns by Robert Nystrom

---

## Table of Contents

1. [Design Patterns Revisited](#design-patterns-revisited)
2. [Sequencing Patterns](#sequencing-patterns)
3. [Behavioral Patterns](#behavioral-patterns)
4. [Decoupling Patterns](#decoupling-patterns)
5. [Optimization Patterns](#optimization-patterns)

---

## Design Patterns Revisited

### 1. Command Pattern

**Intent:** Encapsulate a request as an object to enable undo/redo, input mapping, and AI recording.

**Key Concepts:**
- Turns actions into first-class objects
- Enables undo/redo functionality
- Allows configurable input handling
- Facilitates AI behavior recording and replay

**Scripting Pattern:**

```python
# Base Command Interface
class Command:
    def execute(self, actor):
        pass
    
    def undo(self):
        pass

# Concrete Commands
class JumpCommand(Command):
    def __init__(self):
        self.actor = None
        self.prev_y = 0
    
    def execute(self, actor):
        self.actor = actor
        self.prev_y = actor.y
        actor.jump()
    
    def undo(self):
        if self.actor:
            self.actor.y = self.prev_y

class FireCommand(Command):
    def execute(self, actor):
        actor.fire_weapon()

# Input Handler
class InputHandler:
    def __init__(self):
        self.button_x = JumpCommand()
        self.button_y = FireCommand()
    
    def handle_input(self):
        if is_pressed(BUTTON_X):
            return self.button_x
        if is_pressed(BUTTON_Y):
            return self.button_y
        return None

# Usage with Undo
command_history = []
command = input_handler.handle_input()
if command:
    command.execute(player)
    command_history.append(command)

# Undo last action
if command_history:
    last_command = command_history.pop()
    last_command.undo()
```

---

### 2. Flyweight Pattern

**Intent:** Share data efficiently across many similar objects to reduce memory usage.

**Key Concepts:**
- Separates intrinsic (shared) from extrinsic (unique) state
- Reduces memory footprint for large numbers of similar objects
- Common in rendering (shared meshes/textures)
- Ideal for particle systems and terrain tiles

**Scripting Pattern:**

```python
# Shared Flyweight Data
class TreeModel:
    """Intrinsic state - shared across all trees"""
    def __init__(self, mesh, texture, bark_texture):
        self.mesh = mesh
        self.texture = texture
        self.bark_texture = bark_texture
        self.poly_count = len(mesh)

# Context-Specific Data
class Tree:
    """Extrinsic state - unique per tree"""
    def __init__(self, model, position, height, thickness, color):
        self.model = model  # Reference to shared flyweight
        self.position = position
        self.height = height
        self.thickness = thickness
        self.color = color
    
    def render(self):
        # Use shared model data with unique parameters
        render_mesh(self.model.mesh, self.position, 
                   self.height, self.color)

# Flyweight Factory
class TreeFactory:
    def __init__(self):
        self.tree_models = {}
    
    def get_tree_model(self, mesh_id, texture_id):
        key = (mesh_id, texture_id)
        if key not in self.tree_models:
            # Load and cache the shared data
            self.tree_models[key] = TreeModel(
                load_mesh(mesh_id),
                load_texture(texture_id),
                load_bark_texture()
            )
        return self.tree_models[key]

# Create thousands of trees with minimal memory
factory = TreeFactory()
oak_model = factory.get_tree_model("oak", "oak_tex")

forest = []
for i in range(10000):
    tree = Tree(oak_model, random_position(), 
                random_height(), random_thickness(), random_color())
    forest.append(tree)
```

---

### 3. Observer Pattern

**Intent:** Define one-to-many dependency so when one object changes, dependents are notified automatically.

**Key Concepts:**
- Decouples event producers from consumers
- Supports multiple listeners
- Enables achievement systems, UI updates, audio triggers
- Be careful of performance and notification order

**Scripting Pattern:**

```python
# Subject/Observable
class Subject:
    def __init__(self):
        self.observers = []
    
    def add_observer(self, observer):
        self.observers.append(observer)
    
    def remove_observer(self, observer):
        self.observers.remove(observer)
    
    def notify(self, event, entity):
        for observer in self.observers:
            observer.on_notify(event, entity)

# Observer Interface
class Observer:
    def on_notify(self, event, entity):
        pass

# Concrete Observers
class AchievementObserver(Observer):
    def on_notify(self, event, entity):
        if event == "ENEMY_KILLED":
            self.unlock_achievement("first_blood")
        elif event == "FELL_IN_PIT":
            self.unlock_achievement("gravity_wins")

class AudioObserver(Observer):
    def on_notify(self, event, entity):
        if event == "ENEMY_KILLED":
            play_sound("enemy_death.wav")

# Entity with Observable Behavior
class Entity:
    def __init__(self):
        self.subject = Subject()
    
    def take_damage(self, amount):
        self.health -= amount
        if self.health <= 0:
            self.subject.notify("ENEMY_KILLED", self)

# Setup
player = Entity()
player.subject.add_observer(AchievementObserver())
player.subject.add_observer(AudioObserver())

# Event-Based Alternative (More Modern)
class EventBus:
    def __init__(self):
        self.listeners = {}
    
    def subscribe(self, event_type, callback):
        if event_type not in self.listeners:
            self.listeners[event_type] = []
        self.listeners[event_type].append(callback)
    
    def publish(self, event_type, data):
        if event_type in self.listeners:
            for callback in self.listeners[event_type]:
                callback(data)

event_bus = EventBus()
event_bus.subscribe("player_died", lambda data: show_game_over())
event_bus.subscribe("player_died", lambda data: play_death_sound())
event_bus.publish("player_died", {"player": player, "cause": "fall"})
```

---

### 4. Prototype Pattern

**Intent:** Create new objects by cloning existing instances rather than constructing from scratch.

**Key Concepts:**
- Useful for spawning pre-configured entities
- Enables data-driven object creation
- Supports object pooling and factory patterns
- Common in level editors and spawning systems

**Scripting Pattern:**

```python
import copy

# Prototype Interface
class Monster:
    def clone(self):
        return copy.deepcopy(self)
    
    def spawn(self, position):
        new_monster = self.clone()
        new_monster.position = position
        return new_monster

# Concrete Prototypes
class Ghost(Monster):
    def __init__(self):
        self.health = 50
        self.speed = 5
        self.color = "white"
        self.abilities = ["phase_through_walls"]

class Demon(Monster):
    def __init__(self):
        self.health = 200
        self.speed = 3
        self.color = "red"
        self.abilities = ["fireball", "teleport"]

# Spawner using Prototypes
class Spawner:
    def __init__(self, prototype):
        self.prototype = prototype
    
    def spawn_enemy(self, position):
        return self.prototype.spawn(position)

# Usage
ghost_spawner = Spawner(Ghost())
demon_spawner = Spawner(Demon())

enemies = []
enemies.append(ghost_spawner.spawn_enemy((10, 20)))
enemies.append(ghost_spawner.spawn_enemy((30, 40)))
enemies.append(demon_spawner.spawn_enemy((50, 60)))

# JSON-Based Prototype System
import json

class PrototypeFactory:
    def __init__(self):
        self.prototypes = {}
    
    def register(self, name, prototype):
        self.prototypes[name] = prototype
    
    def create(self, name, **overrides):
        if name in self.prototypes:
            obj = self.prototypes[name].clone()
            for key, value in overrides.items():
                setattr(obj, key, value)
            return obj
        return None

# Load from data file
factory = PrototypeFactory()
factory.register("ghost", Ghost())
factory.register("demon", Demon())

# Create variations
weak_ghost = factory.create("ghost", health=25, speed=3)
fast_demon = factory.create("demon", speed=6)
```

---

### 5. Singleton Pattern

**Intent:** Ensure a class has only one instance and provide global access to it.

**Key Concepts:**
- Global access point for systems (audio, input, logging)
- Can cause tight coupling and hidden dependencies
- Makes unit testing difficult
- **Use sparingly** - often indicates poor architecture
- Consider dependency injection or service locators instead

**Scripting Pattern:**

```python
# Traditional Singleton (Discouraged)
class AudioManager:
    _instance = None
    
    def __new__(cls):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._instance.initialized = False
        return cls._instance
    
    def __init__(self):
        if not self.initialized:
            self.sound_effects = {}
            self.music_volume = 1.0
            self.initialized = True
    
    def play_sound(self, sound_name):
        # Play sound implementation
        pass

# Usage
audio = AudioManager()
audio.play_sound("jump")

# Better Alternative: Service Locator
class ServiceLocator:
    _services = {}
    
    @classmethod
    def provide(cls, service_type, service_instance):
        cls._services[service_type] = service_instance
    
    @classmethod
    def get(cls, service_type):
        return cls._services.get(service_type)

# Register services at startup
class AudioService:
    def play_sound(self, sound_name):
        pass

ServiceLocator.provide("audio", AudioService())

# Access anywhere
audio = ServiceLocator.get("audio")
audio.play_sound("jump")

# Even Better: Dependency Injection
class GameEntity:
    def __init__(self, audio_service):
        self.audio = audio_service
    
    def jump(self):
        self.audio.play_sound("jump")

# Pass dependencies explicitly
audio_service = AudioService()
player = GameEntity(audio_service)
```

---

### 6. State Pattern

**Intent:** Allow an object to change behavior when internal state changes (FSM implementation).

**Key Concepts:**
- Implements finite state machines cleanly
- Each state is a separate class with defined transitions
- Eliminates large switch/if-else chains
- Common for character controllers, AI, game states

**Scripting Pattern:**

```python
# State Interface
class HeroineState:
    def handle_input(self, heroine, input):
        pass
    
    def update(self, heroine):
        pass
    
    def enter(self, heroine):
        pass
    
    def exit(self, heroine):
        pass

# Concrete States
class StandingState(HeroineState):
    def handle_input(self, heroine, input):
        if input == INPUT_PRESS_B:
            heroine.change_state(JumpingState())
        elif input == INPUT_PRESS_DOWN:
            heroine.change_state(DuckingState())

class JumpingState(HeroineState):
    def enter(self, heroine):
        heroine.set_graphics("jump")
        heroine.y_velocity = JUMP_VELOCITY
    
    def update(self, heroine):
        heroine.y += heroine.y_velocity
        heroine.y_velocity += GRAVITY
        
        if heroine.y <= 0:
            heroine.change_state(StandingState())
    
    def handle_input(self, heroine, input):
        if input == INPUT_PRESS_DOWN:
            heroine.change_state(DivingState())

class DuckingState(HeroineState):
    def __init__(self):
        self.charge_time = 0
    
    def enter(self, heroine):
        heroine.set_graphics("duck")
    
    def update(self, heroine):
        self.charge_time += 1
    
    def handle_input(self, heroine, input):
        if input == INPUT_RELEASE_DOWN:
            heroine.change_state(StandingState())

# Context (Heroine)
class Heroine:
    def __init__(self):
        self.state = StandingState()
        self.x = 0
        self.y = 0
        self.y_velocity = 0
    
    def change_state(self, new_state):
        self.state.exit(self)
        self.state = new_state
        self.state.enter(self)
    
    def handle_input(self, input):
        self.state.handle_input(self, input)
    
    def update(self):
        self.state.update(self)

# Hierarchical State Machine
class OnGroundState(HeroineState):
    def handle_input(self, heroine, input):
        if input == INPUT_PRESS_B:
            heroine.change_state(JumpingState())
        # Common ground behavior

class DuckingState(OnGroundState):
    def handle_input(self, heroine, input):
        super().handle_input(heroine, input)  # Check jump first
        if input == INPUT_RELEASE_DOWN:
            heroine.change_state(StandingState())
```

---

## Sequencing Patterns

### 7. Double Buffer Pattern

**Intent:** Make sequential operations appear instantaneous or simultaneous.

**Key Concepts:**
- Two buffers: one being modified, one being read
- Swaps buffers when modification complete
- Essential for rendering (back/front buffer)
- Prevents tearing and partial state visibility
- Used in graphics, audio, and state updates

**Scripting Pattern:**

```python
# Graphics Double Buffer
class Scene:
    def __init__(self, width, height):
        self.current = Buffer(width, height)
        self.next = Buffer(width, height)
    
    def draw(self):
        # Clear next buffer
        self.next.clear()
        
        # Draw everything to next buffer
        for actor in self.actors:
            actor.render(self.next)
        
        # Swap buffers atomically
        self.current, self.next = self.next, self.current
    
    def get_buffer(self):
        return self.current

# Actor State Double Buffer
class Actor:
    def __init__(self):
        self.current_state = ActorState()
        self.next_state = ActorState()
    
    def update(self):
        # Read from current, write to next
        self.next_state.x = self.current_state.x + self.velocity_x
        self.next_state.y = self.current_state.y + self.velocity_y
    
    def swap(self):
        # All actors swap simultaneously
        self.current_state, self.next_state = self.next_state, self.current_state

# Game Loop with Double Buffer
class Game:
    def __init__(self):
        self.actors = []
        self.scene = Scene(800, 600)
    
    def game_loop(self):
        while self.running:
            # Update phase - all actors write to next buffer
            for actor in self.actors:
                actor.update()
            
            # Swap phase - commit all changes simultaneously
            for actor in self.actors:
                actor.swap()
            
            # Render phase
            self.scene.draw()
            display(self.scene.get_buffer())

# Simplified Pattern for Game State
class GameState:
    def __init__(self):
        self.buffers = [[], []]
        self.current = 0
    
    def get_current(self):
        return self.buffers[self.current]
    
    def get_next(self):
        return self.buffers[1 - self.current]
    
    def swap(self):
        self.current = 1 - self.current
```

---

### 8. Game Loop Pattern

**Intent:** Decouple game time progression from user input and processor speed.

**Key Concepts:**
- Core architecture of every game
- Processes input, updates game state, renders
- Handles variable frame rates
- Fixed timestep for consistent physics
- Prevents simulation from running too fast/slow

**Scripting Pattern:**

```python
import time

# Basic Game Loop
class Game:
    def __init__(self):
        self.running = True
        self.last_time = time.time()
    
    def run(self):
        while self.running:
            # Process input
            self.process_input()
            
            # Update game state
            self.update()
            
            # Render
            self.render()

# Variable Time Step
class VariableTimeStepGame:
    def run(self):
        last_time = time.time()
        
        while self.running:
            current_time = time.time()
            delta_time = current_time - last_time
            last_time = current_time
            
            self.process_input()
            self.update(delta_time)
            self.render()

# Fixed Time Step (Physics)
class FixedTimeStepGame:
    def __init__(self):
        self.MS_PER_UPDATE = 1.0 / 60.0  # 60 FPS
        self.lag = 0.0
    
    def run(self):
        previous = time.time()
        
        while self.running:
            current = time.time()
            elapsed = current - previous
            previous = current
            self.lag += elapsed
            
            self.process_input()
            
            # Update in fixed increments
            while self.lag >= self.MS_PER_UPDATE:
                self.update()
                self.lag -= self.MS_PER_UPDATE
            
            # Render with interpolation
            self.render(self.lag / self.MS_PER_UPDATE)

# Frame Rate Limiting
class FrameLimitedGame:
    def __init__(self):
        self.TARGET_FPS = 60
        self.FRAME_TIME = 1.0 / self.TARGET_FPS
    
    def run(self):
        while self.running:
            frame_start = time.time()
            
            self.process_input()
            self.update()
            self.render()
            
            # Sleep to maintain frame rate
            frame_time = time.time() - frame_start
            if frame_time < self.FRAME_TIME:
                time.sleep(self.FRAME_TIME - frame_time)

# Advanced: Fixed Physics, Variable Rendering
class HybridGame:
    def __init__(self):
        self.PHYSICS_STEP = 1.0 / 60.0
        self.accumulator = 0.0
    
    def run(self):
        previous = time.time()
        
        while self.running:
            current = time.time()
            frame_time = current - previous
            previous = current
            
            # Limit maximum frame time (spiral of death protection)
            if frame_time > 0.25:
                frame_time = 0.25
            
            self.accumulator += frame_time
            
            # Fixed physics updates
            while self.accumulator >= self.PHYSICS_STEP:
                self.process_input()
                self.update_physics(self.PHYSICS_STEP)
                self.accumulator -= self.PHYSICS_STEP
            
            # Variable rendering with interpolation
            alpha = self.accumulator / self.PHYSICS_STEP
            self.render(alpha)
```

---

### 9. Update Method Pattern

**Intent:** Simulate independent objects by processing one frame of behavior at a time.

**Key Concepts:**
- Each object gets an `update()` method called per frame
- Maintains illusion of simultaneous execution
- Integrates with game loop
- Central to entity behavior

**Scripting Pattern:**

```python
# Basic Update Method
class Entity:
    def update(self, delta_time):
        """Called once per frame"""
        pass

class Skeleton(Entity):
    def __init__(self):
        self.patrol_distance = 0
    
    def update(self, delta_time):
        # AI behavior
        self.patrol_distance += delta_time * self.speed
        
        if self.patrol_distance > 100:
            self.reverse_direction()

# Update with State Management
class Bullet(Entity):
    def __init__(self, x, y, velocity_x, velocity_y):
        self.x = x
        self.y = y
        self.vx = velocity_x
        self.vy = velocity_y
        self.alive = True
    
    def update(self, delta_time):
        if not self.alive:
            return
        
        self.x += self.vx * delta_time
        self.y += self.vy * delta_time
        
        # Check boundaries
        if self.is_off_screen():
            self.alive = False

# Game World Managing Updates
class World:
    def __init__(self):
        self.entities = []
    
    def add_entity(self, entity):
        self.entities.append(entity)
    
    def update(self, delta_time):
        # Update all entities
        for entity in self.entities:
            entity.update(delta_time)
        
        # Remove dead entities
        self.entities = [e for e in self.entities if e.is_alive()]

# Variable Update Rates
class Entity:
    def __init__(self):
        self.update_interval = 0.016  # ~60 FPS
        self.time_since_update = 0.0
    
    def update(self, delta_time):
        self.time_since_update += delta_time
        
        if self.time_since_update >= self.update_interval:
            self.do_update()
            self.time_since_update = 0.0

# Priority-Based Updates
class EntityManager:
    def __init__(self):
        self.high_priority = []  # Player, nearby enemies
        self.low_priority = []   # Distant objects
    
    def update(self, delta_time):
        # Always update high priority
        for entity in self.high_priority:
            entity.update(delta_time)
        
        # Update low priority less frequently
        for entity in self.low_priority[::2]:  # Every other frame
            entity.update(delta_time * 2)

# Actor-Based Update Pattern
class Actor:
    def update(self, delta_time):
        # Default behavior - can be overridden
        pass

class Player(Actor):
    def update(self, delta_time):
        self.handle_input()
        self.move(delta_time)
        self.check_collisions()

class Enemy(Actor):
    def update(self, delta_time):
        self.run_ai()
        self.move(delta_time)
        self.check_player_distance()
```

---

## Behavioral Patterns

### 10. Bytecode Pattern

**Intent:** Define behavior as data in a virtual machine to enable flexibility and avoid recompilation.

**Key Concepts:**
- Behavior defined in simple instruction set
- Interpreted at runtime by VM
- Enables modding and data-driven design
- Used for AI, scripting, ability systems
- Trade flexibility for performance

**Scripting Pattern:**

```python
# Instruction Set
class Instruction:
    SET_HEALTH = 0x00
    SET_WISDOM = 0x01
    SET_AGILITY = 0x02
    PLAY_SOUND = 0x03
    SPAWN_PARTICLES = 0x04
    ADD = 0x05
    DIVIDE = 0x06

# Virtual Machine
class VM:
    def __init__(self):
        self.stack = []
    
    def interpret(self, bytecode):
        i = 0
        while i < len(bytecode):
            instruction = bytecode[i]
            
            if instruction == Instruction.SET_HEALTH:
                amount = bytecode[i + 1]
                self.set_health(amount)
                i += 2
            
            elif instruction == Instruction.PLAY_SOUND:
                sound_id = bytecode[i + 1]
                self.play_sound(sound_id)
                i += 2
            
            elif instruction == Instruction.SPAWN_PARTICLES:
                self.spawn_particles()
                i += 1
            
            elif instruction == Instruction.ADD:
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
                i += 1
            
            else:
                i += 1
    
    def push(self, value):
        self.stack.append(value)
    
    def pop(self):
        return self.stack.pop()

# Usage: Define Spell in Bytecode
fire_spell = [
    Instruction.PLAY_SOUND, 0x01,        # Play fire sound
    Instruction.SPAWN_PARTICLES,          # Spawn fire particles
    Instruction.SET_HEALTH, 0x64,        # Damage: 100
    Instruction.SET_AGILITY, 0x0A        # Slow: 10
]

vm = VM()
vm.interpret(fire_spell)

# Stack-Based Expression Evaluation
# (45 + 20) / 5
expression = [
    0x45,                    # Push 45
    0x14,                    # Push 20
    Instruction.ADD,         # 45 + 20 = 65
    0x05,                    # Push 5
    Instruction.DIVIDE       # 65 / 5 = 13
]

# Advanced: Ability System
class AbilityVM:
    def __init__(self):
        self.registers = [0] * 16
        self.pc = 0  # Program counter
    
    def execute_ability(self, bytecode, target):
        self.pc = 0
        
        while self.pc < len(bytecode):
            opcode = bytecode[self.pc]
            self.pc += 1
            
            if opcode == 0x01:  # DAMAGE
                amount = bytecode[self.pc]
                target.take_damage(amount)
                self.pc += 1
            
            elif opcode == 0x02:  # HEAL
                amount = bytecode[self.pc]
                target.heal(amount)
                self.pc += 1
            
            elif opcode == 0x03:  # IF_HEALTH_BELOW
                threshold = bytecode[self.pc]
                jump_offset = bytecode[self.pc + 1]
                if target.health < threshold:
                    self.pc += jump_offset
                else:
                    self.pc += 2

# Complex Spell: "If health < 50%, heal 30, else damage 40"
conditional_spell = [
    0x03, 0x32, 0x04,  # IF_HEALTH_BELOW 50%, jump 4
    0x01, 0x28,        # DAMAGE 40
    0xFF,              # END
    0x02, 0x1E         # HEAL 30
]
```

---

### 11. Subclass Sandbox Pattern

**Intent:** Define behavior in base class with protected operations that subclasses combine.

**Key Concepts:**
- Base class provides toolkit of operations
- Subclasses implement behavior using these tools
- Reduces code duplication
- Enforces consistent patterns
- Common for powers, abilities, moves

**Scripting Pattern:**

```python
# Base Superpower Class
class Superpower:
    """Provides protected operations for subclasses"""
    
    def activate(self):
        """Override to define power behavior"""
        pass
    
    # Protected helper methods
    def _move(self, x, y, z):
        """Move to position"""
        pass
    
    def _play_sound(self, sound_id):
        """Play sound effect"""
        pass
    
    def _spawn_particles(self, particle_type):
        """Create particle effect"""
        pass
    
    def _apply_damage(self, target, amount):
        """Deal damage to target"""
        pass

# Concrete Powers using Sandbox
class SkyLaunch(Superpower):
    def activate(self):
        self._play_sound(SOUND_SPROING)
        self._spawn_particles(PARTICLE_DUST)
        self._move(0, 100, 0)  # Jump up

class GroundDive(Superpower):
    def activate(self):
        self._play_sound(SOUND_SWOOP)
        self._spawn_particles(PARTICLE_SPARKLES)
        self._move(0, -100, 0)  # Dive down
        self._apply_damage(self._get_nearby_enemies(), 50)

# More Complex Example
class FireBlast(Superpower):
    def activate(self):
        # Chain protected operations
        self._play_sound(SOUND_FIRE)
        self._spawn_particles(PARTICLE_FIRE)
        
        enemies = self._get_enemies_in_cone()
        for enemy in enemies:
            self._apply_damage(enemy, 30)
            self._apply_status(enemy, STATUS_BURNING)

# Monster AI using Sandbox
class Monster:
    """Base monster with AI operations"""
    
    def update(self):
        """Override in subclasses"""
        pass
    
    # Protected operations
    def _move_towards(self, target):
        pass
    
    def _attack(self, target):
        pass
    
    def _play_animation(self, anim):
        pass
    
    def _spawn_minions(self, count):
        pass

class Goblin(Monster):
    def update(self):
        if self._can_see_player():
            self._move_towards(self._player)
            if self._in_range(self._player):
                self._attack(self._player)

class Dragon(Monster):
    def update(self):
        if self._health < 0.3:
            self._spawn_minions(3)
            self._play_animation("roar")
        
        if self._player_distance() < 50:
            self._breathe_fire()
        else:
            self._fly_around()

# Ability System with Costs
class Ability:
    def __init__(self):
        self.cooldown = 0
    
    def use(self, caster):
        if not self._can_cast(caster):
            return False
        
        self._consume_resources(caster)
        self.cast(caster)
        self.cooldown = self.get_cooldown()
        return True
    
    def cast(self, caster):
        """Override to define ability"""
        pass
    
    # Sandbox operations
    def _can_cast(self, caster):
        return (self.cooldown == 0 and 
                caster.mana >= self.get_mana_cost())
    
    def _consume_resources(self, caster):
        caster.mana -= self.get_mana_cost()
    
    def _deal_damage(self, target, amount):
        target.health -= amount

class Fireball(Ability):
    def get_mana_cost(self):
        return 50
    
    def get_cooldown(self):
        return 5.0
    
    def cast(self, caster):
        target = caster.get_target()
        self._deal_damage(target, 100)
        self._create_explosion(target.position)
```

---

### 12. Type Object Pattern

**Intent:** Share behavior and data between similar objects through delegation to a type object.

**Key Concepts:**
- Separates object type from instance
- Type defines breed/class characteristics
- Enables data-driven object definitions
- Changes to type affect all instances
- Common for monsters, items, units

**Scripting Pattern:**

```python
# Type Object
class Breed:
    """Defines species characteristics"""
    def __init__(self, name, health, attack, texture):
        self.name = name
        self.health = health
        self.attack = attack
        self.defense = 0
        self.texture = texture
    
    def get_attack_string(self):
        return f"The {self.name} attacks!"

# Instance Object
class Monster:
    """Individual monster instance"""
    def __init__(self, breed, x, y):
        self.breed = breed
        self.x = x
        self.y = y
        self.current_health = breed.health  # Instance-specific
    
    def get_attack(self):
        return self.breed.attack  # Shared from breed
    
    def attack_message(self):
        return self.breed.get_attack_string()

# Create breeds
troll_breed = Breed("Troll", 150, 30, "troll.png")
dragon_breed = Breed("Dragon", 500, 80, "dragon.png")

# Spawn monsters
monsters = [
    Monster(troll_breed, 10, 20),
    Monster(troll_breed, 30, 40),
    Monster(dragon_breed, 50, 60)
]

# Modify breed affects all instances
troll_breed.defense = 10  # All trolls now have 10 defense

# Advanced: Inheritance between Types
class Breed:
    def __init__(self, name, parent=None):
        self.name = name
        self.parent = parent
        self.attributes = {}
    
    def get_attribute(self, name):
        """Look up attribute chain"""
        if name in self.attributes:
            return self.attributes[name]
        elif self.parent:
            return self.parent.get_attribute(name)
        return None

# Create breed hierarchy
base_monster = Breed("Monster")
base_monster.attributes = {"health": 100, "speed": 5}

goblin = Breed("Goblin", parent=base_monster)
goblin.attributes = {"attack": 15}  # Inherits health and speed

orc = Breed("Orc", parent=base_monster)
orc.attributes = {"health": 200, "attack": 25}  # Overrides health

# JSON-Based Type System
import json

class ItemType:
    def __init__(self, data):
        self.id = data["id"]
        self.name = data["name"]
        self.stack_size = data.get("stack_size", 1)
        self.sellable = data.get("sellable", True)
        self.icon = data["icon"]

class Item:
    def __init__(self, item_type, quantity=1):
        self.type = item_type
        self.quantity = quantity
    
    def can_stack_with(self, other):
        return (self.type == other.type and 
                self.quantity + other.quantity <= self.type.stack_size)

# Load item types from data
item_types = {}
with open("items.json") as f:
    data = json.load(f)
    for item_data in data["items"]:
        item_type = ItemType(item_data)
        item_types[item_type.id] = item_type

# Create instances
sword = Item(item_types["iron_sword"])
arrow = Item(item_types["arrow"], quantity=50)

# Constructor Functions Pattern
class BreedFactory:
    @staticmethod
    def create_troll():
        breed = Breed("Troll")
        breed.health = 150
        breed.attack = 30
        breed.ai = "aggressive"
        return breed
    
    @staticmethod
    def create_dragon():
        breed = Breed("Dragon")
        breed.health = 500
        breed.attack = 80
        breed.ai = "boss"
        breed.can_fly = True
        return breed
```

---

## Decoupling Patterns

### 13. Component Pattern

**Intent:** Allow single entity to span multiple domains without coupling domains to each other.

**Key Concepts:**
- Entity is bag of components
- Each component handles one concern (rendering, physics, AI)
- Promotes composition over inheritance
- Foundation of modern game engines (Unity, Unreal)
- High flexibility and reusability

**Scripting Pattern:**

```python
# Component Base
class Component:
    def __init__(self):
        self.entity = None
    
    def update(self, delta_time):
        pass
    
    def receive_message(self, message):
        pass

# Concrete Components
class PhysicsComponent(Component):
    def __init__(self):
        super().__init__()
        self.velocity = Vector2(0, 0)
        self.gravity = Vector2(0, -9.8)
    
    def update(self, delta_time):
        self.velocity += self.gravity * delta_time
        self.entity.x += self.velocity.x * delta_time
        self.entity.y += self.velocity.y * delta_time

class GraphicsComponent(Component):
    def __init__(self, sprite):
        super().__init__()
        self.sprite = sprite
    
    def update(self, delta_time):
        render_sprite(self.sprite, self.entity.x, self.entity.y)

class InputComponent(Component):
    def update(self, delta_time):
        if is_key_down(KEY_LEFT):
            self.entity.get_component(PhysicsComponent).velocity.x = -5
        elif is_key_down(KEY_RIGHT):
            self.entity.get_component(PhysicsComponent).velocity.x = 5

# Entity Container
class Entity:
    def __init__(self):
        self.components = []
        self.x = 0
        self.y = 0
    
    def add_component(self, component):
        component.entity = self
        self.components.append(component)
    
    def get_component(self, component_type):
        for component in self.components:
            if isinstance(component, component_type):
                return component
        return None
    
    def update(self, delta_time):
        for component in self.components:
            component.update(delta_time)
    
    def send_message(self, message):
        for component in self.components:
            component.receive_message(message)

# Create Entities
player = Entity()
player.add_component(PhysicsComponent())
player.add_component(GraphicsComponent("player.png"))
player.add_component(InputComponent())

enemy = Entity()
enemy.add_component(PhysicsComponent())
enemy.add_component(GraphicsComponent("enemy.png"))
enemy.add_component(AIComponent())

# Component Communication via Messages
class HealthComponent(Component):
    def __init__(self, max_health):
        super().__init__()
        self.health = max_health
        self.max_health = max_health
    
    def receive_message(self, message):
        if message.type == "take_damage":
            self.health -= message.amount
            if self.health <= 0:
                self.entity.send_message(Message("died"))

# ECS (Entity Component System) Style
class Transform:
    """Pure data component"""
    def __init__(self, x=0, y=0):
        self.x = x
        self.y = y

class Velocity:
    def __init__(self, vx=0, vy=0):
        self.vx = vx
        self.vy = vy

class Sprite:
    def __init__(self, texture):
        self.texture = texture

# Systems process components
class PhysicsSystem:
    def update(self, entities, delta_time):
        for entity in entities:
            if hasattr(entity, 'transform') and hasattr(entity, 'velocity'):
                entity.transform.x += entity.velocity.vx * delta_time
                entity.transform.y += entity.velocity.vy * delta_time

class RenderSystem:
    def update(self, entities):
        for entity in entities:
            if hasattr(entity, 'transform') and hasattr(entity, 'sprite'):
                draw_sprite(entity.sprite.texture, 
                          entity.transform.x, entity.transform.y)

# ECS Entity Creation
class ECSEntity:
    def __init__(self):
        pass

player = ECSEntity()
player.transform = Transform(100, 100)
player.velocity = Velocity(5, 0)
player.sprite = Sprite("player.png")
```

---

### 14. Event Queue Pattern

**Intent:** Decouple when events are sent from when they're processed.

**Key Concepts:**
- Queue stores events for later processing
- Decouples sender from receiver
- Controls timing of event processing
- Prevents recursion and reentrancy issues
- Can merge, filter, or prioritize events

**Scripting Pattern:**

```python
from collections import deque
from enum import Enum

# Event Types
class EventType(Enum):
    SOUND_PLAY = 1
    PARTICLE_SPAWN = 2
    ENTITY_SPAWN = 3
    ACHIEVEMENT_UNLOCK = 4

# Event Structure
class Event:
    def __init__(self, event_type, data=None):
        self.type = event_type
        self.data = data

# Event Queue
class EventQueue:
    def __init__(self, max_size=100):
        self.queue = deque(maxlen=max_size)
    
    def send(self, event_type, data=None):
        """Add event to queue"""
        event = Event(event_type, data)
        self.queue.append(event)
    
    def process(self):
        """Process all pending events"""
        while self.queue:
            event = self.queue.popleft()
            self._handle_event(event)
    
    def _handle_event(self, event):
        if event.type == EventType.SOUND_PLAY:
            play_sound(event.data["sound_id"])
        elif event.type == EventType.PARTICLE_SPAWN:
            spawn_particles(event.data["type"], event.data["position"])
        elif event.type == EventType.ENTITY_SPAWN:
            spawn_entity(event.data["entity_type"], event.data["position"])

# Usage
event_queue = EventQueue()

# Send events during gameplay
def enemy_dies(enemy):
    event_queue.send(EventType.SOUND_PLAY, {"sound_id": "enemy_death"})
    event_queue.send(EventType.PARTICLE_SPAWN, {
        "type": "explosion",
        "position": enemy.position
    })
    event_queue.send(EventType.ACHIEVEMENT_UNLOCK, {"achievement": "first_kill"})

# Process events at controlled time
def game_loop():
    while running:
        handle_input()
        update_physics()
        
        # Process all queued events
        event_queue.process()
        
        render()

# Ring Buffer Implementation (Fixed Size)
class RingBufferQueue:
    def __init__(self, capacity=16):
        self.events = [None] * capacity
        self.head = 0
        self.tail = 0
        self.capacity = capacity
    
    def send(self, event):
        next_tail = (self.tail + 1) % self.capacity
        
        # Queue full - oldest event will be dropped
        if next_tail == self.head:
            print("Warning: Event queue full, dropping oldest event")
            self.head = (self.head + 1) % self.capacity
        
        self.events[self.tail] = event
        self.tail = next_tail
    
    def read(self):
        """Get next event without removing it"""
        if self.head == self.tail:
            return None
        return self.events[self.head]
    
    def process_one(self):
        """Process single event"""
        event = self.read()
        if event:
            handle_event(event)
            self.head = (self.head + 1) % self.capacity

# Priority Event Queue
import heapq

class PriorityEventQueue:
    def __init__(self):
        self.heap = []
        self.counter = 0
    
    def send(self, event, priority=0):
        """Lower priority number = higher priority"""
        # Counter ensures FIFO for same priority
        heapq.heappush(self.heap, (priority, self.counter, event))
        self.counter += 1
    
    def process(self):
        while self.heap:
            priority, _, event = heapq.heappop(self.heap)
            handle_event(event)

# Aggregated/Merged Events
class SmartEventQueue:
    def __init__(self):
        self.events = {}
    
    def send(self, event_type, data):
        # Merge similar events
        if event_type == EventType.SOUND_PLAY:
            # Only keep latest sound request per sound_id
            sound_id = data["sound_id"]
            self.events[f"sound_{sound_id}"] = Event(event_type, data)
        else:
            # Generate unique key
            key = f"{event_type}_{id(data)}"
            self.events[key] = Event(event_type, data)
    
    def process(self):
        for event in self.events.values():
            handle_event(event)
        self.events.clear()

# Delayed Events
class DelayedEventQueue:
    def __init__(self):
        self.events = []
    
    def send(self, event, delay=0.0):
        self.events.append({
            "event": event,
            "time_left": delay
        })
    
    def update(self, delta_time):
        to_process = []
        
        for item in self.events:
            item["time_left"] -= delta_time
            if item["time_left"] <= 0:
                to_process.append(item["event"])
        
        # Process ready events
        for event in to_process:
            handle_event(event)
            self.events = [e for e in self.events if e["event"] != event]
```

---

### 15. Service Locator Pattern

**Intent:** Provide global access to a service without coupling to its implementation.

**Key Concepts:**
- Decouples code from concrete service classes
- Allows swapping implementations (null object, mock)
- Alternative to Singleton with better testability
- Can provide different services per context
- Services registered at runtime

**Scripting Pattern:**

```python
# Service Interface
class Audio:
    def play_sound(self, sound_id):
        pass
    
    def stop_sound(self, sound_id):
        pass
    
    def stop_all_sounds(self):
        pass

# Concrete Service
class ConsoleAudio(Audio):
    def play_sound(self, sound_id):
        print(f"Playing sound: {sound_id}")
    
    def stop_sound(self, sound_id):
        print(f"Stopping sound: {sound_id}")
    
    def stop_all_sounds(self):
        print("Stopping all sounds")

# Null Service (for when audio unavailable)
class NullAudio(Audio):
    def play_sound(self, sound_id):
        pass  # Do nothing
    
    def stop_sound(self, sound_id):
        pass
    
    def stop_all_sounds(self):
        pass

# Service Locator
class ServiceLocator:
    _audio_service = NullAudio()  # Default to null service
    
    @classmethod
    def initialize(cls):
        cls._audio_service = NullAudio()
    
    @classmethod
    def provide(cls, service):
        if service is None:
            cls._audio_service = NullAudio()
        else:
            cls._audio_service = service
    
    @classmethod
    def get_audio(cls):
        return cls._audio_service

# Usage
# At game startup
ServiceLocator.initialize()
ServiceLocator.provide(ConsoleAudio())

# Anywhere in code
audio = ServiceLocator.get_audio()
audio.play_sound("explosion")

# For testing, provide mock
class LoggingAudio(Audio):
    def __init__(self):
        self.sounds_played = []
    
    def play_sound(self, sound_id):
        self.sounds_played.append(sound_id)

# Multiple Services
class ServiceLocator:
    _services = {}
    
    @classmethod
    def provide(cls, service_type, service):
        cls._services[service_type] = service
    
    @classmethod
    def get(cls, service_type):
        if service_type in cls._services:
            return cls._services[service_type]
        return None

# Register multiple services
ServiceLocator.provide("audio", ConsoleAudio())
ServiceLocator.provide("graphics", GraphicsService())
ServiceLocator.provide("physics", PhysicsService())

# Access services
audio = ServiceLocator.get("audio")
graphics = ServiceLocator.get("graphics")

# Scoped Service Locator
class ScopedServiceLocator:
    def __init__(self, parent=None):
        self.services = {}
        self.parent = parent
    
    def provide(self, service_type, service):
        self.services[service_type] = service
    
    def get(self, service_type):
        # Check local scope first
        if service_type in self.services:
            return self.services[service_type]
        # Fall back to parent scope
        elif self.parent:
            return self.parent.get(service_type)
        return None

# Global services
global_locator = ScopedServiceLocator()
global_locator.provide("audio", ConsoleAudio())

# Level-specific services
level_locator = ScopedServiceLocator(parent=global_locator)
level_locator.provide("audio", MuffledAudio())  # Override for underwater level

# Level uses muffled audio, other systems use console audio
audio = level_locator.get("audio")  # Gets MuffledAudio
renderer = level_locator.get("renderer")  # Gets from global_locator

# Decorated Service
class LoggedAudio(Audio):
    def __init__(self, wrapped):
        self.wrapped = wrapped
    
    def play_sound(self, sound_id):
        print(f"[LOG] Playing {sound_id}")
        self.wrapped.play_sound(sound_id)
    
    def stop_sound(self, sound_id):
        print(f"[LOG] Stopping {sound_id}")
        self.wrapped.stop_sound(sound_id)

# Wrap service with logging
base_audio = ConsoleAudio()
logged_audio = LoggedAudio(base_audio)
ServiceLocator.provide(logged_audio)
```

---

## Optimization Patterns

### 16. Data Locality Pattern

**Intent:** Accelerate memory access by arranging data to take advantage of CPU caching.

**Key Concepts:**
- CPU cache is much faster than RAM
- Access data sequentially (cache-friendly)
- Store hot data contiguously
- Separate hot/cold data
- Use arrays of structs or struct of arrays

**Scripting Pattern:**

```python
# Cache-Unfriendly: Object-Oriented Design
class GameEntity:
    def __init__(self):
        self.position = Vector3()
        self.velocity = Vector3()
        self.mesh = Mesh()
        self.material = Material()
        self.collider = Collider()
        self.ai_state = AIState()
        self.health = 100
        # Many more fields...
    
    def update(self, delta_time):
        self.position += self.velocity * delta_time

# Problem: Objects scattered in memory
entities = [GameEntity() for _ in range(10000)]
for entity in entities:  # Poor cache performance
    entity.update(delta_time)

# Cache-Friendly: Data-Oriented Design (AoS - Array of Structs)
class Transform:
    __slots__ = ['x', 'y', 'z', 'vx', 'vy', 'vz']
    
    def __init__(self, x=0, y=0, z=0, vx=0, vy=0, vz=0):
        self.x = x
        self.y = y
        self.z = z
        self.vx = vx
        self.vy = vy
        self.vz = vz

transforms = [Transform() for _ in range(10000)]

def update_transforms(transforms, delta_time):
    for t in transforms:  # Sequential memory access
        t.x += t.vx * delta_time
        t.y += t.vy * delta_time
        t.z += t.vz * delta_time

# Even Better: SoA (Struct of Arrays) - Most cache friendly
import numpy as np

class TransformSoA:
    def __init__(self, count):
        self.x = np.zeros(count)
        self.y = np.zeros(count)
        self.z = np.zeros(count)
        self.vx = np.zeros(count)
        self.vy = np.zeros(count)
        self.vz = np.zeros(count)
        self.count = count
    
    def update(self, delta_time):
        # SIMD-friendly, perfect cache locality
        self.x += self.vx * delta_time
        self.y += self.vy * delta_time
        self.z += self.vz * delta_time

transforms = TransformSoA(10000)
transforms.update(delta_time)

# Hot/Cold Data Splitting
class AIComponent_Bad:
    """Everything together - bad for cache"""
    def __init__(self):
        self.state = "idle"  # Hot - accessed every frame
        self.target = None   # Hot
        self.path = []       # Cold - rarely accessed
        self.dialogue_tree = {} # Cold
        self.quest_data = {}    # Cold

class AIComponent_Good:
    """Hot data separate from cold data"""
    def __init__(self):
        # Hot data - frequently accessed
        self.state = "idle"
        self.target = None
        self.state_timer = 0.0
        
        # Reference to cold data
        self.data_id = 0

# Cold data stored separately
ai_data_storage = {}

def update_ai(ai_component, delta_time):
    # Fast - only touches hot data
    ai_component.state_timer += delta_time
    
    if ai_component.target:
        move_towards(ai_component.target)
    
    # Rarely access cold data
    if needs_dialogue():
        data = ai_data_storage[ai_component.data_id]
        show_dialogue(data.dialogue_tree)

# Particle System - Excellent Data Locality
class ParticleSystem:
    def __init__(self, max_particles):
        self.count = 0
        self.max = max_particles
        
        # All positions together
        self.x = np.zeros(max_particles)
        self.y = np.zeros(max_particles)
        
        # All velocities together
        self.vx = np.zeros(max_particles)
        self.vy = np.zeros(max_particles)
        
        # All lifetimes together
        self.lifetime = np.zeros(max_particles)
    
    def update(self, delta_time):
        if self.count == 0:
            return
        
        # Process active particles sequentially
        alive = self.lifetime[:self.count] > 0
        
        # Vectorized operations - great cache performance
        self.x[:self.count] += self.vx[:self.count] * delta_time
        self.y[:self.count] += self.vy[:self.count] * delta_time
        self.lifetime[:self.count] -= delta_time
        
        # Compact array (remove dead particles)
        self.x[:] = self.x[alive]
        self.y[:] = self.y[alive]
        self.vx[:] = self.vx[alive]
        self.vy[:] = self.vy[alive]
        self.lifetime[:] = self.lifetime[alive]
        self.count = np.sum(alive)

# Component Array Pattern
class ComponentArray:
    """Cache-friendly component storage"""
    def __init__(self, component_type, capacity):
        self.components = [component_type() for _ in range(capacity)]
        self.count = 0
    
    def add(self):
        if self.count < len(self.components):
            component = self.components[self.count]
            self.count += 1
            return component
        return None
    
    def remove_at(self, index):
        # Swap with last element to maintain contiguity
        last_index = self.count - 1
        self.components[index] = self.components[last_index]
        self.count -= 1
    
    def update_all(self, delta_time):
        # Sequential access through active components
        for i in range(self.count):
            self.components[i].update(delta_time)
```

---

### 17. Dirty Flag Pattern

**Intent:** Avoid expensive recomputation by deferring it until result is needed and marking state as "dirty".

**Key Concepts:**
- Track when data becomes invalid (dirty)
- Recompute only when needed and when dirty
- Common for transforms, animations, UI layouts
- Trades memory for CPU time
- Can cascade through hierarchies

**Scripting Pattern:**

```python
# Basic Dirty Flag
class Transform:
    def __init__(self):
        self.local_position = Vector3(0, 0, 0)
        self.local_rotation = Quaternion()
        self.local_scale = Vector3(1, 1, 1)
        
        self.world_transform = Matrix4x4()
        self.is_dirty = True
    
    def set_local_position(self, position):
        self.local_position = position
        self.is_dirty = True
    
    def set_local_rotation(self, rotation):
        self.local_rotation = rotation
        self.is_dirty = True
    
    def get_world_transform(self):
        if self.is_dirty:
            self._recompute_world_transform()
            self.is_dirty = False
        return self.world_transform
    
    def _recompute_world_transform(self):
        # Expensive computation
        self.world_transform = Matrix4x4.compose(
            self.local_position,
            self.local_rotation,
            self.local_scale
        )

# Usage
transform = Transform()
transform.set_local_position(Vector3(10, 20, 30))
transform.set_local_rotation(Quaternion(0, 45, 0))
transform.set_local_scale(Vector3(2, 2, 2))

# Computation only happens here, not on each set call
world_matrix = transform.get_world_transform()

# Hierarchical Dirty Flag (Transform Tree)
class SceneNode:
    def __init__(self, parent=None):
        self.parent = parent
        self.children = []
        self.local_transform = Matrix4x4()
        self.world_transform = Matrix4x4()
        self.is_dirty = True
        
        if parent:
            parent.add_child(self)
    
    def add_child(self, child):
        self.children.append(child)
    
    def set_local_transform(self, transform):
        self.local_transform = transform
        self._mark_dirty()
    
    def _mark_dirty(self):
        """Mark this node and all children as dirty"""
        self.is_dirty = True
        for child in self.children:
            child._mark_dirty()
    
    def get_world_transform(self):
        if self.is_dirty:
            if self.parent:
                parent_world = self.parent.get_world_transform()
                self.world_transform = parent_world * self.local_transform
            else:
                self.world_transform = self.local_transform
            
            self.is_dirty = False
        
        return self.world_transform

# Mesh Bounding Box with Dirty Flag
class Mesh:
    def __init__(self):
        self.vertices = []
        self.bounding_box = None
        self.bbox_dirty = True
    
    def add_vertex(self, vertex):
        self.vertices.append(vertex)
        self.bbox_dirty = True
    
    def transform_vertices(self, matrix):
        for i, vertex in enumerate(self.vertices):
            self.vertices[i] = matrix * vertex
        self.bbox_dirty = True
    
    def get_bounding_box(self):
        if self.bbox_dirty:
            self._recalculate_bounding_box()
            self.bbox_dirty = False
        return self.bounding_box
    
    def _recalculate_bounding_box(self):
        if not self.vertices:
            self.bounding_box = BoundingBox()
            return
        
        min_point = self.vertices[0].copy()
        max_point = self.vertices[0].copy()
        
        for vertex in self.vertices[1:]:
            min_point = Vector3.min(min_point, vertex)
            max_point = Vector3.max(max_point, vertex)
        
        self.bounding_box = BoundingBox(min_point, max_point)

# UI Layout with Dirty Flag
class UIElement:
    def __init__(self):
        self.x = 0
        self.y = 0
        self.width = 0
        self.height = 0
        self.children = []
        self.layout_dirty = True
    
    def set_size(self, width, height):
        self.width = width
        self.height = height
        self.layout_dirty = True
    
    def add_child(self, child):
        self.children.append(child)
        self.layout_dirty = True
    
    def layout(self):
        if not self.layout_dirty:
            return
        
        # Expensive layout calculation
        current_y = 0
        for child in self.children:
            child.x = 0
            child.y = current_y
            current_y += child.height
            child.layout()  # Recursively layout children
        
        self.layout_dirty = False

# Graphics Scene with Dirty Flag
class GraphicsNode:
    def __init__(self):
        self.mesh = None
        self.material = None
        self.mesh_dirty = False
        self.material_dirty = False
        self.render_data = None
    
    def set_mesh(self, mesh):
        self.mesh = mesh
        self.mesh_dirty = True
    
    def set_material(self, material):
        self.material = material
        self.material_dirty = True
    
    def get_render_data(self):
        """Lazy rebuild of GPU data"""
        if self.mesh_dirty or self.material_dirty:
            self._rebuild_render_data()
            self.mesh_dirty = False
            self.material_dirty = False
        return self.render_data
    
    def _rebuild_render_data(self):
        # Expensive: upload to GPU
        self.render_data = RenderData(self.mesh, self.material)

# Shared Dirty Flag (Multiple Observers)
class PhysicsBody:
    def __init__(self):
        self.position = Vector3()
        self.rotation = Quaternion()
        self.dirty_observers = []
    
    def register_observer(self, observer):
        self.dirty_observers.append(observer)
    
    def set_position(self, position):
        self.position = position
        for observer in self.dirty_observers:
            observer.mark_dirty()

class Collider:
    def __init__(self, body):
        self.body = body
        self.world_bounds = None
        self.is_dirty = True
        body.register_observer(self)
    
    def mark_dirty(self):
        self.is_dirty = True
    
    def get_world_bounds(self):
        if self.is_dirty:
            # Recompute based on body position
            self.world_bounds = self._compute_bounds()
            self.is_dirty = False
        return self.world_bounds
```

---

### 18. Object Pool Pattern

**Intent:** Avoid allocation/deallocation overhead by reusing objects from a fixed pool.

**Key Concepts:**
- Pre-allocate objects at startup
- Reuse dead objects instead of creating new ones
- Eliminates garbage collection spikes
- Common for bullets, particles, audio sources
- Manage object lifecycle (active/inactive)

**Scripting Pattern:**

```python
# Basic Object Pool
class Particle:
    def __init__(self):
        self.x = 0
        self.y = 0
        self.vx = 0
        self.vy = 0
        self.lifetime = 0
        self.in_use = False
    
    def init(self, x, y, vx, vy):
        """Reinitialize particle"""
        self.x = x
        self.y = y
        self.vx = vx
        self.vy = vy
        self.lifetime = 1.0
        self.in_use = True
    
    def update(self, delta_time):
        if not self.in_use:
            return False
        
        self.x += self.vx * delta_time
        self.y += self.vy * delta_time
        self.lifetime -= delta_time
        
        if self.lifetime <= 0:
            self.in_use = False
            return False
        return True

class ParticlePool:
    def __init__(self, pool_size=1000):
        self.pool = [Particle() for _ in range(pool_size)]
    
    def create(self, x, y, vx, vy):
        """Get particle from pool"""
        for particle in self.pool:
            if not particle.in_use:
                particle.init(x, y, vx, vy)
                return particle
        
        # Pool exhausted
        return None
    
    def update(self, delta_time):
        for particle in self.pool:
            if particle.in_use:
                particle.update(delta_time)

# Usage
particle_pool = ParticlePool(1000)

def spawn_explosion(x, y):
    for i in range(20):
        angle = (i / 20.0) * 2 * 3.14159
        vx = math.cos(angle) * 50
        vy = math.sin(angle) * 50
        particle_pool.create(x, y, vx, vy)

# Object Pool with Free List
class ObjectPool:
    def __init__(self, object_type, size):
        self.pool = [object_type() for _ in range(size)]
        self.first_available = 0
        
        # Link objects into free list
        for i in range(size - 1):
            self.pool[i].next = self.pool[i + 1]
        self.pool[-1].next = None
    
    def allocate(self):
        if not self.first_available:
            return None
        
        obj = self.first_available
        self.first_available = obj.next
        obj.in_use = True
        return obj
    
    def free(self, obj):
        obj.in_use = False
        obj.next = self.first_available
        self.first_available = obj

# Generic Pool
class Pool:
    def __init__(self, factory, size):
        self.factory = factory
        self.available = [factory() for _ in range(size)]
        self.in_use = []
    
    def acquire(self):
        if self.available:
            obj = self.available.pop()
            self.in_use.append(obj)
            return obj
        # Could expand pool or return None
        return None
    
    def release(self, obj):
        if obj in self.in_use:
            self.in_use.remove(obj)
            obj.reset()  # Clean up object
            self.available.append(obj)

# Bullet Pool Example
class Bullet:
    def __init__(self):
        self.x = 0
        self.y = 0
        self.vx = 0
        self.vy = 0
        self.active = False
    
    def init(self, x, y, vx, vy):
        self.x = x
        self.y = y
        self.vx = vx
        self.vy = vy
        self.active = True
    
    def update(self, delta_time):
        self.x += self.vx * delta_time
        self.y += self.vy * delta_time
        
        # Deactivate if off-screen
        if self.x < 0 or self.x > SCREEN_WIDTH:
            self.active = False

class BulletPool:
    def __init__(self, size=100):
        self.bullets = [Bullet() for _ in range(size)]
        self.next_slot = 0
    
    def spawn(self, x, y, vx, vy):
        # Find free slot (circular)
        start_slot = self.next_slot
        
        while True:
            bullet = self.bullets[self.next_slot]
            self.next_slot = (self.next_slot + 1) % len(self.bullets)
            
            if not bullet.active:
                bullet.init(x, y, vx, vy)
                return bullet
            
            # Wrapped around - pool full
            if self.next_slot == start_slot:
                return None
    
    def update_all(self, delta_time):
        for bullet in self.bullets:
            if bullet.active:
                bullet.update(delta_time)

# Audio Source Pool
class AudioSource:
    def __init__(self):
        self.sound = None
        self.volume = 1.0
        self.playing = False
    
    def play(self, sound, volume=1.0):
        self.sound = sound
        self.volume = volume
        self.playing = True
        # Start playback
    
    def stop(self):
        self.playing = False
        self.sound = None

class AudioPool:
    def __init__(self, size=32):
        self.sources = [AudioSource() for _ in range(size)]
    
    def play_sound(self, sound, volume=1.0):
        # Find available source
        for source in self.sources:
            if not source.playing:
                source.play(sound, volume)
                return source
        
        # All sources busy - either skip or steal oldest
        self.sources[0].play(sound, volume)
        return self.sources[0]
    
    def update(self):
        for source in self.sources:
            if source.playing and source.is_finished():
                source.stop()
```

---

### 19. Spatial Partition Pattern

**Intent:** Efficiently locate objects by storing them in a spatial data structure.

**Key Concepts:**
- Avoid O(n²) collision checks
- Divide space into regions (grid, quadtree, BSP)
- Query only nearby objects
- Essential for physics, rendering culling, AI
- Trade memory for query speed

**Scripting Pattern:**

```python
# Fixed Grid Partition
class Grid:
    def __init__(self, cell_size, width, height):
        self.cell_size = cell_size
        self.cols = width // cell_size
        self.rows = height // cell_size
        self.cells = [[[] for _ in range(self.cols)] 
                     for _ in range(self.rows)]
    
    def add(self, entity):
        cell_x = int(entity.x // self.cell_size)
        cell_y = int(entity.y // self.cell_size)
        
        if 0 <= cell_x < self.cols and 0 <= cell_y < self.rows:
            self.cells[cell_y][cell_x].append(entity)
    
    def get_nearby(self, x, y, radius):
        """Get all entities within radius"""
        nearby = []
        
        # Calculate cell range
        min_cell_x = max(0, int((x - radius) // self.cell_size))
        max_cell_x = min(self.cols - 1, int((x + radius) // self.cell_size))
        min_cell_y = max(0, int((y - radius) // self.cell_size))
        max_cell_y = min(self.rows - 1, int((y + radius) // self.cell_size))
        
        # Check cells in range
        for cy in range(min_cell_y, max_cell_y + 1):
            for cx in range(min_cell_x, max_cell_x + 1):
                nearby.extend(self.cells[cy][cx])
        
        return nearby
    
    def clear(self):
        for row in self.cells:
            for cell in row:
                cell.clear()

# Usage for Collision Detection
grid = Grid(cell_size=50, width=1000, height=1000)

def update(entities):
    grid.clear()
    
    # Add all entities to grid
    for entity in entities:
        grid.add(entity)
    
    # Check collisions only with nearby entities
    for entity in entities:
        nearby = grid.get_nearby(entity.x, entity.y, radius=100)
        for other in nearby:
            if entity != other and entity.collides_with(other):
                handle_collision(entity, other)

# Quadtree
class Quadtree:
    def __init__(self, bounds, capacity=4, max_depth=8, depth=0):
        self.bounds = bounds  # Rectangle(x, y, width, height)
        self.capacity = capacity
        self.max_depth = max_depth
        self.depth = depth
        self.objects = []
        self.divided = False
        self.children = []
    
    def subdivide(self):
        x, y, w, h = self.bounds
        hw, hh = w / 2, h / 2
        
        self.children = [
            Quadtree(Rectangle(x, y, hw, hh), 
                    self.capacity, self.max_depth, self.depth + 1),
            Quadtree(Rectangle(x + hw, y, hw, hh), 
                    self.capacity, self.max_depth, self.depth + 1),
            Quadtree(Rectangle(x, y + hh, hw, hh), 
                    self.capacity, self.max_depth, self.depth + 1),
            Quadtree(Rectangle(x + hw, y + hh, hw, hh), 
                    self.capacity, self.max_depth, self.depth + 1)
        ]
        self.divided = True
    
    def insert(self, entity):
        # Check if entity is in bounds
        if not self.bounds.contains(entity.x, entity.y):
            return False
        
        # If not at capacity, add here
        if len(self.objects) < self.capacity and not self.divided:
            self.objects.append(entity)
            return True
        
        # Otherwise subdivide if needed
        if not self.divided and self.depth < self.max_depth:
            self.subdivide()
        
        # Try to insert into children
        if self.divided:
            for child in self.children:
                if child.insert(entity):
                    return True
        
        # Fallback: keep in this node
        self.objects.append(entity)
        return True
    
    def query(self, range_rect):
        """Find all objects in range"""
        found = []
        
        # If range doesn't intersect bounds, return empty
        if not self.bounds.intersects(range_rect):
            return found
        
        # Check objects in this node
        for obj in self.objects:
            if range_rect.contains(obj.x, obj.y):
                found.append(obj)
        
        # Recursively check children
        if self.divided:
            for child in self.children:
                found.extend(child.query(range_rect))
        
        return found

# BSP Tree (Binary Space Partition) for Rendering
class BSPNode:
    def __init__(self, plane=None):
        self.plane = plane  # Dividing plane
        self.front = None
        self.back = None
        self.polygons = []
    
    def insert(self, polygon):
        if not self.plane:
            self.plane = polygon.get_plane()
            self.polygons.append(polygon)
            return
        
        # Classify polygon relative to plane
        classification = self.plane.classify(polygon)
        
        if classification == FRONT:
            if not self.front:
                self.front = BSPNode()
            self.front.insert(polygon)
        elif classification == BACK:
            if not self.back:
                self.back = BSPNode()
            self.back.insert(polygon)
        else:
            # Polygon spans plane - split it
            front_part, back_part = self.plane.split(polygon)
            self.front.insert(front_part)
            self.back.insert(back_part)
    
    def render(self, camera_pos):
        """Render back-to-front for painter's algorithm"""
        if self.plane.distance(camera_pos) > 0:
            # Camera in front of plane
            if self.back:
                self.back.render(camera_pos)
            self.draw_polygons()
            if self.front:
                self.front.render(camera_pos)
        else:
            # Camera behind plane
            if self.front:
                self.front.render(camera_pos)
            self.draw_polygons()
            if self.back:
                self.back.render(camera_pos)

# Spatial Hash
class SpatialHash:
    def __init__(self, cell_size):
        self.cell_size = cell_size
        self.hash_table = {}
    
    def _hash(self, x, y):
        cell_x = int(x // self.cell_size)
        cell_y = int(y // self.cell_size)
        return (cell_x, cell_y)
    
    def insert(self, entity):
        key = self._hash(entity.x, entity.y)
        if key not in self.hash_table:
            self.hash_table[key] = []
        self.hash_table[key].append(entity)
    
    def query_radius(self, x, y, radius):
        """Get all entities within radius"""
        found = []
        
        # Check cells within radius
        cell_radius = int(radius / self.cell_size) + 1
        center_cell_x = int(x // self.cell_size)
        center_cell_y = int(y // self.cell_size)
        
        for dy in range(-cell_radius, cell_radius + 1):
            for dx in range(-cell_radius, cell_radius + 1):
                key = (center_cell_x + dx, center_cell_y + dy)
                if key in self.hash_table:
                    for entity in self.hash_table[key]:
                        dist_sq = (entity.x - x)**2 + (entity.y - y)**2
                        if dist_sq <= radius**2:
                            found.append(entity)
        
        return found
    
    def clear(self):
        self.hash_table.clear()

# Usage Example: Efficient Enemy Finding
spatial_hash = SpatialHash(cell_size=100)

def update_game():
    spatial_hash.clear()
    
    # Insert all enemies
    for enemy in enemies:
        spatial_hash.insert(enemy)
    
    # Each bullet only checks nearby enemies
    for bullet in bullets:
        nearby_enemies = spatial_hash.query_radius(
            bullet.x, bullet.y, radius=50
        )
        for enemy in nearby_enemies:
            if bullet.collides_with(enemy):
                enemy.take_damage(bullet.damage)
                bullet.destroy()
```

---

## Key Takeaways

### When to Use Each Pattern

**Frequently Useful:**
- **Command** - Input handling, undo systems, AI recording
- **Observer** - Event systems, achievements, decoupled notifications
- **State** - Character controllers, AI behavior, menu systems
- **Component** - Entity systems, flexible object composition
- **Game Loop** - Core architecture (every game needs this)
- **Update Method** - Entity behavior management
- **Object Pool** - Bullets, particles, frequently spawned objects

**Situational:**
- **Flyweight** - Many similar objects (trees, tiles, particles)
- **Prototype** - Data-driven spawning, level editors
- **Service Locator** - Global systems with testability
- **Event Queue** - Decoupling event timing, preventing reentrancy
- **Dirty Flag** - Expensive calculations that change rarely
- **Spatial Partition** - Large numbers of objects needing proximity queries

**Advanced/Specific:**
- **Bytecode** - Scripting systems, mod support, behavior definition
- **Subclass Sandbox** - Ability systems, power definition
- **Type Object** - Data-driven object types, breed systems
- **Double Buffer** - Rendering, state updates, preventing partial reads
- **Data Locality** - Performance-critical systems with many objects

**Use Sparingly:**
- **Singleton** - Often indicates poor design; prefer service locator or dependency injection

---

## Performance Tips

1. **Cache locality is critical** - organize data for sequential access
2. **Avoid premature optimization** - profile first
3. **Object pools prevent GC spikes** - essential for mobile games
4. **Spatial partitioning** saves massive computation in physics/collision
5. **Dirty flags** prevent redundant calculations
6. **Events/observers** can be slow if overused - profile notification chains

---

## Architecture Principles

1. **Decouple when it helps, not by default** - abstraction has costs
2. **Data-driven design** enables rapid iteration
3. **Composition over inheritance** - prefer components
4. **Keep hot code paths simple** - performance-critical loops need clarity
5. **Balance flexibility with performance** - different systems have different needs
6. **Prototype quickly, architect thoughtfully** - know when to refactor

---
