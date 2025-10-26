{
  "metadata": {
    "title": "Comprehensive Game Engine Architecture Reference",
    "description": "Consolidated knowledge base from Jason Gregory's Game Engine Architecture and C++ Game Development Cookbook, providing extensive patterns, concepts, and implementation details for game engine development",
    "version": "1.0",
    "sources": [
      "Game Engine Architecture (3rd Edition) by Jason Gregory",
      "C++ Game Development Cookbook"
    ],
    "last_updated": "2025"
  },

  "architecture_overview": {
    "description": "Game engines are sophisticated layered architectures that sit between hardware/OS and game-specific code, providing reusable systems for common game development tasks",
    "core_philosophy": {
      "layered_approach": "Higher layers depend on lower layers only, creating clear separation of concerns",
      "data_driven_design": "Store game configuration in external files (JSON, XML) to allow designers to modify without code changes",
      "performance_first": "Data locality matters more than algorithms; consider cache lines and memory access patterns",
      "fail_fast": "Use assertions liberally in debug builds and validate inputs at API boundaries to catch bugs quickly",
      "loose_coupling": "Systems communicate via interfaces and use events/messages instead of direct calls"
    },
    "engine_layers": {
      "hardware_layer": {
        "description": "Direct interaction with hardware (CPU, GPU, memory, storage)",
        "components": ["CPU", "GPU", "Memory", "Storage", "Input devices", "Audio hardware"]
      },
      "platform_independence_layer": {
        "description": "Abstracts platform-specific differences",
        "components": ["File system abstraction", "Graphics API wrapper", "Input abstraction", "Audio API wrapper", "Network abstraction", "Threading primitives"]
      },
      "core_systems_layer": {
        "description": "Fundamental building blocks used throughout the engine",
        "components": ["Memory management", "Math library", "Data structures", "String handling", "Debug utilities", "Logging", "Profiling"]
      },
      "resource_layer": {
        "description": "Loading, managing, and caching game assets",
        "components": ["Resource manager", "Asset loading", "Streaming", "Reference counting", "Asset pipeline"]
      },
      "rendering_layer": {
        "description": "Visual output and graphics rendering",
        "components": ["Low-level renderer", "Scene graph", "Visual effects", "Lighting", "Materials", "Post-processing"]
      },
      "animation_layer": {
        "description": "Character animation and skeletal systems",
        "components": ["Skeletal animation", "Blend trees", "IK systems", "Animation compression", "State machines"]
      },
      "physics_layer": {
        "description": "Physical simulation and collision",
        "components": ["Collision detection", "Rigid body dynamics", "Soft body physics", "Ragdoll", "Particle systems"]
      },
      "gameplay_foundation_layer": {
        "description": "Core gameplay systems and entity management",
        "components": ["Entity-Component System", "Event system", "Scripting", "AI", "Camera system", "Game state management"]
      },
      "game_specific_layer": {
        "description": "Actual game implementation using the engine",
        "components": ["Game rules", "Player mechanics", "Level design", "UI", "Missions", "Story system"]
      }
    },
    "engine_subsystem_pattern": {
      "description": "Fundamental pattern for organizing engine systems with consistent lifecycle management",
      "interface": {
        "methods": [
          {
            "name": "Initialize",
            "return_type": "bool",
            "description": "Initializes the subsystem, allocates resources, returns true on success"
          },
          {
            "name": "Update",
            "parameters": ["float deltaTime"],
            "return_type": "void",
            "description": "Called every frame to update subsystem state"
          },
          {
            "name": "Shutdown",
            "return_type": "void",
            "description": "Cleans up resources and prepares for termination"
          }
        ]
      },
      "implementation_notes": [
        "Subsystems are initialized in dependency order during engine startup",
        "Update is called every frame in a specific order",
        "Shutdown is called in reverse order to handle dependencies correctly",
        "Each subsystem should be independent and communicate through well-defined interfaces"
      ],
      "code_example": "See engine_patterns.subsystem_manager for implementation"
    }
  },

  "memory_management": {
    "description": "Memory is the most critical resource in game engines. Proper management prevents fragmentation, improves cache performance, and enables predictable frame times",
    "fundamentals": {
      "stack_vs_heap": {
        "stack": {
          "description": "LIFO (Last In, First Out) memory automatically managed by the CPU",
          "characteristics": [
            "Fast allocation/deallocation (just moving stack pointer)",
            "Limited size (typically 1-8 MB depending on platform)",
            "Automatic cleanup when scope ends",
            "LIFO ordering - last allocated is first freed",
            "No fragmentation",
            "Cache-friendly due to sequential access"
          ],
          "use_cases": [
            "Local variables",
            "Function parameters",
            "Return addresses",
            "Small temporary data",
            "Frame-local allocations"
          ],
          "warnings": [
            "Avoid large arrays on stack (use heap instead)",
            "Stack overflow crashes are unrecoverable",
            "Stack size is platform-dependent"
          ]
        },
        "heap": {
          "description": "Manually managed memory pool for dynamic allocation",
          "characteristics": [
            "Flexible size (limited by system memory)",
            "Manual allocation/deallocation required",
            "Persists across function calls",
            "Can fragment over time",
            "Slower than stack due to bookkeeping",
            "Requires explicit cleanup (new/delete pairing)"
          ],
          "use_cases": [
            "Large data structures",
            "Objects with dynamic lifetime",
            "Arrays with runtime-determined size",
            "Long-lived data",
            "Objects that need to outlive function scope"
          ],
          "warnings": [
            "Always pair new with delete (new[] with delete[])",
            "Memory leaks accumulate and cause crashes",
            "Fragmentation reduces available memory",
            "Allocation/deallocation is relatively expensive"
          ]
        }
      },
      "alignment": {
        "description": "Memory alignment ensures data starts at addresses that are multiples of specific values, critical for performance and correctness",
        "importance": [
          "SIMD instructions require 16-byte or 32-byte alignment",
          "Cache lines are typically 64 bytes",
          "Misaligned access can cause crashes on some platforms",
          "Aligned data enables vectorization optimizations",
          "Reduces cache line splits for better performance"
        ],
        "common_alignments": {
          "1_byte": "char, bool",
          "4_bytes": "int, float",
          "8_bytes": "double, long long, pointers (64-bit)",
          "16_bytes": "SIMD vectors (__m128, Vec4)",
          "32_bytes": "AVX SIMD vectors (__m256)",
          "64_bytes": "Cache line alignment"
        },
        "implementation": "Use _aligned_malloc or alignas keyword in C++11+"
      },
      "fragmentation": {
        "description": "Fragmentation occurs when free memory is divided into small, non-contiguous blocks, preventing large allocations even when total free memory is sufficient",
        "types": {
          "external_fragmentation": "Free memory exists but is scattered in small chunks between allocated blocks",
          "internal_fragmentation": "Allocated blocks contain unused space due to alignment or allocation granularity"
        },
        "prevention_strategies": [
          "Use custom allocators designed for specific allocation patterns",
          "Employ object pooling for fixed-size objects",
          "Allocate from stack when possible",
          "Use arena/linear allocators for temporary data",
          "Implement compacting garbage collection (complex)",
          "Group allocations by lifetime"
        ]
      }
    },
    "allocator_types": {
      "stack_allocator": {
        "description": "Fast linear allocator that allocates from a pre-allocated buffer by simply incrementing a pointer. Deallocates all at once by resetting the pointer.",
        "characteristics": [
          "O(1) allocation - just pointer increment",
          "No per-allocation overhead",
          "No fragmentation",
          "Must deallocate in reverse order (or all at once)",
          "Perfect for per-frame temporary data",
          "Extremely cache-friendly due to linear access"
        ],
        "use_cases": [
          "Per-frame temporary allocations",
          "String formatting buffers",
          "Temporary calculation buffers",
          "Command buffers",
          "Any short-lived data with clear lifecycle"
        ],
        "implementation_details": {
          "structure": [
            "Pre-allocated buffer (typically multiple MB)",
            "Current top pointer (current allocation position)",
            "Buffer size",
            "Optional markers for nested scopes"
          ],
          "allocation": "Align requested size, check available space, return current top, increment top",
          "deallocation": "Reset top pointer to 0 (or to a marker for nested scopes)",
          "markers": "Can push/pop markers to create nested allocation scopes"
        },
        "code_reference": "See allocators.stack_allocator for implementation"
      },
      "pool_allocator": {
        "description": "Pre-allocates a fixed number of fixed-size objects and manages them via a free list. Extremely fast for repeated allocation/deallocation of same-sized objects.",
        "characteristics": [
          "O(1) allocation and deallocation",
          "Zero fragmentation (all blocks same size)",
          "Fixed memory footprint known at startup",
          "Cache-friendly when objects are accessed sequentially",
          "No allocation overhead during gameplay"
        ],
        "use_cases": [
          "Particles (typically thousands of same-sized objects)",
          "Game entities in entity pools",
          "Bullets/projectiles",
          "Audio voices",
          "Network packets",
          "Any frequently created/destroyed fixed-size objects"
        ],
        "implementation_details": {
          "structure": [
            "Array of N objects (pre-allocated)",
            "Free list (linked list of available objects)",
            "Each free block contains pointer to next free block"
          ],
          "allocation": "Pop from free list, return object pointer",
          "deallocation": "Push object back onto free list",
          "initialization": "All objects linked in free list at startup"
        },
        "optimization_notes": [
          "Consider storing indices instead of pointers for better cache locality",
          "Can use bitset for allocation tracking instead of free list",
          "Pool size should be carefully tuned based on profiling"
        ],
        "code_reference": "See allocators.pool_allocator for implementation"
      },
      "double_ended_stack_allocator": {
        "description": "Stack allocator that allows allocation from both ends of the buffer, useful for separating different lifetime categories",
        "use_cases": [
          "Low end: Per-frame temporary data",
          "High end: Level-lifetime data",
          "Prevents short-lived and long-lived data from fragmenting each other"
        ]
      },
      "free_list_allocator": {
        "description": "General-purpose allocator maintaining a list of free blocks of varying sizes. Similar to malloc but with game-specific optimizations.",
        "characteristics": [
          "Supports variable-size allocations",
          "Can coalesce adjacent free blocks",
          "Slower than specialized allocators",
          "Subject to fragmentation"
        ],
        "strategies": {
          "first_fit": "Use first block that's large enough (fast)",
          "best_fit": "Use smallest block that fits (reduces fragmentation)",
          "worst_fit": "Use largest block (leaves larger remaining blocks)"
        }
      }
    },
    "memory_patterns": {
      "handle_based_references": {
        "description": "Instead of storing raw pointers, use handles (typically 32 or 64-bit integers) that are looked up in a table. Prevents dangling pointers and enables better memory management.",
        "benefits": [
          "Dangling pointer detection (generation counter)",
          "Enables memory defragmentation/relocation",
          "Smaller than pointers on 64-bit systems (can use 32-bit handles)",
          "Can be serialized easily",
          "Provides level of indirection for lazy loading"
        ],
        "handle_structure": {
          "index": "Index into array of resources",
          "generation": "Incremented when resource is freed, detects stale handles",
          "type": "Optional type identifier for type safety"
        },
        "implementation": "See resource_management.handle_system for details"
      },
      "reference_counting": {
        "description": "Track number of references to a resource. Automatically deallocate when count reaches zero.",
        "types": {
          "intrusive": "Reference count is part of the object itself",
          "non_intrusive": "Reference count stored separately (like std::shared_ptr)"
        },
        "considerations": [
          "Circular references cause memory leaks",
          "Thread-safe counting requires atomics (performance cost)",
          "Can use weak references to break cycles"
        ]
      },
      "smart_pointers": {
        "description": "RAII wrappers around raw pointers that automatically manage lifetime",
        "types": {
          "unique_ptr": {
            "description": "Exclusive ownership, non-copyable",
            "use_when": "Single owner, transfer ownership with std::move",
            "overhead": "Zero (optimized away in release builds)"
          },
          "shared_ptr": {
            "description": "Shared ownership via reference counting",
            "use_when": "Multiple owners, unclear lifetime",
            "overhead": "Reference count (atomic), control block allocation",
            "warning": "Avoid in performance-critical code due to atomic operations"
          },
          "weak_ptr": {
            "description": "Non-owning reference to shared_ptr managed object",
            "use_when": "Breaking circular references, observing without owning",
            "usage": "Must lock() before use to get shared_ptr"
          }
        },
        "best_practices": [
          "Prefer unique_ptr by default",
          "Use shared_ptr only when truly needed",
          "Avoid raw new/delete in modern C++",
          "Use make_unique/make_shared for exception safety"
        ]
      }
    },
    "cache_optimization": {
      "description": "Modern CPUs are memory-bound; cache optimization is more important than algorithmic complexity for many operations",
      "cache_hierarchy": {
        "L1_cache": {
          "size": "32-64 KB per core",
          "latency": "~4 cycles",
          "description": "Fastest, smallest cache"
        },
        "L2_cache": {
          "size": "256-512 KB per core",
          "latency": "~12 cycles",
          "description": "Larger but slower"
        },
        "L3_cache": {
          "size": "8-32 MB shared",
          "latency": "~40 cycles",
          "description": "Shared between cores"
        },
        "main_memory": {
          "size": "Gigabytes",
          "latency": "~200 cycles",
          "description": "Orders of magnitude slower than L1"
        }
      },
      "cache_line": {
        "size": "64 bytes (typical)",
        "description": "Minimum unit of cache transfer. CPU loads entire cache lines, not individual bytes.",
        "implications": [
          "Sequential access is much faster than random access",
          "Accessing one byte of a struct loads the entire cache line",
          "False sharing: Two threads modifying different variables in same cache line causes slowdown"
        ]
      },
      "optimization_strategies": {
        "data_oriented_design": {
          "description": "Structure of Arrays (SoA) instead of Array of Structures (AoS)",
          "rationale": "Accessing only position data doesn't load unused rotation, scale, etc. into cache",
          "example": "Instead of array of structs with position/rotation/scale, have separate arrays for positions, rotations, scales"
        },
        "hot_cold_splitting": {
          "description": "Separate frequently accessed (hot) data from rarely accessed (cold) data",
          "example": "Keep entity active flags and positions together, move debug names and editor data elsewhere"
        },
        "prefetching": {
          "description": "Hint to CPU to load data into cache before it's needed",
          "techniques": [
            "Software prefetch intrinsics",
            "Process data in batches to enable hardware prefetcher",
            "Predictable access patterns help hardware prefetcher"
          ]
        },
        "avoid_pointer_chasing": {
          "description": "Dereferencing pointers stalls CPU waiting for memory",
          "solutions": [
            "Use array indices instead of pointers",
            "Flatten data structures",
            "Use contiguous arrays instead of linked lists"
          ]
        }
      }
    }
  },

  "parallelism_and_concurrency": {
    "description": "Modern games must leverage multi-core CPUs to achieve performance targets. Parallelism enables processing multiple tasks simultaneously, while concurrency manages multiple tasks making progress.",
    "fundamentals": {
      "concurrency_vs_parallelism": {
        "concurrency": "Multiple tasks making progress, not necessarily simultaneously (can be single core with time-slicing)",
        "parallelism": "Multiple tasks executing simultaneously on different cores",
        "in_games": "Both are important - parallelism for performance, concurrency for task management"
      },
      "types_of_parallelism": {
        "task_parallelism": {
          "description": "Different tasks running on different cores",
          "examples": [
            "Physics on one core, rendering on another, AI on a third",
            "Multiple enemy AI updates on different cores",
            "Async resource loading while game runs"
          ],
          "implementation": "Job/task systems"
        },
        "data_parallelism": {
          "description": "Same operation on multiple data elements simultaneously",
          "examples": [
            "Updating 1000 particle positions",
            "Transforming all vertices of a mesh",
            "Applying physics to many rigid bodies"
          ],
          "implementation": "SIMD, GPU compute shaders, parallel algorithms"
        }
      },
      "challenges": {
        "race_conditions": {
          "description": "Multiple threads accessing shared data with at least one write, outcome depends on timing",
          "solution": "Mutex locks, atomic operations, or lock-free data structures"
        },
        "deadlocks": {
          "description": "Two or more threads waiting for each other, none can proceed",
          "prevention": [
            "Always acquire locks in the same order",
            "Use try_lock with timeout",
            "Avoid nested locking when possible",
            "Use lock hierarchies"
          ]
        },
        "false_sharing": {
          "description": "Different threads modifying different variables that share a cache line, causing cache thrashing",
          "solution": "Pad data to separate cache lines (64-byte alignment)"
        }
      }
    },
    "job_system": {
      "description": "Modern game engines use job/task systems instead of raw threads. Jobs are small units of work executed by a thread pool.",
      "benefits": [
        "Better load balancing (work-stealing queues)",
        "Lower overhead than creating threads",
        "Automatic CPU utilization",
        "Dependencies express task relationships",
        "Easier to reason about than manual threading"
      ],
      "components": {
        "job": {
          "description": "Unit of work to be executed",
          "contains": [
            "Function pointer or lambda",
            "Optional data pointer",
            "Optional counter for completion tracking",
            "Optional parent job for dependencies"
          ]
        },
        "job_queue": {
          "description": "Queue of jobs waiting to be executed",
          "types": {
            "global_queue": "Shared by all worker threads",
            "per_thread_queue": "Each thread has its own queue, can steal from others"
          }
        },
        "worker_threads": {
          "description": "Pool of threads that execute jobs",
          "count": "Typically core_count - 1 (leave one for main thread)",
          "behavior": "Loop: pop job from queue, execute, repeat"
        },
        "counter": {
          "description": "Atomic integer used to track job completion",
          "usage": "Parent job can wait until counter reaches zero"
        }
      },
      "usage_pattern": {
        "create_jobs": "Define work as jobs with dependencies",
        "schedule": "Add jobs to queue",
        "wait": "Wait on counter or specific job completion",
        "completion": "Workers execute jobs, decrement counters"
      },
      "advanced_features": {
        "work_stealing": "Idle threads steal jobs from busy threads' queues",
        "job_priorities": "High-priority jobs execute before low-priority",
        "affinity": "Pin certain jobs to specific cores",
        "continuations": "Job scheduled when another completes"
      },
      "code_reference": "See parallelism.job_system for implementation"
    },
    "synchronization_primitives": {
      "mutex": {
        "description": "Mutual exclusion lock - only one thread can hold it at a time",
        "use_when": "Protecting shared data from concurrent modification",
        "types": {
          "std::mutex": "Basic mutex, non-recursive",
          "std::recursive_mutex": "Can be locked multiple times by same thread",
          "std::shared_mutex": "Reader-writer lock (multiple readers or one writer)"
        },
        "best_practices": [
          "Hold locks for minimum time possible",
          "Use std::lock_guard or std::unique_lock (RAII)",
          "Never lock and unlock manually unless absolutely necessary",
          "Avoid calling unknown code while holding lock"
        ]
      },
      "atomic": {
        "description": "Operations that complete without interruption, thread-safe without locks",
        "use_when": "Simple operations on integers, pointers, flags",
        "examples": [
          "std::atomic<int> counter",
          "std::atomic<bool> isRunning",
          "std::atomic<T*> pointer"
        ],
        "operations": [
          "load/store",
          "fetch_add/fetch_sub",
          "compare_exchange (CAS)",
          "exchange"
        ],
        "memory_ordering": {
          "relaxed": "No synchronization guarantees (fastest)",
          "acquire": "Subsequent reads/writes can't be reordered before this",
          "release": "Previous reads/writes can't be reordered after this",
          "seq_cst": "Sequential consistency (default, safest, slowest)"
        }
      },
      "condition_variable": {
        "description": "Allows threads to wait until a condition is met, avoiding busy-waiting",
        "use_when": "Thread needs to wait for work or event",
        "usage": "wait() with predicate, notify_one() or notify_all() to wake threads"
      },
      "semaphore": {
        "description": "Counting synchronization primitive, allows N threads to proceed",
        "use_when": "Limiting concurrent access to resource pool",
        "example": "Limiting number of concurrent file operations"
      }
    },
    "lock_free_programming": {
      "description": "Data structures and algorithms that avoid locks using atomic operations and careful memory ordering",
      "benefits": [
        "No deadlocks possible",
        "Better scalability (no lock contention)",
        "More responsive (no waiting for locks)"
      ],
      "challenges": [
        "Much harder to implement correctly",
        "Requires deep understanding of memory models",
        "Difficult to debug",
        "May not be faster for all workloads"
      ],
      "common_patterns": {
        "lock_free_queue": {
          "description": "Queue that can be accessed by multiple threads without locks",
          "types": {
            "single_producer_single_consumer": "Simplest, ring buffer with atomic read/write indices",
            "multiple_producer_single_consumer": "More complex, requires CAS operations",
            "multiple_producer_multiple_consumer": "Most complex, often uses CAS loops"
          }
        },
        "lock_free_stack": {
          "description": "Stack using CAS to atomically update head pointer",
          "aba_problem": "Head pointer changes to A->B->A, CAS succeeds incorrectly. Solution: tagged pointers with generation counter"
        }
      },
      "code_reference": "See parallelism.lock_free_structures for implementations"
    },
    "simd_vectorization": {
      "description": "Single Instruction Multiple Data - process multiple data elements with one instruction",
      "instruction_sets": {
        "SSE": "128-bit (4 floats or 2 doubles)",
        "AVX": "256-bit (8 floats or 4 doubles)",
        "AVX-512": "512-bit (16 floats or 8 doubles)",
        "NEON": "ARM SIMD instructions"
      },
      "use_cases": [
        "Vector math (Vec4, Matrix4x4 operations)",
        "Particle updates",
        "Physics calculations",
        "Image processing",
        "Audio processing"
      ],
      "considerations": [
        "Data must be aligned (16/32/64 bytes)",
        "Best when no branching in inner loop",
        "Can use intrinsics or rely on compiler auto-vectorization",
        "Gather/scatter operations are slower than sequential access"
      ]
    }
  },

  "math_systems": {
    "description": "3D math is the foundation of game engines, used for transformations, physics, rendering, and gameplay",
    "core_types": {
      "vector": {
        "description": "Represents direction and magnitude in space, or a point in space",
        "types": {
          "Vector2": {
            "components": ["x", "y"],
            "use_cases": ["2D positions", "texture coordinates", "screen coordinates"]
          },
          "Vector3": {
            "components": ["x", "y", "z"],
            "use_cases": ["3D positions", "directions", "velocities", "normals", "colors (RGB)"]
          },
          "Vector4": {
            "components": ["x", "y", "z", "w"],
            "use_cases": ["Homogeneous coordinates", "colors (RGBA)", "plane equations", "SIMD optimization (padding)"]
          }
        },
        "operations": {
          "addition": "Component-wise addition, represents translation",
          "subtraction": "Component-wise subtraction, vector between two points",
          "scalar_multiplication": "Scale magnitude by scalar value",
          "dot_product": {
            "formula": "a · b = ax*bx + ay*by + az*bz",
            "result": "Scalar",
            "uses": [
              "Angle between vectors: cos(θ) = (a · b) / (|a| * |b|)",
              "Projection: projection of a onto b = (a · b / |b|²) * b",
              "Perpendicularity test: dot product = 0 means perpendicular",
              "Forward/backward test: dot product > 0 means same direction"
            ]
          },
          "cross_product": {
            "formula": "a × b = (ay*bz - az*by, az*bx - ax*bz, ax*by - ay*bx)",
            "result": "Vector perpendicular to both inputs",
            "uses": [
              "Find surface normal from two edges",
              "Determine handedness of coordinate system",
              "Compute torque in physics",
              "Test if point is left/right of line"
            ],
            "note": "Only defined for 3D vectors, right-hand rule determines direction"
          },
          "normalization": {
            "formula": "v / |v|",
            "result": "Unit vector (length = 1) in same direction",
            "uses": ["Direction vectors", "surface normals", "before dot product for angle"]
          },
          "length": {
            "formula": "√(x² + y² + z²)",
            "optimization": "Use length_squared when only comparing lengths to avoid sqrt"
          }
        }
      },
      "quaternion": {
        "description": "4D complex number representing rotation, avoids gimbal lock and enables smooth interpolation",
        "components": {
          "x": "Imaginary i component",
          "y": "Imaginary j component",
          "z": "Imaginary k component",
          "w": "Real component"
        },
        "advantages_over_euler": [
          "No gimbal lock (Euler angles can lose degree of freedom)",
          "Smooth interpolation (SLERP)",
          "Smaller storage (4 floats vs 9 for matrix)",
          "Faster concatenation than matrices",
          "Numerically stable"
        ],
        "operations": {
          "multiplication": {
            "description": "Concatenates rotations (note: order matters, non-commutative)",
            "usage": "q1 * q2 = rotation by q2, then q1"
          },
          "conjugate": {
            "description": "Negates x,y,z components, represents inverse rotation",
            "formula": "q* = (-x, -y, -z, w)"
          },
          "slerp": {
            "description": "Spherical Linear Interpolation - smooth rotation interpolation",
            "usage": "Animating from one rotation to another",
            "formula": "slerp(q1, q2, t) = (sin((1-t)θ) * q1 + sin(tθ) * q2) / sin(θ)",
            "optimization": "Use nlerp (normalized lerp) when angle is small"
          },
          "to_matrix": "Convert quaternion to rotation matrix for rendering",
          "from_axis_angle": "Create quaternion from rotation axis and angle",
          "from_euler": "Create from pitch/yaw/roll (beware gimbal lock in source angles)"
        },
        "best_practices": [
          "Keep quaternions normalized (length = 1)",
          "Check for shortest path when interpolating (negate one if dot product < 0)",
          "Store rotations as quaternions, convert to matrix only when needed",
          "Use double-cover property: q and -q represent same rotation"
        ],
        "code_reference": "See math.quaternion_operations for SLERP implementation"
      },
      "matrix": {
        "description": "Rectangular array of numbers used for transformations, basis representation, and projections",
        "types": {
          "Matrix3x3": {
            "description": "3D rotation and scale (no translation)",
            "use_cases": ["Normal transformations", "rotation-only transforms", "inertia tensors"]
          },
          "Matrix4x4": {
            "description": "Full 3D transformation using homogeneous coordinates",
            "use_cases": ["Position, rotation, and scale", "view and projection matrices", "skeletal animation"]
          }
        },
        "transformations": {
          "translation": {
            "description": "Move object in space",
            "matrix": "Identity with translation vector in last column",
            "note": "Cannot be represented in 3x3 matrix, requires 4x4 homogeneous coordinates"
          },
          "rotation": {
            "description": "Rotate object around origin or axis",
            "representations": ["Euler angles (gimbal lock)", "Axis-angle", "Quaternion (preferred)"],
            "note": "Rotation matrices are orthonormal (rows and columns are unit vectors, perpendicular)"
          },
          "scale": {
            "description": "Change size along axes",
            "types": {
              "uniform_scale": "Same scale factor for all axes",
              "non_uniform_scale": "Different scale for each axis (can skew object)"
            }
          },
          "shear": {
            "description": "Skew object along axes",
            "use": "Rare in games, sometimes for special effects"
          }
        },
        "operations": {
          "multiplication": {
            "description": "Combine transformations (order matters!)",
            "rule": "Read right to left: TRS = translate, then rotate, then scale (but write scale * rotation * translation)",
            "cost": "O(n³) for n×n matrices, but 4x4 is fast enough (64 multiplications)"
          },
          "transpose": {
            "description": "Swap rows and columns",
            "for_rotation_matrix": "Transpose equals inverse (orthonormal property)"
          },
          "inverse": {
            "description": "Undo transformation",
            "use_cases": ["Convert world to local space", "undo camera transform"],
            "optimization": "For TRS matrices, can compute inverse efficiently using separate T, R, S"
          },
          "determinant": {
            "description": "Scalar representing volume scaling factor",
            "interpretation": [
              "det = 0: matrix is singular (not invertible, collapsed dimension)",
              "det > 0: preserves handedness",
              "det < 0: flips handedness (mirror)"
            ]
          }
        },
        "memory_layout": {
          "row_major": "Rows contiguous in memory, used by DirectX, most math libraries",
          "column_major": "Columns contiguous in memory, used by OpenGL",
          "note": "Must match GPU shader expectations"
        },
        "optimizations": [
          "Use SIMD for matrix multiplication (SSE, AVX)",
          "Cache frequently used matrices (view, projection)",
          "Decompose matrix for easier interpolation (separate T, R, S)",
          "Use dirty flags to avoid recalculating unchanged matrices"
        ]
      },
      "transform": {
        "description": "Represents position, rotation, and scale of an object in 3D space, typically in a hierarchy",
        "components": {
          "position": {
            "type": "Vector3",
            "description": "Location in space relative to parent"
          },
          "rotation": {
            "type": "Quaternion",
            "description": "Orientation relative to parent"
          },
          "scale": {
            "type": "Vector3",
            "description": "Size along each axis relative to parent"
          },
          "parent": {
            "type": "Transform*",
            "description": "Parent transform in hierarchy (null if root)"
          }
        },
        "hierarchies": {
          "description": "Transforms organized in parent-child relationships (scene graph)",
          "local_space": "Position/rotation/scale relative to parent",
          "world_space": "Absolute position/rotation/scale in world",
          "computation": "World = Parent_World * Local",
          "benefits": [
            "Moving parent moves all children",
            "Skeletal animation (bones are transforms)",
            "Organizational hierarchy for scene",
            "Easier to reason about relative movement"
          ],
          "implementation": "Recursively multiply parent matrices to get world matrix"
        },
        "dirty_flags": {
          "description": "Track when transform changes to avoid recalculating matrix every frame",
          "pattern": "Set dirty flag on change, recalculate matrix on first access, clear flag",
          "benefit": "Significant performance gain for static or infrequently moving objects"
        },
        "code_reference": "See math.transform_hierarchy for implementation"
      }
    },
    "coordinate_systems": {
      "handedness": {
        "right_handed": {
          "description": "OpenGL, mathematical convention: X right, Y up, Z toward viewer",
          "cross_product": "Right-hand rule: fingers curl from X to Y, thumb points Z"
        },
        "left_handed": {
          "description": "DirectX, Unity: X right, Y up, Z away from viewer",
          "cross_product": "Left-hand rule: fingers curl from X to Y, thumb points Z"
        },
        "note": "Choice affects matrix multiplication order and winding order for culling"
      },
      "spaces": {
        "local_space": {
          "description": "Object's own coordinate system, origin at object center",
          "use": "Modeling, defining object vertices"
        },
        "world_space": {
          "description": "Global coordinate system for entire scene",
          "use": "Physics, collision detection, global positioning"
        },
        "view_space": {
          "description": "Camera coordinate system, camera at origin looking down -Z",
          "use": "Lighting calculations, frustum culling",
          "transformation": "World matrix multiplied by view matrix"
        },
        "clip_space": {
          "description": "After projection, coordinates in [-1,1] (or [0,1] for Z in some APIs)",
          "use": "Hardware rasterization",
          "transformation": "View space multiplied by projection matrix"
        },
        "screen_space": {
          "description": "2D pixel coordinates on screen",
          "transformation": "Clip space mapped to viewport dimensions"
        }
      }
    },
    "geometric_primitives": {
      "ray": {
        "description": "Half-line starting at origin extending in direction",
        "representation": {
          "origin": "Vector3 start point",
          "direction": "Vector3 direction (should be normalized)",
          "equation": "P(t) = origin + t * direction, where t >= 0"
        },
        "uses": [
          "Ray casting (click to select object)",
          "Line of sight tests",
          "Bullet trajectories",
          "Procedural raymarching"
        ]
      },
      "plane": {
        "description": "Infinite flat 2D surface in 3D space",
        "representations": {
          "normal_distance": {
            "normal": "Vector3 perpendicular to plane (unit vector)",
            "distance": "float distance from origin to plane",
            "equation": "dot(normal, point) = distance"
          },
          "point_normal": {
            "point": "Any point on the plane",
            "normal": "Vector perpendicular to plane"
          },
          "three_points": "Define plane from three non-collinear points"
        },
        "uses": [
          "Frustum culling (6 planes define view frustum)",
          "Clipping",
          "Ground plane for characters",
          "Portal systems"
        ],
        "operations": {
          "point_distance": "Signed distance from point to plane: dot(normal, point) - distance",
          "ray_intersection": "Find t where ray intersects plane",
          "side_test": "Determine which side of plane a point is on"
        }
      },
      "aabb": {
        "description": "Axis-Aligned Bounding Box - box aligned with coordinate axes",
        "representation": {
          "min": "Vector3 minimum corner",
          "max": "Vector3 maximum corner"
        },
        "properties": [
          "Very fast intersection tests",
          "Not tight fit for rotated objects",
          "Easy to compute from vertices",
          "Cheap to enlarge or merge"
        ],
        "uses": [
          "Broad-phase collision detection",
          "Frustum culling",
          "Spatial partitioning (octrees, grids)",
          "Quick rejection tests"
        ],
        "operations": {
          "ray_intersection": "Check if ray hits box, return near and far t values",
          "box_intersection": "Check if two AABBs overlap",
          "contains_point": "Check if point is inside",
          "expand": "Grow box to contain point or another box"
        }
      },
      "sphere": {
        "description": "All points equidistant from center",
        "representation": {
          "center": "Vector3 center point",
          "radius": "float radius"
        },
        "properties": [
          "Simple intersection tests",
          "Rotation invariant",
          "Loose fit for most objects",
          "Useful for quick rejection"
        ],
        "uses": [
          "Simple collision detection",
          "Trigger volumes",
          "Audio attenuation ranges",
          "Particle system bounds"
        ]
      },
      "obb": {
        "description": "Oriented Bounding Box - box with arbitrary rotation",
        "representation": {
          "center": "Vector3 center",
          "extents": "Vector3 half-sizes along local axes",
          "rotation": "Quaternion or Matrix3x3 orientation"
        },
        "properties": [
          "Tighter fit than AABB for rotated objects",
          "More expensive intersection tests",
          "Good balance of accuracy and performance"
        ]
      },
      "frustum": {
        "description": "Truncated pyramid shape defining camera view volume",
        "representation": "Six planes: near, far, left, right, top, bottom",
        "uses": [
          "View frustum culling (don't render objects outside view)",
          "Shadow frustum for shadow mapping",
          "Audio occlusion"
        ],
        "extraction": "Extract plane equations from view-projection matrix",
        "testing": {
          "point": "Check point against all 6 planes",
          "sphere": "Check sphere center distance to each plane",
          "aabb": "Check box corners against planes"
        }
      }
    }
  },

  "rendering_systems": {
    "description": "Rendering is the process of generating images from 3D scene data. Modern game engines use complex rendering pipelines with multiple passes and advanced techniques.",
    "rendering_pipeline": {
      "overview": "Series of stages transforming 3D geometry into 2D pixels on screen",
      "stages": {
        "application_stage": {
          "description": "CPU-side work preparing data for GPU",
          "tasks": [
            "Culling (frustum, occlusion, small feature)",
            "LOD selection (Level of Detail)",
            "Sorting (opaque front-to-back, transparent back-to-front)",
            "Animation update",
            "Building draw calls",
            "Updating GPU buffers"
          ],
          "output": "List of draw calls with associated state"
        },
        "geometry_stage": {
          "description": "GPU vertex processing",
          "vertex_shader": {
            "description": "Transform vertices from local space to clip space",
            "tasks": [
              "Matrix transformations (model, view, projection)",
              "Vertex lighting calculations",
              "Normal transformation",
              "Texture coordinate generation",
              "Skinning (skeletal animation)"
            ],
            "input": "Per-vertex attributes (position, normal, UV, etc.)",
            "output": "Transformed vertices in clip space"
          },
          "tessellation": {
            "description": "Optional: subdivide geometry on GPU",
            "use_cases": ["Displacement mapping", "dynamic LOD", "smooth surfaces"]
          },
          "geometry_shader": {
            "description": "Optional: generate or modify primitives",
            "use_cases": ["Particle generation", "shadow volume extrusion", "instancing"]
          },
          "clipping": {
            "description": "Remove geometry outside view frustum",
            "result": "Clip triangles against frustum planes"
          },
          "perspective_divide": {
            "description": "Divide x,y,z by w to get normalized device coordinates",
            "result": "Coordinates in [-1,1] range"
          }
        },
        "rasterization_stage": {
          "description": "Convert triangles to pixels (fragments)",
          "tasks": [
            "Determine which pixels triangle covers",
            "Interpolate vertex attributes across triangle",
            "Generate fragments for pixel shader"
          ],
          "culling": "Discard back-facing triangles (based on winding order)"
        },
        "pixel_stage": {
          "description": "Determine final pixel colors",
          "pixel_shader": {
            "description": "Execute per fragment to determine color",
            "tasks": [
              "Texture sampling",
              "Lighting calculations",
              "Normal mapping",
              "Material properties (PBR)",
              "Fog",
              "Alpha testing"
            ],
            "input": "Interpolated vertex attributes",
            "output": "Color and depth"
          },
          "output_merger": {
            "description": "Combine pixel shader output with framebuffer",
            "depth_test": "Discard fragments behind existing geometry",
            "stencil_test": "Mask rendering to certain pixels",
            "blending": "Combine colors (for transparency, additive effects)"
          }
        }
      }
    },
    "culling_techniques": {
      "frustum_culling": {
        "description": "Don't render objects outside camera view",
        "method": "Test object bounds against frustum planes",
        "result": "Can reject 70-90% of objects in typical scenes"
      },
      "occlusion_culling": {
        "description": "Don't render objects blocked by other geometry",
        "techniques": {
          "software_occlusion": "Rasterize occluders to depth buffer on CPU",
          "hardware_occlusion_queries": "GPU tests if object visible",
          "portal_culling": "For indoor scenes, only render through visible portals",
          "pvs": "Potentially Visible Set - precomputed visibility"
        }
      },
      "backface_culling": {
        "description": "Don't rasterize triangles facing away from camera",
        "method": "Check if triangle normal points toward or away from camera",
        "savings": "Roughly 50% of triangles (for closed meshes)"
      },
      "small_feature_culling": {
        "description": "Don't render objects smaller than certain pixel threshold",
        "method": "Project bounding box to screen space, check size",
        "benefit": "Avoid processing geometry that won't be visible"
      }
    },
    "level_of_detail": {
      "description": "Use simpler geometry for distant objects to maintain performance",
      "discrete_lod": {
        "description": "Switch between pre-made models at specific distances",
        "implementation": "Artist creates multiple versions, engine selects based on distance",
        "considerations": ["Popping artifacts at transitions", "Memory overhead for multiple models"]
      },
      "continuous_lod": {
        "description": "Gradually simplify geometry based on distance",
        "techniques": ["Progressive meshes", "Geomorphing", "Tessellation adjustment"]
      },
      "imposters": {
        "description": "Replace distant objects with billboards (textured quads)",
        "use_cases": ["Trees in forest", "buildings on skyline", "crowds"]
      }
    },
    "lighting_models": {
      "forward_rendering": {
        "description": "Traditional approach: render each object, apply all lights affecting it",
        "advantages": ["Simple", "MSAA support", "Transparent objects easy"],
        "disadvantages": ["Cost increases with lights (O(objects * lights))", "Light limit per object"]
      },
      "deferred_rendering": {
        "description": "Separate geometry and lighting passes using G-buffer",
        "g_buffer": {
          "description": "Multiple render targets storing surface properties",
          "typical_contents": ["World position", "Normal", "Albedo (color)", "Metalness/Roughness", "Specular"],
          "size": "Full screen resolution, multiple buffers (often 100+ MB)"
        },
        "advantages": ["Many lights with constant cost", "Light culling", "Consistent lighting"],
        "disadvantages": ["Memory bandwidth", "No MSAA (need workarounds)", "Transparent objects difficult"],
        "process": [
          "Geometry pass: Render all objects, write to G-buffer",
          "Lighting pass: For each light, read G-buffer, compute lighting, accumulate in framebuffer",
          "Combine pass: Apply lighting to albedo"
        ]
      },
      "tile_based_rendering": {
        "description": "Divide screen into tiles, determine which lights affect each tile",
        "benefit": "Reduce overdraw in lighting pass",
        "implementation": "Build per-tile light lists, shade tiles in parallel"
      },
      "pbr": {
        "description": "Physically-Based Rendering - lighting model based on physics",
        "principles": [
          "Energy conservation: Reflected light never more than incoming",
          "Microfacet theory: Surfaces are microscopic mirrors",
          "Fresnel effect: Reflectance increases at grazing angles"
        ],
        "parameters": {
          "albedo": "Base color (diffuse)",
          "metalness": "How metallic surface is (0-1)",
          "roughness": "How rough surface is (0=mirror, 1=diffuse)",
          "normal": "Surface normal (from normal map)",
          "ao": "Ambient occlusion (shadows in crevices)"
        },
        "benefits": ["Consistent lighting", "Looks good in all lighting", "Artist-friendly"],
        "implementation": "Cook-Torrance BRDF or GGX"
      }
    },
    "shadow_techniques": {
      "shadow_maps": {
        "description": "Render scene from light's perspective into depth texture, then test if pixel is in shadow",
        "process": [
          "Render depth from light's view to shadow map texture",
          "Render scene normally, transform pixel to light space",
          "Compare pixel depth with shadow map depth",
          "If pixel is farther from light, it's in shadow"
        ],
        "issues": {
          "shadow_acne": "Self-shadowing artifacts due to depth precision",
          "peter_panning": "Shadows disconnect from objects due to bias",
          "aliasing": "Blocky shadow edges due to limited resolution"
        },
        "solutions": {
          "bias": "Add small offset to depth comparison",
          "pcf": "Percentage Closer Filtering - sample multiple shadow map texels",
          "cascaded_shadow_maps": "Multiple shadow maps at different resolutions for different distances",
          "variance_shadow_maps": "Store depth statistics for better filtering"
        }
      },
      "shadow_volumes": {
        "description": "Extrude geometry away from light to create shadow volume",
        "method": "Use stencil buffer to count front/back face hits",
        "advantages": ["Sharp shadows", "Works well with deferred"],
        "disadvantages": ["Fill rate intensive", "Requires closed geometry"]
      }
    },
    "post_processing": {
      "description": "Effects applied to final rendered image",
      "common_effects": {
        "bloom": {
          "description": "Bright areas glow and bleed into surroundings",
          "method": "Threshold bright pixels, blur, add back to image"
        },
        "tone_mapping": {
          "description": "Map HDR colors to displayable LDR range",
          "operators": ["Reinhard", "ACES", "Uncharted 2"]
        },
        "color_grading": {
          "description": "Adjust colors for mood/style",
          "method": "LUT (lookup table) transformation"
        },
        "depth_of_field": {
          "description": "Blur based on distance from focal plane",
          "method": "Use depth buffer to determine blur amount, apply bokeh"
        },
        "motion_blur": {
          "description": "Blur based on object/camera movement",
          "types": ["Full-screen (camera)", "Per-object (velocity buffer)"]
        },
        "ambient_occlusion": {
          "description": "Darken areas where ambient light is blocked (crevices)",
          "techniques": ["SSAO (Screen Space)", "HBAO+", "Ray-traced"]
        },
        "anti_aliasing": {
          "types": {
            "msaa": "Multisample Anti-Aliasing - supersample at edges",
            "fxaa": "Fast Approximate - post-process edge detection and blur",
            "taa": "Temporal Anti-Aliasing - accumulate samples across frames",
            "smaa": "Subpixel Morphological - pattern detection"
          }
        }
      }
    },
    "modern_techniques": {
      "ray_tracing": {
        "description": "Trace rays through scene to simulate light transport physically",
        "uses": ["Reflections", "Refractions", "Shadows", "Global illumination"],
        "implementations": ["DXR (DirectX Raytracing)", "Vulkan RT", "Software"],
        "hybrid_rendering": "Combine rasterization with selective ray tracing for performance"
      },
      "virtual_texturing": {
        "description": "Stream only visible texture data to GPU",
        "benefit": "Support massive texture sets without loading everything",
        "implementation": "Mega-texture divided into tiles, loaded on demand"
      },
      "compute_shaders": {
        "description": "General-purpose GPU computation",
        "uses": ["Particles", "Post-processing", "Culling", "Skinning", "Physics"]
      }
    }
  },

  "animation_systems": {
    "description": "Animation brings characters and objects to life. Modern games use skeletal animation with blend trees and state machines for complex behaviors.",
    "skeletal_animation": {
      "description": "Mesh deformed by hierarchy of bones, each bone a transform in hierarchy",
      "components": {
        "skeleton": {
          "description": "Hierarchy of bones (joints)",
          "bone": {
            "components": ["Name", "Parent index", "Local transform", "Inverse bind pose matrix"],
            "bind_pose": "Rest position of skeleton when mesh was modeled"
          }
        },
        "mesh": {
          "description": "Vertices with bone weights",
          "vertex_format": {
            "position": "Base position in bind pose",
            "normal": "Base normal",
            "bone_indices": "Up to 4 bones influencing this vertex",
            "bone_weights": "Weight for each bone (sum to 1.0)"
          }
        },
        "animation_clip": {
          "description": "Sequence of keyframes specifying bone transforms over time",
          "keyframe": {
            "time": "Time in animation",
            "transforms": "Array of bone transforms (position, rotation, scale)"
          },
          "interpolation": "Lerp (position/scale) or Slerp (rotation) between keyframes"
        }
      },
      "skinning_process": {
        "steps": [
          "Sample animation at current time to get bone transforms (interpolate keyframes)",
          "Compute world-space bone matrices by multiplying through hierarchy",
          "For each vertex: transform by weighted sum of bone matrices",
          "Vertex_final = sum(weight[i] * BoneMatrix[i] * InverseBindPose[i] * Vertex_bind)"
        ],
        "gpu_skinning": "Perform skinning in vertex shader for performance",
        "cpu_skinning": "Rare, used for physics ragdoll blending or special cases"
      },
      "inverse_bind_pose": {
        "description": "Matrix that transforms vertex from bind pose to bone's local space",
        "purpose": "Allows vertex to move relative to bone during animation",
        "computation": "Inverse of bone's world matrix in bind pose"
      }
    },
    "animation_blending": {
      "description": "Combine multiple animations to create complex behaviors",
      "lerp_blend": {
        "description": "Linear interpolation between two animations",
        "formula": "Anim_result = (1-t) * Anim_A + t * Anim_B",
        "use_cases": ["Walk to run transition", "Idle variations", "Directional movement"]
      },
      "additive_blend": {
        "description": "Add animation on top of base animation",
        "formula": "Anim_result = Anim_base + weight * (Anim_additive - Anim_reference)",
        "use_cases": ["Aiming while moving", "Breathing while idle", "Looking around"]
      },
      "partial_skeleton_blending": {
        "description": "Blend different animations for different parts of skeleton",
        "example": "Upper body plays shooting animation while lower body plays running",
        "implementation": "Define bone masks for different regions"
      },
      "blend_trees": {
        "description": "Hierarchical structure of blend nodes",
        "node_types": {
          "clip_node": "Leaf node, plays single animation",
          "lerp_node": "Blends two children by parameter (e.g., speed)",
          "additive_node": "Adds child animation to base",
          "blend_2d": "Blends based on 2 parameters (e.g., direction x/y)"
        },
        "parameters": "Runtime values controlling blend (speed, direction, etc.)",
        "evaluation": "Traverse tree, blend at each node, output final pose"
      }
    },
    "animation_state_machines": {
      "description": "Manage transitions between different animations based on game state",
      "components": {
        "states": {
          "description": "Each state plays animation or blend tree",
          "examples": ["Idle", "Walk", "Run", "Jump", "Attack"]
        },
        "transitions": {
          "description": "Rules for moving between states",
          "components": ["Source state", "Target state", "Condition", "Blend time"],
          "conditions": "Boolean expressions (e.g., speed > 1.0, jump button pressed)"
        },
        "entry_state": "Default state when starting",
        "any_state": "Special state that can transition to any other state"
      },
      "execution": {
        "steps": [
          "Evaluate current state's animation/blend tree",
          "Check transition conditions",
          "If condition met, begin transition (crossfade to new state)",
          "During transition, blend between old and new states",
          "Once transition complete, fully in new state"
        ]
      },
      "hierarchical_state_machines": {
        "description": "States can contain sub-state machines",
        "example": "Combat state contains sub-states for different attack types"
      }
    },
    "inverse_kinematics": {
      "description": "Position bones to reach target, rather than playing animation directly",
      "use_cases": [
        "Foot placement on uneven terrain",
        "Hand reaching for object (grabbing)",
        "Looking at target (head/eye tracking)",
        "Weapon aiming"
      ],
      "techniques": {
        "two_bone_ik": {
          "description": "Solve for two-bone chain (e.g., upper arm, forearm to reach hand target)",
          "method": "Analytical solution using law of cosines",
          "parameters": ["End effector target position", "Pole vector (elbow direction)"]
        },
        "fabrik": {
          "description": "Forward And Backward Reaching Inverse Kinematics",
          "method": "Iteratively pull bones toward target from end, then from start",
          "advantages": ["Handles any chain length", "Fast convergence", "No singularities"]
        },
        "ccd": {
          "description": "Cyclic Coordinate Descent",
          "method": "Rotate each bone toward target, starting from end",
          "advantages": "Simple", 
          "disadvantages": "Can create unnatural poses"
        }
      },
      "constraints": {
        "description": "Limit joint movement to realistic ranges",
        "types": ["Hinge (elbow, knee)", "Ball-and-socket (shoulder, hip)", "Fixed"]
      }
    },
    "animation_compression": {
      "description": "Reduce memory and streaming bandwidth for animation data",
      "techniques": {
        "keyframe_reduction": {
          "description": "Remove keyframes that can be interpolated",
          "method": "If interpolated value is within error threshold, remove keyframe"
        },
        "quantization": {
          "description": "Reduce precision of values",
          "example": "Store rotation as 16-bit quaternion instead of 32-bit floats"
        },
        "curve_fitting": {
          "description": "Represent animation as mathematical curve",
          "types": ["Bezier", "Hermite", "B-splines"]
        },
        "delta_compression": {
          "description": "Store difference from bind pose or previous frame",
          "benefit": "Values are smaller, compress better"
        }
      }
    },
    "advanced_techniques": {
      "motion_matching": {
        "description": "Search database of animation clips for best match to desired motion",
        "process": [
          "Define features (position, velocity, facing direction)",
          "Each frame, search database for best matching pose",
          "Transition to that pose/clip",
          "Creates very natural, responsive animation"
        ],
        "requirements": "Large animation database, fast search (k-d trees)"
      },
      "procedural_animation": {
        "description": "Generate animation algorithmically instead of from artist-created clips",
        "techniques": ["Inverse kinematics", "Procedural walk cycles", "Ragdoll physics", "Spring systems"],
        "examples": ["Tentacles", "Cloth simulation", "Soft body deformation"]
      },
      "animation_retargeting": {
        "description": "Apply animation from one skeleton to different skeleton",
        "challenges": "Different proportions, bone count",
        "method": "Map bones between skeletons, adjust for size differences"
      }
    }
  },

  "physics_systems": {
    "description": "Physics simulation adds realism through collision, forces, and rigid body dynamics. Modern games balance realism with performance and gameplay feel.",
    "collision_detection": {
      "description": "Determine when and where objects intersect",
      "phases": {
        "broad_phase": {
          "description": "Quickly identify pairs of objects that might be colliding",
          "goal": "Reduce O(n²) naive check to manageable number",
          "techniques": {
            "spatial_hashing": {
              "description": "Divide world into grid, hash objects to cells",
              "benefit": "O(1) lookup, easy to implement",
              "drawback": "Objects can span multiple cells"
            },
            "sweep_and_prune": {
              "description": "Sort objects along axis, maintain sorted list, find overlaps",
              "benefit": "Efficient for many objects",
              "complexity": "O(n log n) + O(k) where k is overlap count"
            },
            "bounding_volume_hierarchy": {
              "description": "Tree of bounding volumes (AABBs or spheres)",
              "benefit": "O(log n) query, good for static geometry",
              "types": ["BVH (binary tree)", "Octree", "k-d tree"]
            },
            "dynamic_aabb_tree": {
              "description": "BVH optimized for moving objects",
              "benefit": "Incremental updates without rebuilding tree"
            }
          },
          "output": "List of potentially colliding pairs"
        },
        "narrow_phase": {
          "description": "Precise collision detection for potentially colliding pairs",
          "primitives": {
            "sphere_sphere": {
              "test": "Distance between centers < sum of radii",
              "cost": "Very cheap"
            },
            "aabb_aabb": {
              "test": "Check overlap on all three axes",
              "cost": "Very cheap (6 comparisons)"
            },
            "sphere_aabb": {
              "test": "Find closest point on box to sphere, check distance",
              "cost": "Cheap"
            },
            "obb_obb": {
              "test": "Separating Axis Theorem (SAT)",
              "cost": "Moderate (15 axis tests)"
            },
            "capsule_capsule": {
              "test": "Closest point between line segments",
              "cost": "Moderate"
            }
          },
          "mesh_collision": {
            "triangle_triangle": "Expensive, avoid for real-time",
            "triangle_sphere": "Reasonable for simple meshes",
            "convex_mesh": "Use GJK (Gilbert-Johnson-Keerthi) algorithm",
            "concave_mesh": "Decompose into convex hulls or use BVH"
          }
        }
      },
      "continuous_collision_detection": {
        "description": "Detect collisions for fast-moving objects (prevent tunneling)",
        "problem": "Object can pass through thin wall between frames",
        "solutions": {
          "swept_volumes": "Test collision along movement path",
          "conservative_advancement": "Incrementally advance time until collision",
          "time_of_impact": "Solve for exact time of first collision"
        }
      },
      "contact_generation": {
        "description": "Find collision points, normals, and penetration depth",
        "manifold": {
          "description": "Set of contact points between two objects",
          "typical_size": "1-4 points",
          "uses": "Stable stacking, realistic contact"
        }
      }
    },
    "rigid_body_dynamics": {
      "description": "Simulate motion of solid objects under forces",
      "properties": {
        "mass": "Resistance to linear acceleration",
        "inertia_tensor": "3x3 matrix representing resistance to angular acceleration",
        "position": "Center of mass location",
        "orientation": "Rotation (quaternion)",
        "linear_velocity": "Rate of position change",
        "angular_velocity": "Rate of rotation change"
      },
      "integration": {
        "description": "Update position and velocity based on forces",
        "methods": {
          "explicit_euler": {
            "formula": "v += a * dt; x += v * dt",
            "properties": "Simple, fast, unstable for stiff systems"
          },
          "semi_implicit_euler": {
            "formula": "v += a * dt; x += v * dt (using new v)",
            "properties": "Stable, energy-preserving, industry standard"
          },
          "runge_kutta_4": {
            "formula": "Four substeps for accuracy",
            "properties": "Very accurate, expensive, rarely needed in games"
          },
          "verlet": {
            "formula": "x_new = 2*x_current - x_old + a * dt²",
            "properties": "Time-reversible, good for constraints, no explicit velocity"
          }
        }
      },
      "forces": {
        "gravity": "Constant downward force (F = m * g)",
        "drag": "Opposes motion (F = -k * v)",
        "spring": "Pulls toward rest length (F = -k * (x - rest_length))",
        "impulse": "Instantaneous force (velocity change)",
        "torque": "Rotational force (angular acceleration)"
      },
      "constraints": {
        "description": "Restrict object movement or relation to other objects",
        "types": {
          "fixed": "Lock object in place",
          "hinge": "Allow rotation around one axis (door)",
          "ball_socket": "Allow rotation around all axes (ragdoll shoulder)",
          "slider": "Allow translation along one axis",
          "distance": "Maintain constant distance between two points (rope)"
        },
        "solving": {
          "penalty_method": "Apply force to correct constraint violation",
          "projection": "Directly adjust positions to satisfy constraint",
          "lagrange_multipliers": "Solve system of equations for constraint forces"
        }
      }
    },
    "collision_response": {
      "description": "Determine how objects react to collisions",
      "impulse_based": {
        "description": "Apply instantaneous velocity changes",
        "formula": "j = -(1 + restitution) * vrel · n / (1/m1 + 1/m2)",
        "restitution": "Bounciness (0 = no bounce, 1 = elastic bounce)",
        "friction": "Prevents sliding (tangential impulse)"
      },
      "constraint_based": {
        "description": "Treat collisions as constraints, solve iteratively",
        "methods": ["PGS (Projected Gauss-Seidel)", "Sequential Impulse"],
        "benefit": "Stable stacking, accurate resting contact"
      },
      "penalty_method": {
        "description": "Apply forces proportional to penetration depth",
        "drawback": "Can be unstable, requires careful tuning"
      }
    },
    "performance_optimization": {
      "sleeping": {
        "description": "Objects at rest stop being simulated",
        "criteria": "Velocity below threshold for several frames",
        "wake_up": "Collision with moving object or external force"
      },
      "island_detection": {
        "description": "Group connected objects, solve independently",
        "benefit": "Parallel processing of separate groups"
      },
      "fixed_timestep": {
        "description": "Run physics at constant rate (typically 60 Hz)",
        "benefit": "Deterministic, stable",
        "implementation": "Accumulate frame time, run multiple physics steps if needed"
      },
      "simplified_collision_shapes": {
        "description": "Use simple primitives instead of mesh",
        "benefit": "Much faster collision detection"
      }
    },
    "special_systems": {
      "ragdoll": {
        "description": "Skeleton driven by physics instead of animation",
        "use_cases": ["Death animations", "Physical reactions", "Unconscious characters"],
        "implementation": "Bones are rigid bodies, joints are constraints",
        "blending": "Can blend from animation to ragdoll (and back)"
      },
      "soft_body": {
        "description": "Deformable objects",
        "techniques": ["Mass-spring systems", "Finite element method", "Position-based dynamics"],
        "use_cases": ["Cloth", "Jelly", "Soft characters"]
      },
      "destructible_objects": {
        "description": "Objects that break apart",
        "approaches": ["Pre-fractured pieces", "Runtime voronoi fracturing", "Progressive damage"],
        "challenges": "Performance, visual quality, gameplay"
      },
      "vehicles": {
        "description": "Specialized physics for cars, planes, etc.",
        "components": ["Suspension (springs)", "Wheels (friction, rolling)", "Aerodynamics", "Engine/transmission"],
        "implementations": ["Ray-cast wheels", "Rigid body with forces", "Custom solvers"]
      }
    }
  },

  "entity_component_system": {
    "description": "ECS is a data-oriented architecture pattern separating data (Components) from behavior (Systems), organized by Entities. Enables high performance and flexibility.",
    "motivation": {
      "problems_with_oop": [
        "Deep inheritance hierarchies are rigid and hard to maintain",
        "Multiple inheritance leads to diamond problem",
        "Adding new behavior requires modifying class hierarchy",
        "Poor cache locality (virtual functions, scattered data)",
        "Difficulty composing behaviors"
      ],
      "ecs_solutions": [
        "Composition over inheritance - build entities from components",
        "Data-oriented design - components stored contiguously",
        "Flexibility - add/remove components at runtime",
        "Performance - systems process components in tight loops"
      ]
    },
    "core_concepts": {
      "entity": {
        "description": "Unique identifier (typically uint32 or uint64) representing a game object",
        "properties": [
          "Just an ID - no behavior or data",
          "Tags set of components",
          "Can be created/destroyed dynamically",
          "May include generation counter to detect stale references"
        ],
        "example": "Entity player = 42; Entity enemy = 43;"
      },
      "component": {
        "description": "Pure data structure attached to entities, no behavior",
        "characteristics": [
          "POD (Plain Old Data) preferred",
          "No member functions (maybe constructors)",
          "Stored in contiguous arrays (one array per component type)",
          "Entity ID maps to component array index"
        ],
        "examples": {
          "TransformComponent": {
            "position": "Vector3",
            "rotation": "Quaternion",
            "scale": "Vector3"
          },
          "PhysicsComponent": {
            "velocity": "Vector3",
            "mass": "float",
            "friction": "float"
          },
          "RenderComponent": {
            "mesh": "MeshHandle",
            "material": "MaterialHandle",
            "visible": "bool"
          },
          "HealthComponent": {
            "current": "float",
            "maximum": "float"
          }
        }
      },
      "system": {
        "description": "Logic that operates on entities with specific component combinations",
        "characteristics": [
          "Queries for entities with required components",
          "Processes components in tight loops",
          "Can run in parallel if no data dependencies",
          "Owns no data, just operates on components"
        ],
        "examples": {
          "PhysicsSystem": "Updates entities with Transform + Physics components",
          "RenderSystem": "Renders entities with Transform + Render components",
          "AISystem": "Updates entities with AI + Transform components",
          "DamageSystem": "Applies damage to entities with Health component"
        }
      }
    },
    "archetypes": {
      "description": "Entities with same component types are stored together (archetype)",
      "benefits": [
        "Maximum cache locality - components for all entities of same type contiguous",
        "Fast iteration - no need to check component presence",
        "Memory efficient - no gaps in arrays"
      ],
      "tradeoffs": [
        "Adding/removing component moves entity to different archetype (expensive)",
        "Need to maintain mappings between entities and archetypes"
      ],
      "structure": {
        "archetype": "Combination of component types (e.g., Transform+Physics+Render)",
        "chunks": "Fixed-size memory blocks holding entity data",
        "component_arrays": "Separate arrays for each component type in archetype"
      }
    },
    "implementation_patterns": {
      "component_storage": {
        "sparse_set": {
          "description": "Dense array for components, sparse array for entity->component mapping",
          "benefits": ["Fast iteration (dense)", "Fast lookup (sparse)", "Fast add/remove"],
          "structure": {
            "dense": "Array of components",
            "sparse": "Array of indices (indexed by entity ID)",
            "entity_to_index": "sparse[entity_id] gives index in dense array"
          }
        },
        "archetype_based": {
          "description": "Group entities by component combination (used by Unity DOTS, Bevy)",
          "benefits": ["Maximum cache locality", "Parallel iteration"],
          "structure": "Separate storage for each unique component combination"
        },
        "hierarchical": {
          "description": "Hybrid approach (used by Flecs)",
          "benefits": "Balance between flexibility and performance"
        }
      },
      "queries": {
        "description": "Select entities based on component requirements",
        "syntax_examples": {
          "all": "Query.All<Transform, Physics>() - has both",
          "any": "Query.Any<Weapon, Armor>() - has at least one",
          "none": "Query.None<Dead>() - does not have",
          "optional": "Query.Optional<AI>() - may or may not have"
        },
        "caching": "Results cached and updated incrementally when entities change",
        "iteration": "Iterate over matched entities, accessing components directly"
      },
      "change_detection": {
        "description": "Track which components were modified",
        "methods": {
          "dirty_flags": "Mark component as modified, systems check flag",
          "versioning": "Increment version number on write, systems store last processed version",
          "change_events": "Emit event when component modified"
        },
        "benefit": "Avoid processing unchanged data"
      }
    },
    "parallelization": {
      "independent_systems": {
        "description": "Systems with non-overlapping component access run in parallel",
        "example": "RenderSystem (reads Transform) and PhysicsSystem (writes Transform) cannot run in parallel",
        "scheduling": "Build dependency graph, run independent systems in parallel"
      },
      "chunked_iteration": {
        "description": "Divide component arrays into chunks, process chunks in parallel",
        "benefit": "Single system processes different entities on different threads",
        "implementation": "Job system with one job per chunk"
      },
      "read_write_tracking": {
        "description": "Track which systems read/write which components",
        "purpose": "Determine which systems can run in parallel",
        "implementation": "Annotate systems with read/write requirements at compile time"
      }
    },
    "advanced_features": {
      "hierarchies": {
        "description": "Parent-child relationships between entities",
        "implementation": ["Parent component with entity ID", "Children component with array of entity IDs"],
        "use_cases": ["Scene graph transforms", "Skeletal hierarchies", "Weapon attached to character"]
      },
      "prefabs": {
        "description": "Templates for creating entities",
        "implementation": "Serialize component set, instantiate multiple entities from same template",
        "benefits": "Reusability, consistency, easy to modify template"
      },
      "entity_lifecycle": {
        "creation": "Allocate entity ID, add components",
        "modification": "Add/remove components (may change archetype)",
        "destruction": "Remove from all component arrays, return ID to pool"
      }
    },
    "comparison_with_oop": {
      "oop_approach": {
        "structure": "GameObject base class, derived classes (Enemy, Player, Bullet)",
        "composition": "GameObject contains components, may use inheritance",
        "problems": ["Deep hierarchies", "tight coupling", "poor cache locality", "virtual function overhead"]
      },
      "ecs_approach": {
        "structure": "Entities are IDs, components are data, systems are logic",
        "composition": "Entity is set of components, add/remove freely",
        "benefits": ["Flexible composition", "cache-friendly", "parallelizable", "data-oriented"]
      },
      "when_to_use": {
        "ecs": "Large numbers of entities, need performance, complex compositions",
        "oop": "Small number of objects, complex behaviors, rapid prototyping"
      }
    }
  },

  "design_patterns": {
    "description": "Reusable solutions to common problems in game development",
    "creational_patterns": {
      "singleton": {
        "description": "Ensure only one instance of a class exists, provide global access",
        "use_cases": ["Engine managers", "Resource caches", "Input managers", "Audio systems"],
        "implementation": {
          "private_constructor": "Prevent direct instantiation",
          "static_instance": "Hold the single instance",
          "static_getter": "Provide access (GetInstance())"
        },
        "warnings": [
          "Global state is difficult to test",
          "Tight coupling to singleton",
          "Order of initialization issues",
          "Threading concerns",
          "Use sparingly - consider dependency injection"
        ],
        "alternatives": ["Dependency injection", "Service locator", "Static classes"]
      },
      "factory": {
        "description": "Create objects without specifying exact class",
        "use_cases": ["Spawning enemies", "Creating UI elements", "Instantiating prefabs"],
        "variations": {
          "simple_factory": "Function that creates objects based on parameter",
          "factory_method": "Virtual method in base class, overridden to create different types",
          "abstract_factory": "Interface for creating families of related objects"
        },
        "example": "EnemyFactory.CreateEnemy(EnemyType.Zombie) returns Zombie object"
      },
      "object_pool": {
        "description": "Reuse objects instead of creating/destroying",
        "motivation": [
          "Avoid allocation cost during gameplay",
          "Prevent memory fragmentation",
          "Consistent performance (no GC pauses)"
        ],
        "use_cases": ["Particles", "Bullets", "Audio voices", "Network packets"],
        "implementation": {
          "available_list": "Objects ready to be used",
          "in_use_list": "Objects currently active",
          "acquire": "Pop from available, add to in_use",
          "release": "Remove from in_use, reset state, add to available"
        },
        "sizing": "Profile to determine peak usage, add buffer"
      },
      "prototype": {
        "description": "Create new objects by cloning prototype",
        "use_cases": ["Prefab instantiation", "Entity duplication"],
        "benefit": "Avoid complex initialization, easy to create variants"
      }
    },
    "structural_patterns": {
      "flyweight": {
        "description": "Share immutable data between many objects to save memory",
        "use_cases": ["Texture sharing", "Material instances", "Particle properties"],
        "pattern": {
          "intrinsic_state": "Shared, immutable (texture, mesh)",
          "extrinsic_state": "Unique per instance (position, color)"
        },
        "example": "10000 trees share one TreeMesh, each has unique position"
      },
      "adapter": {
        "description": "Wrap interface to match expected interface",
        "use_cases": ["Integrate third-party libraries", "Platform abstraction layers"],
        "example": "PhysicsEngineAdapter wraps Bullet/PhysX with common interface"
      },
      "facade": {
        "description": "Simplified interface to complex subsystem",
        "use_cases": ["High-level engine API", "Subsystem interfaces"],
        "example": "AudioSystem.PlaySound() hides complex mixing, streaming, spatialization"
      },
      "proxy": {
        "description": "Placeholder for another object, controls access",
        "types": {
          "virtual_proxy": "Lazy initialization - create real object only when needed",
          "protection_proxy": "Access control - check permissions before delegating",
          "smart_reference": "Reference counting, logging, etc."
        },
        "use_cases": ["Lazy loading resources", "Network object replication"]
      }
    },
    "behavioral_patterns": {
      "observer": {
        "description": "Objects subscribe to events, notified when event occurs",
        "use_cases": ["UI updates", "Achievement system", "Game events", "Networking"],
        "implementation": {
          "subject": "Maintains list of observers, notifies on change",
          "observer": "Interface with Update() method",
          "concrete_observers": "Implement Update() to react to changes"
        },
        "variations": {
          "event_system": "Centralized, loosely coupled (see Event System)",
          "delegates": "C# style, type-safe callbacks",
          "signals_slots": "Qt style"
        },
        "warnings": [
          "Memory leaks if observers not unregistered",
          "Order of notification matters",
          "Potential infinite loops (observer triggers another event)"
        ]
      },
      "command": {
        "description": "Encapsulate request as object, enabling undo/redo, queuing, logging",
        "use_cases": ["Input buffering", "Undo/redo", "Replays", "Networking (send commands)", "Macro recording"],
        "structure": {
          "command_interface": "Execute(), Undo() methods",
          "concrete_commands": "Implement specific actions (MoveCommand, AttackCommand)",
          "invoker": "Stores and executes commands",
          "receiver": "Object that performs the action"
        },
        "example": "MoveCommand stores old position, Execute() moves object, Undo() restores position"
      },
      "state": {
        "description": "Object changes behavior when internal state changes",
        "use_cases": ["AI states", "Animation states", "Game states (menu, playing, paused)"],
        "implementation": {
          "context": "Object whose behavior varies by state",
          "state_interface": "Handle(), Update(), Enter(), Exit()",
          "concrete_states": "Implement behavior for each state",
          "transitions": "States can trigger state changes"
        },
        "example": "Enemy AI states: Idle, Patrol, Chase, Attack - different behavior in each"
      },
      "strategy": {
        "description": "Define family of algorithms, make them interchangeable",
        "use_cases": ["AI behaviors", "Pathfinding algorithms", "Rendering techniques"],
        "structure": {
          "context": "Uses a strategy",
          "strategy_interface": "Common interface for all algorithms",
          "concrete_strategies": "Different implementations of algorithm"
        },
        "example": "SortingAlgorithm interface with QuickSort, MergeSort implementations"
      },
      "template_method": {
        "description": "Define skeleton of algorithm, let subclasses override specific steps",
        "use_cases": ["Game loop variations", "AI behavior templates"],
        "structure": {
          "abstract_class": "Template method calls primitive operations",
          "primitive_operations": "Virtual methods subclasses override"
        }
      },
      "visitor": {
        "description": "Separate algorithm from object structure, add new operations without modifying classes",
        "use_cases": ["Scene graph traversal", "Serialization", "Collision dispatch"],
        "implementation": "Double dispatch - object accepts visitor, visitor visits object"
      }
    },
    "game_specific_patterns": {
      "game_loop": {
        "description": "Core loop running continuously while game is active",
        "structure": {
          "input": "Process user input",
          "update": "Update game state",
          "render": "Draw frame"
        },
        "timing": {
          "fixed_timestep": "Update at constant rate (e.g., 60 Hz)",
          "variable_timestep": "Update based on elapsed time",
          "semi_fixed": "Cap maximum timestep, subdivide large steps"
        },
        "considerations": [
          "Decouple update rate from render rate",
          "Frame rate independence",
          "Catch-up for slow frames"
        ]
      },
      "double_buffer": {
        "description": "Use two buffers - read from one while writing to the other",
        "use_cases": ["Rendering (front/back buffer)", "Physics state", "Game state updates"],
        "benefit": "Avoid reading partially-updated state, eliminate tearing"
      },
      "dirty_flag": {
        "description": "Mark data as 'dirty' when changed, recalculate only when accessed and dirty",
        "use_cases": ["Transform matrices", "Bounding volumes", "Derived values"],
        "pattern": {
          "cache": "Cached calculated value",
          "dirty_flag": "Boolean indicating if cache is invalid",
          "on_change": "Set dirty flag",
          "on_access": "If dirty, recalculate and clear flag"
        },
        "benefit": "Avoid redundant calculations"
      },
      "spatial_partition": {
        "description": "Divide space into regions for efficient queries",
        "types": {
          "grid": "Uniform cells, simple, good for uniform distribution",
          "quadtree": "Recursive 2D subdivision",
          "octree": "Recursive 3D subdivision",
          "bsp_tree": "Binary space partition, good for static geometry",
          "bvh": "Bounding volume hierarchy"
        },
        "use_cases": ["Collision detection", "Visibility", "Pathfinding", "LOD"]
      },
      "data_locality": {
        "description": "Organize data for cache efficiency",
        "patterns": {
          "soa": "Structure of Arrays - separate arrays for each field",
          "aos": "Array of Structures - struct has all fields",
          "hot_cold": "Separate frequently and rarely accessed data"
        }
      }
    }
  },

  "ai_systems": {
    "description": "AI brings non-player characters and game systems to life with decision making, pathfinding, and behavioral systems",
    "finite_state_machines": {
      "description": "AI uses states with transitions to model behavior",
      "components": {
        "states": "Discrete behaviors (Idle, Patrol, Chase, Attack, Flee)",
        "transitions": "Conditions for moving between states",
        "state_logic": "Update() called each frame while in state"
      },
      "advantages": ["Simple to understand", "Easy to debug", "Designer-friendly"],
      "disadvantages": ["State explosion", "Difficult to reuse", "Hard to add nuance"],
      "implementation": "See gameplay_systems.state_machine for code",
      "enhancements": {
        "hierarchical_fsm": "States can contain sub-FSMs",
        "stack_fsm": "Push/pop states instead of hard transitions"
      }
    },
    "behavior_trees": {
      "description": "Hierarchical structure of nodes representing decisions and actions, highly modular and reusable",
      "node_types": {
        "action": {
          "description": "Leaf node that performs action (move, attack, wait)",
          "returns": "Running, Success, or Failure"
        },
        "condition": {
          "description": "Test condition (health < 50, player nearby)",
          "returns": "Success (true) or Failure (false)"
        },
        "sequence": {
          "description": "Execute children in order, fail if any fails",
          "use": "AND logic - all must succeed",
          "example": "Sequence: IsPlayerNearby, HasAmmo, Shoot"
        },
        "selector": {
          "description": "Try children in order until one succeeds",
          "use": "OR logic - first success wins",
          "example": "Selector: Attack, Chase, Patrol"
        },
        "parallel": {
          "description": "Execute multiple children simultaneously",
          "policies": ["Success when any succeeds", "Success when all succeed"],
          "use": "Concurrent behaviors (move while shooting)"
        },
        "decorator": {
          "description": "Modify child behavior",
          "types": ["Inverter (flip result)", "Repeater (repeat N times)", "Succeeder (always succeed)", "Until fail"]
        }
      },
      "execution": {
        "tick": "Called every frame (or less frequently), traverses tree, returns status",
        "evaluation": "Top-down traversal, parent decides which children to execute",
        "state": "Nodes can store state (current child index, loop count, etc.)"
      },
      "advantages": ["Modular", "Reusable", "Visual editing", "Easy to balance"],
      "disadvantages": ["Overhead", "Can be complex", "Designer learning curve"],
      "implementation": "See gameplay_systems.behavior_tree for code"
    },
    "utility_ai": {
      "description": "Score actions based on multiple factors, choose highest score",
      "process": {
        "considerations": "Factors affecting score (health, distance to target, ammo, etc.)",
        "scoring": "Combine considerations into overall score for each action",
        "selection": "Choose action with highest score"
      },
      "curves": "Map input (0-1) to score (0-1) with curves (linear, polynomial, sigmoid)",
      "advantages": ["Flexible", "Emergent behavior", "Easy to tweak", "Good for complex decisions"],
      "disadvantages": ["Harder to debug", "Can feel 'gamey' if not tuned well", "Performance (evaluating many actions)"],
      "use_cases": ["Strategy games", "Complex AI decisions", "Companion behavior"]
    },
    "goal_oriented_action_planning": {
      "description": "AI defines goal, planner finds sequence of actions to achieve it",
      "components": {
        "world_state": "Set of boolean facts (HasWeapon=true, EnemyAlive=true)",
        "goal": "Desired world state (EnemyAlive=false)",
        "actions": {
          "preconditions": "World state required to execute action",
          "effects": "How action changes world state",
          "cost": "Numeric cost of action"
        }
      },
      "planning": "Use A* to find lowest-cost sequence of actions transforming current state to goal state",
      "advantages": ["Flexible", "Emergent sequences", "Easy to add new actions"],
      "disadvantages": ["Complex", "Planning cost", "May produce unexpected sequences"],
      "use_cases": ["F.E.A.R. (influential implementation)", "Complex problem solving AI"]
    },
    "pathfinding": {
      "description": "Find path from start to goal avoiding obstacles",
      "algorithms": {
        "a_star": {
          "description": "Best-first search using cost + heuristic",
          "heuristic": "Estimate of remaining distance (Euclidean, Manhattan)",
          "properties": "Optimal (finds shortest path), complete, can be expensive",
          "optimization": "Jump Point Search (JPS) for grid-based"
        },
        "dijkstra": {
          "description": "Explore in order of cost, no heuristic",
          "use": "When heuristic unavailable, or finding paths to multiple goals"
        },
        "bfs": {
          "description": "Breadth-First Search - unweighted shortest path",
          "use": "Uniform cost graph, simpler than A*"
        },
        "hierarchical_pathfinding": {
          "description": "Plan at high level, then refine",
          "example": "Plan between rooms, then within room",
          "benefit": "Faster for long paths"
        },
        "navmesh": {
          "description": "Represent walkable area as polygon mesh",
          "benefits": ["More natural paths than grid", "Efficient", "Handles slopes"],
          "path_finding": "A* on navmesh nodes (polygons), string pull for final path"
        },
        "flow_fields": {
          "description": "Precompute direction field pointing toward goal",
          "use": "Many agents to same goal (RTS)",
          "benefit": "O(1) lookup per agent after O(n) precomputation"
        }
      },
      "path_smoothing": {
        "description": "Improve path quality after initial pathfinding",
        "techniques": ["String pulling", "Spline fitting", "Iterative corner cutting"],
        "benefit": "More natural movement"
      },
      "dynamic_avoidance": {
        "description": "Avoid moving obstacles and other agents",
        "techniques": {
          "local_avoidance": "Adjust velocity to avoid collisions",
          "rvo": "Reciprocal Velocity Obstacles - each agent adjusts",
          "social_forces": "Agents repel/attract based on social rules"
        }
      }
    },
    "steering_behaviors": {
      "description": "Low-level movement behaviors that can be combined",
      "basic_behaviors": {
        "seek": "Move toward target",
        "flee": "Move away from target",
        "arrive": "Seek but slow down near target",
        "pursue": "Predict target's future position, seek that",
        "evade": "Predict target's future position, flee that",
        "wander": "Random walk for exploration"
      },
      "group_behaviors": {
        "separation": "Avoid crowding neighbors",
        "alignment": "Match velocity with neighbors",
        "cohesion": "Move toward center of neighbors",
        "flocking": "Combine separation, alignment, cohesion"
      },
      "complex_behaviors": {
        "obstacle_avoidance": "Detect and steer around obstacles",
        "wall_following": "Stay near wall",
        "path_following": "Follow predefined path"
      },
      "combination": "Weighted sum or priority-based arbitration of behaviors"
    },
    "perception_systems": {
      "description": "AI needs to perceive environment",
      "vision": {
        "line_of_sight": "Ray cast from AI to target, check for obstruction",
        "field_of_view": "Check if target in view cone (angle and distance)",
        "occlusion": "Hidden objects not perceived"
      },
      "hearing": {
        "sound_events": "Broadcast sound with position and type",
        "listener": "AI subscribes to sounds within range",
        "investigation": "AI moves to investigate sound source"
      },
      "memory": {
        "last_known_position": "Remember where target was last seen",
        "search_behavior": "Check last known position, expand search"
      },
      "communication": {
        "blackboard": "Shared memory for AI squad",
        "alerts": "AI notifies others of threats",
        "coordination": "Plan together (flanking, suppression)"
      }
    }
  },

  "event_systems": {
    "description": "Decoupled communication between game systems using message passing",
    "motivation": [
      "Reduce coupling between systems",
      "Add/remove listeners without modifying sender",
      "Enable scripting and data-driven behavior",
      "Simplify debugging (log all events)"
    ],
    "implementation_approaches": {
      "immediate_dispatch": {
        "description": "Execute listeners synchronously when event is sent",
        "pros": ["Simple", "No queuing overhead", "Immediate response"],
        "cons": ["Can cause reentrancy", "Stack overflow risk", "Order matters"]
      },
      "queued_dispatch": {
        "description": "Add events to queue, process later in specific phase",
        "pros": ["Controlled timing", "Thread-safe", "Batch processing"],
        "cons": ["Delayed response", "Memory for queue"]
      },
      "hybrid": {
        "description": "Some events immediate, some queued",
        "use": "Critical events immediate, others queued"
      }
    },
    "event_structure": {
      "type": "Identifies event (enum or string)",
      "timestamp": "When event occurred",
      "sender": "Entity that sent event (optional)",
      "data": "Event-specific payload"
    },
    "patterns": {
      "subscribe_unsubscribe": {
        "description": "Listeners register interest in event types",
        "api": ["Subscribe(eventType, callback)", "Unsubscribe(eventType, callback)"],
        "storage": "Map from event type to list of callbacks"
      },
      "event_handlers": {
        "description": "Objects implement handler interface",
        "interface": "OnEvent(Event* event)",
        "benefit": "Type-safe, no function pointers"
      },
      "delegates": {
        "description": "C# style type-safe callbacks",
        "benefit": "Compiler-enforced signatures"
      }
    },
    "threading_considerations": {
      "thread_safe_queue": "Use lock or lock-free queue for cross-thread events",
      "dispatch_thread": "Process events on specific thread (e.g., main thread)",
      "locks": "Protect listener list if modified during dispatch"
    },
    "examples": [
      "OnPlayerDeath - update UI, trigger respawn, play sound",
      "OnCollision - apply damage, play effect, notify physics",
      "OnScoreChanged - update UI, check achievements",
      "OnDoorOpened - trigger ambient sound, update navmesh"
    ],
    "code_reference": "See gameplay_systems.event_system for implementation"
  },

  "scripting_integration": {
    "description": "Embed scripting languages for rapid iteration and modding",
    "languages": {
      "lua": {
        "pros": ["Small", "fast", "easy to embed", "widely used"],
        "cons": ["Dynamically typed", "no multithreading"],
        "use_cases": ["Game logic", "AI", "quests", "UI"]
      },
      "python": {
        "pros": ["Easy to learn", "rich libraries", "popular"],
        "cons": ["Slower than Lua", "GIL limits threading", "larger"],
        "use_cases": ["Tools", "editors", "pipelines"]
      },
      "c_sharp": {
        "pros": ["Performance", "type safety", "tooling (Visual Studio)"],
        "cons": ["Requires runtime (Mono, .NET)", "memory management"],
        "use_cases": ["Unity scripts", "high-level gameplay"]
      }
    },
    "binding_approaches": {
      "manual_binding": {
        "description": "Manually write C functions to expose to script",
        "pro": "Full control",
        "con": "Tedious, error-prone"
      },
      "automatic_binding": {
        "description": "Generate bindings from C++ headers",
        "tools": ["SWIG", "Pybind11", "Embeddable Lua"],
        "pro": "Less work",
        "con": "Not all C++ constructs supported"
      }
    },
    "exposing_engine_to_scripts": {
      "c_api_layer": "Provide C-compatible functions for script bindings",
      "object_handles": "Pass opaque handles to scripts, not raw pointers",
      "callbacks": "Scripts register callbacks for engine events"
    },
    "hot_reloading": {
      "description": "Reload scripts without restarting game",
      "implementation": ["Detect file changes", "Reload script", "Preserve state or reinitialize"],
      "benefit": "Rapid iteration"
    }
  },

  "resource_management": {
    "description": "Efficiently load, manage, and unload game assets (textures, models, sounds, etc.)",
    "resource_lifecycle": {
      "loading": {
        "description": "Read asset from disk, parse, create runtime representation",
        "stages": ["Read bytes", "Decompress", "Parse format", "Upload to GPU (if applicable)", "Create handle"],
        "async_loading": {
          "motivation": "Avoid blocking main thread (maintain frame rate)",
          "implementation": "Load on background thread, queue GPU upload for main thread",
          "challenges": "Thread safety, managing loading queue, prioritization"
        }
      },
      "usage": {
        "description": "Reference counting or handle-based access",
        "reference_counting": "Increment on acquire, decrement on release, unload at zero",
        "handles": "Indirect reference, manager maintains lifetime"
      },
      "unloading": {
        "description": "Free memory when resource no longer needed",
        "strategies": ["Manual (explicit unload)", "Reference counted (unload at zero)", "LRU cache (unload least recently used)"]
      }
    },
    "resource_types": {
      "textures": {
        "formats": ["PNG", "JPEG", "DDS", "KTX"],
        "compression": ["BC1-BC7 (desktop)", "ASTC (mobile)", "ETC2 (mobile)"],
        "mipmaps": "Precompute smaller versions for distant objects",
        "streaming": "Load high-res mips only when needed"
      },
      "meshes": {
        "formats": ["FBX", "OBJ", "GLTF", "Custom"],
        "optimization": ["Vertex cache optimization", "Strip/list conversion", "LOD generation"],
        "storage": "Vertex buffer + index buffer"
      },
      "audio": {
        "formats": ["WAV", "OGG", "MP3"],
        "streaming": "Load chunks on demand for music",
        "memory": "Keep small sounds in memory, stream large"
      },
      "animations": {
        "formats": ["FBX", "Custom"],
        "compression": "Keyframe reduction, quantization, curve fitting"
      }
    },
    "streaming": {
      "description": "Load resources on demand as player explores",
      "motivation": "Support worlds larger than memory",
      "implementation": {
        "divide_world": "Split into chunks/zones",
        "distance_based": "Load nearby chunks, unload distant",
        "portal_based": "Load rooms visible through portals",
        "trigger_volumes": "Load when player enters volume"
      },
      "challenges": ["Latency (visible pop-in)", "I/O bandwidth", "Memory budget"],
      "solutions": ["Aggressive prediction", "Fast decompression", "Visible feedback (loading screens)"]
    },
    "asset_pipeline": {
      "description": "Tools that transform source assets to runtime formats",
      "stages": {
        "import": "Read source format (PSD, FBX, etc.)",
        "process": "Optimize, compress, generate LODs, mipmaps",
        "export": "Write runtime format (optimized for loading)",
        "metadata": "Store asset properties, dependencies"
      },
      "automation": "Watch source folder, rebuild on changes",
      "benefits": ["Runtime optimization", "Platform-specific formats", "Automated optimization"]
    },
    "handle_system": {
      "description": "Indirect reference to resources, enables management",
      "handle_structure": {
        "index": "Index into resource array",
        "generation": "Detect stale handles",
        "type": "Optional type tag"
      },
      "benefits": ["Detect use-after-free", "Enable defragmentation", "Smaller than pointers (32-bit)"],
      "code_reference": "See memory_management.handle_based_references"
    }
  },

  "audio_systems": {
    "description": "Audio adds immersion through sound effects, music, and ambience",
    "sound_types": {
      "sfx": {
        "description": "Short sound effects (gunshot, footstep, UI click)",
        "storage": "Loaded into memory",
        "format": "Compressed (OGG, MP3) or uncompressed (WAV)"
      },
      "music": {
        "description": "Long background music tracks",
        "storage": "Streamed from disk",
        "implementation": "Read chunks into buffer, submit to audio device"
      },
      "ambience": {
        "description": "Environmental sounds (wind, rain, crowd)",
        "implementation": "Looping sounds, possibly layered"
      },
      "voice": {
        "description": "Dialogue, narration",
        "format": "Compressed, possibly localized (multiple languages)"
      }
    },
    "3d_audio": {
      "spatialization": {
        "description": "Position sounds in 3D space relative to listener",
        "implementation": ["Calculate direction to source", "Apply volume/panning", "Distance attenuation"],
        "apis": ["OpenAL", "FMOD", "Wwise", "Platform APIs (XAudio2, OpenSL)"]
      },
      "hrtf": {
        "description": "Head-Related Transfer Function - binaural audio",
        "benefit": "Realistic 3D positioning in headphones"
      },
      "occlusion_obstruction": {
        "occlusion": "Direct path blocked (muffled, quiet)",
        "obstruction": "Partial blocking",
        "implementation": "Ray cast from listener to source, apply filter"
      },
      "reverb": {
        "description": "Simulate environment acoustics",
        "implementation": "Convolution with impulse response or algorithmic reverb",
        "zones": "Different reverb settings per area (cave, cathedral)"
      }
    },
    "voice_management": {
      "description": "Limited simultaneous sounds (hardware/software limit)",
      "virtualization": "When limit reached, stop least important sound",
      "priority": "Assign priority (critical=dialogue, low=ambient)",
      "culling": "Don't play sounds too far away or too quiet"
    },
    "mixing": {
      "description": "Combine multiple sounds into final output",
      "channels": "Group sounds (SFX, music, dialogue) for volume control",
      "ducking": "Reduce music volume when dialogue plays",
      "compression": "Limit dynamic range (prevent clipping, balance quiet/loud)"
    },
    "middleware": {
      "description": "Third-party audio engines",
      "examples": {
        "fmod": "Popular, feature-rich, cross-platform",
        "wwise": "AAA standard, powerful, complex",
        "openal": "Open-source, simple 3D audio"
      },
      "benefits": ["Designer tools", "Advanced features", "Platform support"]
    }
  },

  "networking": {
    "description": "Multiplayer games require network communication to sync state between players",
    "architectures": {
      "client_server": {
        "description": "Authoritative server, clients send input and receive state updates",
        "server": {
          "responsibilities": ["Game logic", "Physics", "AI", "Validation", "Broadcasting state"],
          "authority": "Server is source of truth, prevents cheating"
        },
        "client": {
          "responsibilities": ["Render", "Input", "Prediction", "Interpolation"],
          "thin_client": "Just render, server does everything (high latency)",
          "thick_client": "Predict locally, correct when server disagrees"
        },
        "advantages": ["Anti-cheat", "Authoritative", "Easier logic"],
        "disadvantages": ["Server cost", "Single point of failure", "Latency affects gameplay"]
      },
      "peer_to_peer": {
        "description": "All clients connect directly, no central server",
        "variations": {
          "full_mesh": "Every client connects to every other (N² connections)",
          "relay": "Host client acts as server for others"
        },
        "advantages": ["No server cost", "Lower latency between peers"],
        "disadvantages": ["Cheating easier", "Synchronization complex", "NAT traversal"]
      },
      "hybrid": {
        "description": "Server for matchmaking/rooms, P2P for gameplay",
        "example": "Fighting games (low player count, latency-sensitive)"
      }
    },
    "protocols": {
      "tcp": {
        "description": "Reliable, ordered byte stream",
        "characteristics": ["Guaranteed delivery", "In-order", "Retransmission on packet loss", "Higher latency", "Head-of-line blocking"],
        "use_cases": ["Login", "Chat", "File transfer", "Turn-based games"]
      },
      "udp": {
        "description": "Unreliable, unordered datagram",
        "characteristics": ["No delivery guarantee", "No ordering", "Fast", "Low overhead"],
        "use_cases": ["Real-time gameplay", "Position updates", "Shooter games", "Racing games"],
        "reliability_layer": "Build custom reliability on top (ACKs, sequence numbers, retransmission)"
      },
      "custom_reliability": {
        "description": "Selective reliability - mark critical packets for ACK/retransmission",
        "example": "Movement updates unreliable, weapon fire reliable"
      }
    },
    "state_synchronization": {
      "full_state": {
        "description": "Send complete game state every update",
        "pro": "Simple, resilient to packet loss",
        "con": "High bandwidth"
      },
      "delta_compression": {
        "description": "Send only changed values since last ACK'd state",
        "pro": "Lower bandwidth",
        "con": "More complex, requires ACKs"
      },
      "snapshot_interpolation": {
        "description": "Server sends snapshots at fixed rate, client interpolates between them",
        "benefit": "Smooth motion despite jittery packets",
        "implementation": "Store recent snapshots, render 100-200ms in past"
      }
    },
    "client_side_prediction": {
      "description": "Client simulates player actions immediately, server corrects if needed",
      "motivation": "Waiting for server confirmation adds latency",
      "implementation": {
        "apply_input_immediately": "Predict movement",
        "server_confirmation": "Server sends result of input",
        "reconciliation": "If mismatch, rewind and replay"
      },
      "challenges": ["Prediction errors (jitter on correction)", "Replicating physics deterministically"]
    },
    "lag_compensation": {
      "description": "Account for network latency in hit detection",
      "techniques": {
        "lag_compensation": "Rewind world to when client fired, check hit there",
        "favor_the_shooter": "If client sees hit, count it (even if target moved)",
        "hitbox_prediction": "Predict where target will be"
      }
    },
    "packet_structure": {
      "header": {
        "sequence_number": "Unique packet ID, detect duplicates/order",
        "ack": "Last sequence received (for reliability)",
        "ack_bitfield": "Which recent packets received (for reliability)",
        "timestamp": "When sent (for latency calculation)"
      },
      "payload": {
        "message_type": "What kind of data (input, state, event)",
        "data": "Serialized game data"
      }
    },
    "serialization": {
      "description": "Convert game data to/from byte stream",
      "approaches": {
        "manual": "Write each field to buffer",
        "reflection": "Automatically serialize based on type metadata",
        "generated": "Code generator from IDL (Interface Definition Language)"
      },
      "compression": {
        "quantization": "Reduce precision (e.g., position to cm, not mm)",
        "range_encoding": "Store as fraction of known range",
        "bit_packing": "Pack booleans into single byte"
      }
    },
    "matchmaking": {
      "description": "Find appropriate players for a match",
      "elo_mmr": "Skill rating to match similar players",
      "latency": "Prefer geographically close players",
      "backfill": "Add players to partially-filled matches",
      "parties": "Keep friends together"
    }
  },

  "engine_patterns": {
    "subsystem_manager": {
      "description": "Manages initialization, update, and shutdown of engine subsystems in correct order",
      "implementation": "Array of ISubsystem pointers, iterate to initialize/update/shutdown",
      "code_example": "See architecture_overview.engine_subsystem_pattern"
    },
    "game_loop_patterns": {
      "fixed_timestep": {
        "description": "Physics and gameplay run at fixed rate, rendering runs as fast as possible",
        "implementation": "Accumulate frame time, run fixed updates until caught up",
        "benefit": "Deterministic simulation, consistent physics",
        "code_example": "while(accumulator >= dt) { update(dt); accumulator -= dt; }"
      },
      "variable_timestep": {
        "description": "Update based on actual elapsed time",
        "benefit": "Simpler",
        "problem": "Unstable physics, frame-rate dependent gameplay"
      }
    }
  },

  "allocators": {
    "stack_allocator": {
      "code_example": "See memory_management.allocator_types.stack_allocator for implementation"
    },
    "pool_allocator": {
      "code_example": "See memory_management.allocator_types.pool_allocator for implementation"
    }
  },

  "parallelism": {
    "job_system": {
      "code_example": "See parallelism_and_concurrency.job_system for implementation"
    },
    "lock_free_structures": {
      "code_example": "See parallelism_and_concurrency.lock_free_programming for implementations"
    }
  },

  "math": {
    "quaternion_operations": {
      "code_example": "See math_systems.core_types.quaternion.operations.slerp for SLERP implementation"
    },
    "transform_hierarchy": {
      "code_example": "See math_systems.core_types.transform for implementation"
    }
  },

  "gameplay_systems": {
    "state_machine": {
      "description": "Manage game states with transitions",
      "code_example": "See file line 1237-1269 in first source"
    },
    "behavior_tree": {
      "description": "Hierarchical AI decision making",
      "code_example": "See file line 1271-1335 in first source"
    },
    "event_system": {
      "description": "Decouple systems via message passing",
      "implementation_details": "See event_systems section for comprehensive details"
    }
  },

  "performance_patterns": {
    "cache_optimization": {
      "soa_vs_aos": {
        "description": "Structure of Arrays vs Array of Structures",
        "code_example": "See file line 1343-1370 in first source"
      }
    },
    "object_pooling": {
      "description": "Reuse objects to avoid allocation",
      "code_example": "See file line 1372-1406 in first source"
    },
    "dirty_flags": {
      "description": "Avoid redundant calculations",
      "code_example": "See file line 1408-1433 in first source"
    }
  },

  "best_practices": {
    "memory_management": [
      "Always pair new with delete (new[] with delete[])",
      "Check for nullptr before dereferencing",
      "Use smart pointers to avoid memory leaks",
      "Prefer stack allocation for small, short-lived data",
      "Use custom allocators for performance-critical code",
      "Profile memory usage and identify leaks early"
    ],
    "performance": [
      "Profile before optimizing - measure, don't guess",
      "Data locality matters more than algorithmic complexity",
      "Prefer cache-friendly data layouts (SoA over AoS)",
      "Use object pooling for frequently created/destroyed objects",
      "Minimize allocations during gameplay",
      "Batch similar operations together",
      "Use dirty flags to avoid redundant calculations"
    ],
    "architecture": [
      "Separate data from behavior (consider ECS)",
      "Use loose coupling via interfaces and events",
      "Data-driven design for flexibility",
      "Layer architecture - higher layers depend on lower only",
      "Fail fast with assertions in debug builds",
      "Keep subsystems independent and testable"
    ],
    "multithreading": [
      "Always use RAII wrappers for locks (lock_guard, unique_lock)",
      "Acquire locks in consistent order to avoid deadlocks",
      "Minimize time holding locks",
      "Use atomic operations for simple thread-safe access",
      "Prefer lock-free structures when possible (but be careful)",
      "Consider task-based parallelism (job system) over raw threads"
    ],
    "code_quality": [
      "Use meaningful variable and function names",
      "Comment WHY, not WHAT (code shows what)",
      "Keep functions small and focused (single responsibility)",
      "Prefer const correctness",
      "Use enum class instead of plain enum",
      "Leverage modern C++ features (auto, lambdas, smart pointers)",
      "Write unit tests for critical systems"
    ],
    "debugging": [
      "Use assertions liberally in debug builds",
      "Add logging at key decision points",
      "Implement debug visualization (physics shapes, AI paths, etc.)",
      "Use profiling tools regularly",
      "Keep debug builds fast enough to iterate",
      "Add developer console for runtime tweaking"
    ]
  },

  "recommended_reading_order": {
    "foundations": [
      "Memory management (critical for everything)",
      "Parallelism and concurrency (modern requirement)",
      "Math systems (foundation for rendering, physics, gameplay)"
    ],
    "core_systems": [
      "Resource management (how assets are loaded)",
      "Rendering systems (visual output)",
      "Animation systems (bringing characters to life)",
      "Physics systems (realistic interactions)"
    ],
    "gameplay": [
      "Entity Component System (modern architecture)",
      "AI systems (NPC behavior)",
      "Event systems (decoupled communication)",
      "Audio systems (immersive soundscapes)"
    ],
    "advanced": [
      "Networking (multiplayer)",
      "Design patterns (reusable solutions)",
      "Performance optimization (profiling and tuning)"
    ]
  },

  "key_takeaways": [
    "Architecture matters: Well-structured engines are easier to maintain and extend",
    "Performance is design: Cache-friendly data layouts beat clever algorithms",
    "Data-driven wins: Separate content from code for faster iteration",
    "Parallelism is essential: Modern hardware demands concurrent programming",
    "Profile, don't guess: Measure before optimizing",
    "Systems thinking: Understand how subsystems interact and depend on each other",
    "Iteration speed: Fast compile times and hot-reloading enable creativity",
    "Memory management is critical: Understand stack vs heap, use custom allocators",
    "ECS enables performance: Data-oriented design with component systems",
    "Loose coupling via events: Reduces dependencies, improves maintainability"
  ],

  "common_pitfalls": [
    {
      "pitfall": "Premature optimization",
      "description": "Optimizing before profiling wastes time and complicates code",
      "solution": "Profile first, optimize hot spots, measure improvement"
    },
    {
      "pitfall": "Overusing singletons",
      "description": "Global state makes testing difficult and creates tight coupling",
      "solution": "Use dependency injection, service locators, or static classes"
    },
    {
      "pitfall": "Ignoring memory layout",
      "description": "Cache misses dominate performance, algorithmic complexity is secondary",
      "solution": "Profile memory access patterns, use SoA, consider hot/cold splitting"
    },
    {
      "pitfall": "Deep inheritance hierarchies",
      "description": "Rigid, difficult to maintain, poor cache locality",
      "solution": "Prefer composition (ECS), shallow hierarchies, interfaces"
    },
    {
      "pitfall": "Not testing edge cases",
      "description": "Division by zero, null pointers, empty arrays cause crashes",
      "solution": "Add assertions, validate inputs, write unit tests"
    },
    {
      "pitfall": "Allocating during gameplay",
      "description": "Causes hitches, fragmentation, unpredictable performance",
      "solution": "Preallocate, use object pools, minimize dynamic allocation"
    },
    {
      "pitfall": "Blocking main thread",
      "description": "Loading, I/O, or expensive calculations cause frame drops",
      "solution": "Async loading, job system, spread work across frames"
    },
    {
      "pitfall": "Not handling failure",
      "description": "Resource load failures, allocation failures cause crashes",
      "solution": "Check return values, use Result<T> types, have fallback assets"
    }
  ]
}
