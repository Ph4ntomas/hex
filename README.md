# HEX
`Hex` is a header only ECS implementation written in **C++ 17**

## Introduction
The entity-component-system is a software architectural pattern that follows the principle of composition over inheritance.

The three building blocks are :
- Entities, the stuff ones want to represent
- Components, these are what define an entity, that is an entity is composed of components that store data only
- Systems, where logic is implemented

Here is how `Hex` implements this pattern:
- Entities are an opaque class that is convertible to a std::size_t. They should be created by `Hex` `entity_manager` class.
- Components can be anything, and are stored internally in sparse array, a type of container that store it's data sequentially, but that can contain holes.
- Systems are up to the user to write.

To manage this, `Hex` provide four classes :
- `entity_manager` : Create, manage and recycle entities. This class can be used to check whether an entity is alive or not, as well as to add components to it, or kill it. It also recycle "old" id, so the components array are kept somewhat packed.
- `component_registry` : store components arrays. Components type **must** be registered to the registry before being used.
- `system_manager` : provide a convenient way to run systems. It provide methods that automatically deduces the systems parameters upon registration, and a `run` method, to run your systems in sequence.
- `context` : ensure these class are properly build, and expose a reference to each of these.

## Code Example
```c++
#include <iostream>

#include <hex/hex.hpp>

struct position {
    float x;
    float y;
};

struct velocity {
    float dx;
    float dy;
};

void count_position(hex::sparse_array<position> const &pos) {
    int count = 0;

    for (std::size_t i = 0; i < pos.size(); ++i) {
        if (pos[i]) ++count;
    }

    std::cout << "There are " << count << " entities with a position" << std::endl;
}

struct functor_counter {

    void operator()(hex::components_registry const &c, hex::sparse_array<velocity> &velocities) {
        hex::sparse_array<position> const & positions = c.get<position>();

        std::size_t max = std::min(positions.size(), velocities.size());

        for (std::size_t i = 0; i < max; ++i) {
            if (positions[i] && velocities[i])
                ++count;
        }
    }

    int count = 0;
};

int main() {
    hex::context ctx;

    ctx.components().register_type<position>();
    ctx.components().register_type<velocity>();

    //spawning five entities with position only
    for (int i = 0; i < 5; ++i)
        ctx.entities().spawn_with<position>({5, 6});

    //spawning five entities with velocity only
    for (int i = 0; i < 5; ++i)
        ctx.entities().spawn_with<velocity>({1, 0});

    //spawning five entities with both
    for (int i = 0; i < 5; ++i)
        ctx.entities().spawn_with<position, velocity>({5, 6}, {1, 0});

    auto fc = functor_counter{};

    ctx.systems().register_system(count_position); // free function registration
    ctx.systems().register_system(fc); // functor registration
    ctx.systems().register_system<position, velocity>([](auto &positions, auto const &velocities){
            // ...
            }); // registering lambda with auto parametes
}
```

## Motivation
I've been intrigued by the ECS pattern from a long time, and wanted to try to design one by myself. It's mostly the reason I made `Hex`.
I also have plan to tinker a bit with it once it'll have enough features.

## Planned Features
In my opinion, `Hex` is in a usable state right now. 

However I have some planned feature/changes still in the making:
- Rework sparse arrays to be more space-efficient (they are implemented as a vector of optional at the moment)
- Implements some way to `zip` Sparse Array to iterate on several components at the same time, and reduce boiler-plate code
- Include an event dispatcher in the context, that would be available to the systems at all time.

## Contributions
Contributions are welcome as code, bugs report, or feature requests. 

I'll write a CONTRIBUTING file, but in the meantime, please use the templates to ask for new features, be courteous and when commiting try to fit in the current style.


