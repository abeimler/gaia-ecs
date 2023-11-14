<!--
@cond TURN_OFF_DOXYGEN
-->
# Gaia-ECS
[![Version][badge.version]][version]
[![Build Status][badge.actions]][actions]
[![language][badge.language]][language]
[![license][badge.license]][license]
[![codacy][badge.codacy]][codacy]

[badge.version]: https://img.shields.io/github/v/release/richardbiely/gaia-ecs?style=for-the-badge
[badge.actions]: https://img.shields.io/github/actions/workflow/status/richardbiely/gaia-ecs/build.yml?branch=main&style=for-the-badge
[badge.language]: https://img.shields.io/badge/language-C%2B%2B17-yellow?style=for-the-badge
[badge.license]: https://img.shields.io/badge/license-MIT-blue?style=for-the-badge
[badge.codacy]: https://app.codacy.com/project/badge/Grade/bb28fa362fce4054bbaf7a6ba9aed140?style=for-the-badge

[version]: https://github.com/richardbiely/gaia-ecs/releases
[actions]: https://github.com/richardbiely/gaia-ecs/actions
[language]: https://en.wikipedia.org/wiki/C%2B%2B17
[license]: https://en.wikipedia.org/wiki/MIT_License
[codacy]: https://app.codacy.com/gh/richardbiely/gaia-ecs/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade

Gaia-ECS is a fast and easy-to-use [ECS](#ecs) framework. Some of its current features and highlights are:
* very simple and safe API designed in such a way as to prevent you from using bad coding patterns
* based on modern [C++17](https://en.cppreference.com/w/cpp/17) technologies
* no external dependencies
* archetype / chunk-based storage for maximum iteration speed and easy code parallelization
* ability to [organize data as AoS or SoA](#data-layouts) on the component level with very few changes to your code
* compiles warning-free on [all major compilers](https://github.com/richardbiely/gaia-ecs/actions)
* compiles almost instantly
* stability secured by running thousands of [unit tests](#unit-testing) and debug-mode asserts in the code
* thoroughly documented both public and internal code
* exists also as a [single-header](#single-header) library so you can simply drop it into your project and start using it

# Table of Contents

* [Introduction](#introduction)
  * [ECS](#ecs)
  * [Implementation](#implementation)
* [Usage](#usage)
  * [Minimum requirements](#minimum-requirements)
  * [Basic operations](#basic-operations)
    * [Create or delete entity](#create-or-delete-entity)
    * [Name entity](#name-entity)
    * [Add or remove component](#add-or-remove-component)
    * [Set or get component value](#set-or-get-component-value)
    * [Component presence](#component-presence)
  * [Data processing](#data-processing)
    * [Query](#query)
    * [Iteration](#iteration)
    * [Constraints](#constraints)
    * [Change detection](#change-detection)
  * [Unique components](#unique-components)
  * [Delayed execution](#delayed-execution)
  * [Data layouts](#data-layouts)
  * [Serialization](#serialization)
  * [Multithreading](#multithreading)
* [Requirements](#requirements)
  * [Compiler](#compiler)
  * [Dependencies](#dependencies)
* [Installation](#installation)
  * [CMake](#cmake)
    * [Project settings](#project-settings)
    * [Sanitizers](#sanitizers)
    * [Single-header](#single-header)
* [Project structure](#project-structure)
  * [Examples](#examples)
  * [Benchmarks](#benchmarks)
  * [Profiling](#profiling)
  * [Testing](#testing)
* [Future](#future)
* [Contributions](#contributions)
* [License](#license)

# Introduction

## ECS
[Entity-Component-System (ECS)](https://en.wikipedia.org/wiki/Entity_component_system) is a software architectural pattern based on organizing your code around data which follows the principle of [composition over inheritance](https://en.wikipedia.org/wiki/Composition_over_inheritance).

Instead of looking at "items" in your program as objects you normally know from the real world (car, house, human) you look at them as pieces of data necessary for you to achieve some result.

This way of thinking is more natural for machines than people but when used correctly it allows you to write faster code (on most architectures). What is most important, however, is it allows you to write code that is easier to maintain, expand and reason about.

For instance, when moving some object from point A to point B you do not care if it is a house or a car. You only care about its position. If you want to move it at some specific speed you will consider also the object's velocity. Nothing else is necessary.

Three building blocks of ECS are:
* **Entity** - an index that uniquely identifies a group of components
* **Component** - a piece of data (position, velocity, age)
* **System** - a place where your program's logic is implemented

Following the example given above, a vehicle could be anything with Position and Velocity components. If it is a car we could attach the Driving component to it. If it is an airplane we would attach the Flying component.<br/>
The actual movement is handled by systems. Those that match against the Flying component will implement the logic for flying. Systems matching against the Driving component handle the land movement.

## Implementation
Gaia-ECS is an archetype-based entity component system. This means that unique combinations of components are grouped into archetypes. Each archetype consists of chunks - blocks of memory holding your entities and components. You can think of them as [database tables](https://en.wikipedia.org/wiki/Table_(database)) where components are columns and entities are rows. Each chunk is either 8 or 16 KiB big depending on how much data can be effectively used by it. This size is chosen so that the entire chunk at its fullest can fit into the L1 cache on most CPUs.

Chunk memory is preallocated in blocks organized into pages via the internal chunk allocator. Thanks to that all data is organized in a cache-friendly way which most computer architectures like and actual heap allocations which are slow are reduced to a minimum.

The main benefits of archetype-based architecture are fast iteration and good memory layout by default. They are also easy to parallelize. On the other hand, adding and removing components can be somewhat slower because it involves moving data around. In our case, this weakness is mitigated by building an archetype graph.

# Usage
## Minimum requirements

```cpp
#include <gaia.h>
```

The entire framework is placed in a namespace called **gaia**.
The ECS part of the library is found under **gaia::ecs** namespace.<br/>
In the code examples below we will assume we are inside the namespace already.

## Basic operations
### Create or delete entity

```cpp
ecs::World w;
ecs::Entity e = w.add();
... // do something with the created entity
w.del(e);
```

### Name entity

Each entity can be assigned a unique name. This is useful for debugging or entity lookup.

```cpp
ecs::World w;
ecs::Entity e = w.add();

// Entity "e" named "my_unique_name"
w.name(e, "my_unique_name");

// Pointer to the "my_unique_name" string stored in the world returned
const char* name = w.name(e);

// Entity returned identified by the string returned
ecs::Entity e_by_name = w.get("my_unique_name");
```

### Add or remove component

```cpp
struct Position {
  float x, y, z;
};
struct Velocity {
  float x, y, z;
};

ecs::World w;

// Create an entity with Position and Velocity.
ecs::Entity e = w.add();
w.add<Position>(e, {0, 100, 0});
w.add<Velocity>(e, {0, 0, 1});

// Remove Velocity from the entity.
w.del<Velocity>(e);
```

When adding or removing multiple components at once if is more efficient doing it via chaining:

```cpp
ecs::World w;

// Create an entity with Position.
ecs::Entity e = w.add();
w.add<Position>();

w.bulk(e)
 // add Position to entity e
 .add<Velocity>()
 // remove Position from entity e
 .del<Position>()
 // add Velocity to entity e
 .add<Rotation>()
 // add a bunch of other components to entity e
 .add<Something1, Something2, Something3>();

```

### Set or get component value

```cpp
// Change Velocity's value.
w.set<Velocity>(e, {0, 0, 2});
// Same as above but the world version is not updated so nobody gets notified of this change.
w.sset<Velocity>(e, {4, 2, 0});
```

When setting multiple component values at once it is more efficient doing it via chaining:

```cpp
w.set(e)
// Change Velocity's value on entity "e"
  .set<Velocity>({0, 0, 2})
// Change Position's value on entity "e"
  .set<Position>({0, 100, 0})
// Change...
  .set...;
```

Components are returned by value for components with sizes up to 8 bytes (including). Bigger components are returned by const reference.

```cpp
// Read Velocity's value. As shown above Velocity is 12 bytes in size.
// Therefore, it is returned by const reference.
const auto& velRef = w.get<Velocity>(e);
// However, it is easy to store a copy.
auto velCopy = w.get<Velocity>(e);
```

Both read and write operations are also accessible via views. Check [simple iteration](#simple-iteration) and [query iteration](#query-iteration) sections to see how.

### Component presence
Whether or not a certain component is associated with an entity can be checked in two different ways. Either via an instance of a World object or by the means of ***Iterator*** which can be acquired when running [queries](#query).

```cpp
// Check if entity e has Velocity (via world).
const bool hasVelocity = w.has<Velocity>(e);
...

// Check if entities hidden behind the iterator have Velocity (via iterator).
ecs::Query q = w.query().any<Position, Velocity>(); 
q.each([&](ecs::Iterator iter) {
  const bool hasPosition = iter.has<Position>();
  const bool hasVelocity = iter.has<Velocity>();
  ...
});
```

## Data processing
### Query
For querying data you can use a Query. It can help you find all entities, components or chunks matching the specified set of components and constraints and iterate or return them in the form of an array. You can also use them to quickly check if any entities satisfying your requirements exist or calculate how many of them there are.

>**NOTE:**<br/>Every Query creates a cache internally. Therefore, the first usage is a little bit slower than the subsequent usage is going to be. You likely use the same query multiple times in your program, often without noticing. Because of that, caching becomes useful as it avoids wasting memory and performance when finding matches.

```cpp
ecs::Query q = w.query();
q.all<Position>(); // Consider only entities with Position

// Fill the entities array with entities with a Position component.
cnt::darray<ecs::Entity> entities;
q.arr(entities);

// Fill the positions array with position data.
cnt::darray<Position> positions;
q.arr(positions);

// Calculate the number of entities satisfying the query
const auto numberOfMatches = q.count();

// Check if any entities satisfy the query.
// Possibly faster than count() because it stops on the first match.
const bool hasMatches = !q.empty();
```

More complex queries can be created by combining All, Any and None in any way you can imagine:

```cpp
ecs::Query q = w.query();
// Take into account everything with Position and Velocity...
q.all<Position, Velocity>();
// ... at least Something or SomethingElse...
q.any<Something, SomethingElse>();
// ... and no Player component...
q.none<Player>();
```

All Query operations can be chained and it is also possible to invoke various filters multiple times with unique components:

```cpp
ecs::Query q = w.query();
// Take into account everything with Position...
q.all<Position>()
// ... and at the same time everything with Velocity...
 .all<Velocity>()
 // ... at least Something or SomethingElse...
 .any<Something, SomethingElse>()
 // ... and no Player component...
 .none<Player>(); 
```

All queries are cached by default. This makes sense for queries which happen very often. They are fast to process but might take more time to prepare initially. If caching is not needed you should use uncached queries and save some resources. You would normally do this for one-time initializations or rarely used operations.

```cpp
// Create an uncached query taking into account all entities with either Positon or Velocity components
ecs::QueryUncached q = w.query<false>().any<Position, Velocity>(); 
```

### Iteration
To process data from queries one uses the ***ecs::Query::each*** function.
It accepts either a list of components or an iterator as its argument.

```cpp
ecs::Query q = w.query();
// Take into account all entities with Position and Velocity...
q.all<Position&, Velocity>();
// ... but no Player component.
q.none<Player>();

q.each([&](Position& p, const Velocity& v) {
  // Run the scope for each entity with Position, Velocity and no Player component
  p.x += v.x * dt;
  p.y += v.y * dt;
  p.z += v.z * dt;
});
```

>**NOTE:**<br/>Iterating over components not present in the query is not supported and results in asserts and undefined behavior. This is done to prevent various logic errors which might sneak in otherwise.

Processing via an iterator gives you even more expressive power and also opens doors for new kinds of optimizations.
Iterator is an abstraction above underlying data structures and gives you access to their public API.

There are three types of iterators:
1) ***ecs::Iterator*** - iterates over enabled entities
2) ***ecs::IteratorDisabled*** - iterates over distabled entities
3) ***ecs::IteratorAll*** - iterate over all entities

```cpp
ecs::Query q = w.query();
q.all<Position&, Velocity>();

q.each([](ecs::IteratorAll iter) {
  auto p = iter.view_mut<Position>(); // Read-write access to Position
  auto v = iter.view<Velocity>(); // Read-only access to Velocity

  // Iterate over all enabled entities and update their x-axis position
  GAIA_EACH(iter) {
    if (!iter.enabled(i))
      return;
    p[i].x += 1.f;
  }

  // Iterate over all entities and update their position based on their velocity
  GAIA_EACH(iter) {
    p[i].x += v[i].x * dt;
    p[i].y += v[i].y * dt;
    p[i].z += v[i].z * dt;
  }
});
```

***GAIA_EACH(iter)*** is simply a shortcut for
```
for (uint32_t i=0; i<iter.size(); ++i)
```
A similar macro exists where you can specify the variable name. It is called ***GAIA_EACH_(iter,k)***
```
for (uint32_t k=0; k<iter.size(); ++k)
```

### Constraints

Query behavior can also be modified by setting constraints. By default, only enabled entities are taken into account. However, by changing constraints, we can filter disabled entities exclusively or make the query consider both enabled and disabled entities at the same time.

Changing the enabled state of an entity is a special operation that marks the entity as invisible to queries by default. Archetype of the entity is not changed and therefore the operation is very fast (basically, just setting a flag).

```cpp
ecs::Entity e1, e2;

// Create 2 entities with Position component
w.add(e1);
w.add(e2);
w.add<Position>(e1);
w.add<Position>(e2);

// Disable the first entity
w.enable(e1, false);

// Check if e1 is enabled
const bool is_e1_enabled = w.enabled(e1);
if (is_e1_enabled) { ... }

// Prepare out query
ecs::Query q = w.query().all<Position&>();

// Fills the array with only e2 because e1 is disabled.
cnt::darray<ecs::Entity> entities;
q.arr(entities);

// Fills the array with both e1 and e2.
q.arr(entities, ecs::Query::Constraint::AcceptAll);

// Fills the array with only e1 because e1 is disabled.
q.arr(entities, ecs::Query::Constraint::DisabledOnly);

q.each([](ecs::Iterator iter) {
  auto p = iter.view_mut<Position>(); // Read-Write access to Position
  // Iterates over enabled entities
  GAIA_EACH(iter) p[i] = {}; // reset the position of each enabled entity
});
q.each([](ecs::IteratorDisabled iter) {
  auto p = iter.view_mut<Position>(); // Read-Write access to Position
  // Iterates over disabled entities
  GAIA_EACH(iter) p[i] = {}; // reset the position of each disabled entity
});
q.each([](ecs::IteratorAll iter) {
  auto p = iter.view_mut<Position>(); // Read-Write access to Position
  // Iterates over all entities
  GAIA_EACH(iter) {
    if (iter.enabled(i)) {
      p[i] = {}; // reset the position of each enabled entity
    }
  }
});
```

If you do not wish to fragment entities inside the chunk you can simply create a tag component and assign it to your entity. This will move the entity to a new archetype so it is a lot slower. However, because disabled entities are now clearly separated calling some query operations might be slightly faster (no need to check if the entity is disabled or not internally).

```cpp
struct Disabled {};

...

e.add<Disabled>(); // disable entity

ecs::Query q = w.query().all<Position, Disabled>; 
q.each([&](ecs::Iterator iter){
  // Processes all disabled entities
});

e.del<Disabled>(); // enable entity
```

### Change detection
Using changed we can make the iteration run only if particular components change. You can save quite a bit of performance using this technique.

```cpp
ecs::Query q = w.query();
// Take into account all entities with Position and Velocity...
q.all<Position&, Velocity>();
// ... no Player component...
q.none<Player>(); 
// ... but only iterate when Velocity changes
q.changed<Velocity>();

q.each([&](Position& p, const Velocity& v) {
  // This scope runs for each entity with Position, Velocity and no Player component
  // but only when Velocity has changed.
  p.x += v.x * dt;
  p.y += v.y * dt;
  p.z += v.z * dt;
});
```

>**NOTE:**<br/>If there are 100 Position components in the chunk and only one them changes, the other 99 are considered changed as well. This chunk-wide behavior might seem counter-intuitive but it is in fact a performance optimization. The reason why this works is because it is easier to reason about a group of entities than checking each of them separately.

## Unique components
Unique component is a special kind of data that exists at most once per chunk. In other words, you attach data to one chunk specifically. It survives entity removals and unlike generic components, they do not transfer to a new chunk along with their entity.

If you organize your data with care (which you should) this can save you some very precious memory or performance depending on your use case.

For instance, imagine you have a grid with fields of 100 meters squared.
Now if you create your entities carefully they get organized in grid fields implicitly on the data level already without you having to use any sort of spatial map container.

```cpp
w.add<Position>(e1, {10,1});
w.add<Position>(e2, {19,1});
// Make both e1 and e2 share a common grid position of {1,0}
w.add<ecs::uni<GridPosition>>(e1, {1, 0});
```

## Delayed execution
Sometimes you need to delay executing a part of the code for later. This can be achieved via CommandBuffers.

CommandBuffer is a container used to record commands in the order in which they were requested at a later point in time.

Typically you use them when there is a need to perform structural changes (adding or removing an entity or component) while iterating chunks.

Performing an unprotected structural change is undefined behavior and most likely crashes the program. However, using a CommandBuffer you can collect all requests first and commit them when it is safe.

```cpp
ecs::CommandBuffer cb;
q.each([&](Entity e, const Position& p) {
  if (p.y < 0.0f) {
    // Queue entity e for deletion if its position falls below zero
    cb.del(e);
  }
});
// Make the queued command happen
cb.commit(&w);
```

If you try to make an unprotected structural change with GAIA_DEBUG enabled (set by default when Debug configuration is used) the framework will assert letting you know you are using it the wrong way.

## Data layouts
By default, all data inside components are treated as an array of structures (AoS) via an implicit

```cpp
static constexpr auto Layout = mem::DataLayout::AoS
```

This is the natural behavior of the language and what you would normally expect.

If we imagine an ordinary array of 4 Position components defined above with this layout they are organized like this in memory: xyz xyz xyz xyz.

However, in specific cases, you might want to consider organizing your component's internal data as structure or arrays (SoA):

```cpp
static constexpr auto Layout = mem::DataLayout::SoA
```

Using the example above will make Gaia-ECS treat Position components like this in memory: xxxx yyyy zzzz.

If used correctly this can have vast performance implications. Not only do you organize your data in the most cache-friendly way this usually also means you can simplify your loops which in turn allows the compiler to optimize your code better.

```cpp
struct PositionSoA {
  float x, y, z;
  static constexpr auto Layout = mem::DataLayout::SoA;
};
struct VelocitySoA {
  float x, y, z;
  static constexpr auto Layout = mem::DataLayout::SoA;
};
...

ecs::Query q = w.query().all<PositionSoA&, VelocitySoA>;
q.each([](ecs::Iterator iter) {
  // Position
  auto vp = iter.view_mut<PositionSoA>(); // read-write access to PositionSoA
  auto px = vp.set<0>(); // continuous block of "x" from PositionSoA
  auto py = vp.set<1>(); // continuous block of "y" from PositionSoA
  auto pz = vp.set<2>(); // continuous block of "z" from PositionSoA

  // Velocity
  auto vv = iter.view<VelocitySoA>(); // read-only access to VelocitySoA
  auto vx = vv.get<0>(); // continuous block of "x" from VelocitySoA
  auto vy = vv.get<1>(); // continuous block of "y" from VelocitySoA
  auto vz = vv.get<2>(); // continuous block of "z" from VelocitySoA

  // Handle x coordinates
  GAIA_EACH(iter) px[i] += vx[i] * dt;
  // Handle y coordinates
  GAIA_EACH(iter) py[i] += vy[i] * dt;
  // Handle z coordinates
  GAIA_EACH(iter) pz[i] += vz[i] * dt;

  /*
  You can even use SIMD intrinsics now without a worry.
  Note, this is just an example not an optimal way to rewrite the loop.
  Also, most compilers will auto-vectorize this code in release builds anyway.

  The code bellow uses x86 SIMD intrinsics.
  uint32_t i = 0;
  // Process SSE-sized blocks first
  for (; i < iter.size(); i+=4) {
    const auto pVec = _mm_load_ps(px.data() + i);
    const auto vVec = _mm_load_ps(vx.data() + i);
    const auto respVec = _mm_fmadd_ps(vVec, dtVec, pVec);
    _mm_store_ps(px.data() + i, respVec);
  }
  // Process the rest of the elements
  for (; i < iter.size(); ++i) {
    ...
  }
  */
});
```

## Serialization
Serialization of arbitrary data is available via following functions:
- ***ser::bytes*** - calculates how many bytes the data needs to serialize
- ***ser::save*** - writes data to serialization buffer
- ***ser::load*** - loads data from serialization buffer

Any data structure can be serialized at compile-time into the provided serialization buffer. Native types, compound types, arrays and anything with data() + size() functions are supported out-of-the-box. If resize() is available it will be utilized.

Example:
```cpp
struct Position {
  float x, y, z;
};
struct Quaternion {
  float x, y, z, w;
};
struct Transform {
  Position p;
  Quaternion r;
};
struct Transform {
  cnt::darray<Transform> transforms;
  int some_int_data;
};

...
Transform in, out;
GAIA_FOR(10) t.transforms.push_back({});
t.some_int_data = 42069;

ecs::SerializationBuffer s;
// Calculate how many bytes is it necessary to serialize "in"
s.reserve(ser::bytes(in));
// Save "in" to our buffer
ser::save(s, in);
// Load the contents of buffer to "out" 
s.seek(0);
ser::load(s, out);
```

Customization is possible for data types which require special attention. We can guide the serializer by either external or internal means.

External specialization comes handy in cases where we can not or do not want to modify the source type:

```cpp
struct CustomStruct {
  char* ptr;
  uint32_t size;
};

namespace gaia::ser {
  template <>
  uint32_t bytes(const CustomStruct& data) {
    return data.size + sizeof(data.size);
  }
  
  template <typename Serializer>
  void save(Serializer& s, const CustomStruct& data) {
    // Save the size integer
    s.save(data.size);
    // Save data.size raw bytes staring at data.ptr
    s.save(data.ptr, data.size);
  }
  
  template <typename Serializer>
  void load(Serializer& s, CustomStruct& data) {
    // Load the size integer
    s.load(data.size);
    // Load data.size raw bytes to location pointed at by data.ptr
    data.ptr = new char[data.size];
    s.load(data.ptr, data.size);
  }
}

...
CustomStruct in, out;
in.ptr = new char[5];
in.ptr[0] = 'g';
in.ptr[1] = 'a';
in.ptr[2] = 'i';
in.ptr[3] = 'a';
in.ptr[4] = '\0';
in.size = 5;

ecs::SerializationBuffer s;
// Reserve enough bytes in the buffer so it can fit the entire in
s.reserve(ser::bytes(in));
ser::save(s, in);
// Move to the start of the buffer and load its contents to out
s.seek(0);
ser::load(s, out);
// Let's release the memory we allocated
delete in.ptr;
delete out.ptr;
```

You will usually use internal specialization when you have the access to your data container and at the same time do not want to expose its internal structure. Or if you simply like intrusive coding style better. In order to use it the following 3 member functions need to be provided:

```cpp
struct CustomStruct {
  char* ptr;
  uint32_t size;
  
  constexpr uint32_t bytes() const noexcept {
    return size + sizeof(size);
  }
  
  template <typename Serializer>
  void save(Serializer& s) const {
    s.save(size);
    s.save(ptr, size);
  }
  
  template <typename Serializer>
  void load(Serializer& s) {
    s.load(size);
    ptr = new char[size];
    s.load(ptr, size);
  }
};
```

 It doesn't matter which kind of specialization you use. However, note that if both are used the external one has priority.

## Multithreading
To fully utilize your system's potential Gaia-ECS allows you to spread your tasks into multiple threads. This can be achieved in multiple ways.

Tasks that can not be split into multiple parts or it does not make sense for them to be split can use ***sched***. It registers a job in the job system and immediately submits it so worker threads can pick it up:
```cpp
mt::Job job0 {[]() {
  InitializeScriptEngine();
}};
mt::Job job1 {[]() {
  InitializeAudioEngine();
}};

// Schedule jobs for parallel execution
mt::JobHandle jobHandle0 = tp.sched(job0);
mt::JobHandle jobHandle1 = tp.sched(job1);

// Wait for jobs to complete
tp.wait(jobHandle0);
tp.wait(jobHandle1);
```

>**NOTE:<br/>**
It is important to call ***wait*** for each scheduled ***JobHandle*** because it also performs cleanup.

Instead of waiting for each job separately, we can also wait for all jobs to be completed using ***wait_all***. This however introduces a hard sync point so it should be used with caution. Ideally, you would want to schedule many jobs and have zero sync points. In most, cases this will not ever happen and at least some sync points are going to be introduced. For instance, before any character can move in the game, all physics calculations will need to be finished.
```cpp
// Wait for all jobs to complete
tp.wait_all();
```

When crunching larger data sets it is often beneficial to split the load among threads automatically. This is what ***sched_par*** is for. 

```cpp
static uint32_t SumNumbers(std::span<const uint32_t> arr) {
  uint32_t sum = 0;
  for (uint32_t val: arr)
    sum += val;
  return sum;
}
...

constexpr uint32_t N = 1'000'000;
cnt::darray<uint32_t> arr;
...

std::atomic_uint32_t sum = 0;
mt::JobParallel job {[&arr, &sum](const mt::JobArgs& args) {
  sum += SumNumbers({arr.data() + args.idxStart, args.idxEnd - args.idxStart});
  }};

// Schedule multiple jobs to run in paralell. Make each job process up to 1234 items.
mt::JobHandle jobHandle = tp.sched_par(job, N, 1234);
// Alternatively, we can tell the job system to figure out the group size on its own
// by simply omitting the group size or using 0:
// mt::JobHandle jobHandle = tp.sched_par(job, N);
// mt::JobHandle jobHandle = tp.sched_par(job, N, 0);

// Wait for jobs to complete
tp.wait(jobHandle);

// Use the result
GAIA_LOG("Sum: %u\n", sum);
```

A similar result can be achieved via ***sched***. It is a bit more complicated because we need to handle workload splitting ourselves. The most compact (and least efficient) version would look something like this:

```cpp
...
constexpr uint32_t ItemsPerJob = 1234;
constexpr uint32_t Jobs = (N + ItemsPerJob - 1) / ItemsPerJob;

std::atomic_uint32_t sum = 0;
GAIA_FOR(Jobs) { // for (uint32_t i=0; i<Jobs; ++i)
  mt::Job job {[&arr, &sum, i, ItemsPerJob, func]() {
    const auto idxStart = i * ItemsPerJob;
    const auto idxEnd = std::min((i + 1) * ItemsPerJob, N);
    sum += SumNumbers({arr.data() + idxStart, idxEnd - idxStart});
  }};
  tp.sched(job);
}

// Wait for all previous tasks to complete
tp.wait_all();
```

Sometimes we need to wait for the result of another operation before we can proceed. To achieve this we need to use low-level API and handle job registration and submitting jobs on our own.
>**NOTE:<br/>** 
This is because once submitted we can not modify the job anymore. If we could, dependencies would not necessary be adhered to.<br/>
Let us say there is a job A depending on job B. If job A is submitted before creating the dependency, a worker thread could execute the job before the dependency is created. As a result, the dependency would not be respected and job A would be free to finish before job B.

```cpp
mt::Job job0;
job0.func = [&arr, i]() {
  arr[i] = i;
};
mt::Job job1;
job1.func = [&arr, i]() {
  arr[i] *= i;
};
mt::Job job2;
job2.func = [&arr, i]() {
  arr[i] += i;
};

// Register our jobs in the job system
auto job0Handle = tp.add(job0);
auto job1Handle = tp.add(job1);
auto job2Handle = tp.add(job2);

// Create dependencies
tp.dep(job1Handle, job0Handle);
tp.dep(job2Handle, job1Handle);

// Submit jobs so worker threads can pick them up.
// The order in which jobs are submitted does not matter.
tp.submit(job2Handle);
tp.submit(job1Handle);
tp.submit(job0Handle);

// Wait for the last job to complete.
// Calling wait() for dependencies is not necessary. It is be done internally.
tp.wait(job2Handle);
```

# Requirements

## Compiler
Compiler with a good support of C++17 is required.<br/>
The project is [continuously tested](https://github.com/richardbiely/gaia-ecs/actions/workflows/build.yml) and guaranteed to build warning-free on the following compilers:<br/>
- [MSVC](https://visualstudio.microsoft.com/) 15 (Visual Studio 2017) or later<br/>
- [Clang](https://clang.llvm.org/) 7 or later<br/>
- [GCC](https://www.gnu.org/software/gcc/) 7 or later<br/>
- [ICX](https://www.intel.com/content/www/us/en/developer/articles/tool/oneapi-standalone-components.html#dpcpp-cpp) 2021.1.2 or later<br/>

## Dependencies
[CMake](https://cmake.org) 3.12 or later is required to prepare the build. Other tools are officially not supported at the moment.

Unit testing is handled via [Catch2 v3.4.0](https://github.com/catchorg/Catch2/releases/tag/v3.4.0). It can be controlled via -DGAIA_BUILD_UNITTEST=ON/OFF when configuring the project (OFF by default).

# Installation

## CMake
The following shows the steps needed to build the library:

```bash
# Check out the library
git clone https://github.com/richardbiely/gaia-ecs.git
# Go to the library root
cd gaia-ecs
# Make a build directory
cmake -E make_directory "build"
# Generate cmake build files (Release for Release configuration)
cmake -DCMAKE_BUILD_TYPE=Release -S . -B "build"
# ... or if you use CMake older than 3.13 you can do:
# cmake -E chdir "build" cmake -DCMAKE_BUILD_TYPE=Release ../
# Build the library
cmake --build "build" --config Release
```

To target a specific build system you can use the [***-G*** parameter](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#manual:cmake-generators(7)):
```bash
# Microsoft Visual Studio 2022, 64-bit, x86 architecture 
cmake -G "Visual Studio 17 2022" -A x64 ...
# Microsoft Visual Studio 2022, 64-bit, ARM architecture 
cmake -G "Visual Studio 17 2022" -A ARM64 ...
# XCode
cmake -G Xcode ...
# Ninja
cmake -G Ninja
```

### Project settings
Following is a list of parameters you can use to customize your build

Parameter | Description      
-|-
**GAIA_BUILD_UNITTEST** | Builds the [unit test project](#unit-testing)
**GAIA_BUILD_BENCHMARK** | Builds the [benchmark project](#benchmarks)
**GAIA_BUILD_EXAMPLES** | Builds [example projects](#examples)
**GAIA_GENERATE_CC** | Generates ***compile_commands.json***
**GAIA_MAKE_SINGLE_HEADER** | Generates a [single-header](#single-header-library) version of the framework
**GAIA_PROFILER_CPU** | Enables CPU [profiling](#profiling) features
**GAIA_PROFILER_MEM** | Enabled memory [profiling](#profiling) features
**GAIA_PROFILER_BUILD** | Builds the [profiler](#profiling) ([Tracy](https://github.com/wolfpld/tracy) by default)
**USE_SANITIZER** | Applies the specified set of [sanitizers](#sanitizers)

### Sanitizers
Possible options are listed in [cmake/sanitizers.cmake](https://github.com/richardbiely/gaia-ecs/blob/main/cmake/sanitizers.cmake).<br/>
Note, that some options don't work together or might not be supported by all compilers.
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SANITIZER=address -S . -B "build"
```

### Single-header
Gaia-ECS is shipped also as a [single header file](https://github.com/richardbiely/gaia-ecs/blob/main/single_include/gaia.h) which you can simply drop into your project and start using. To generate the header we use a wonderful Python tool [Quom](https://github.com/Viatorus/quom).

To generate the header use the following command inside your root directory.
```bash
quom ./include/gaia.h ./single_include/gaia.h -I ./include
```

You can also use the attached make_single_header.sh or create your script for your platform.

Creation of the single header can be automated via -GAIA_MAKE_SINGLE_HEADER.

# Project structure

## Examples
The repository contains some code examples for guidance.<br/>
Examples are built if GAIA_BUILD_EXAMPLES is enabled when configuring the project (OFF by default).

Project name | Description      
-|-
[External](https://github.com/richardbiely/gaia-ecs/tree/main/src/examples/example_external)|A dummy example showing how to use the framework in an external project.
[Standalone](https://github.com/richardbiely/gaia-ecs/tree/main/src/examples/example1)|A dummy example showing how to use the framework in a standalone project.
[Basic](https://github.com/richardbiely/gaia-ecs/tree/main/src/examples/example2)|Simple example using some basic features of the framework.
[Roguelike](https://github.com/richardbiely/gaia-ecs/tree/main/src/examples/example_roguelike)|Roguelike game putting all parts of the framework to use and represents a complex example of how everything would be used in practice. It is work-in-progress and changes and evolves with the project.

## Benchmarks
To be able to reason about the project's performance and prevent regressions benchmarks were created.

Benchmarking relies on a modified [picobench](https://github.com/iboB/picobench). It can be controlled via -DGAIA_BUILD_BENCHMARK=ON/OFF when configuring the project (OFF by default).

Project name | Description      
-|-
[Duel](https://github.com/richardbiely/gaia-ecs/tree/main/src/perf/duel)|Compares different coding approaches such as the basic model with uncontrolled OOP with data all-over-the heap, OOP where allocators are used to controlling memory fragmentation and different ways of data-oriented design and it puts them to test against our ECS framework itself. DOD performance is the target level we want to reach or at least be as close as possible to with this project because it does not get any faster than that.
[Iteration](https://github.com/richardbiely/gaia-ecs/tree/main/src/perf/iter)|Covers iteration performance with different numbers of entities and archetypes.
[Entity](https://github.com/richardbiely/gaia-ecs/tree/main/src/perf/entity)|Focuses on performance of creating and removing entities and components of various sizes.
[Multithreading](https://github.com/richardbiely/gaia-ecs/tree/main/src/perf/mt)|Measures performance of the job system.

## Profiling
It is possible to measure the performance and memory usage of the framework via any 3rd party tool. However, support for [Tracy](https://github.com/wolfpld/tracy) is added by default.

CPU part can be controlled via -DGAIA_PROF_CPU=ON/OFF (OFF by default).

Memory part can be controlled via -DGAIA_PROF_MEM=ON/OFF (OFF by default).

Building the profiler server can be controlled via -DGAIA_PROF_CPU=ON (OFF by default).
>**NOTE:<br/>** This is a low-level feature mostly targeted for maintainers. However, if paired with your own profiler code it can become a very helpful tool.

## Unit testing
The project is thoroughly unit-tested and includes thousands of unit tests covering essentially every feature of the framework. Benchmarking relies on a modified [picobench](https://github.com/iboB/picobench).

It can be controlled via -DGAIA_BUILD_UNITTEST=ON/OFF (OFF by default).

# Future
To see what the future holds for this project navigate [here](https://github.com/users/richardbiely/projects/1/views/1)

# Contributions

Requests for features, PRs, suggestions, and feedback are highly appreciated.

If you find you can help and want to contribute to the project feel free to contact
me directly (you can find the mail on my [profile page](https://github.com/richardbiely)).

# License

Code and documentation Copyright (c) 2021-2023 Richard Biely.

Code released under
[the MIT license](https://github.com/richardbiely/gaia-ecs/blob/master/LICENSE).
<!--
@endcond TURN_OFF_DOXYGEN
-->

