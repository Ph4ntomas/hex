/**
** \file functions.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-12-04 15:24
** \date Last update: 2022-01-02 13:22
*/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <criterion/criterion.h>

#include <hex/system_registry.hpp>

struct position;
struct velocity;

namespace hex::test::systems::functions {
    bool &called() {
        thread_local bool is_called = false;
        return is_called;
    };

    int & counter() {
        thread_local int count = 0;
        return count;
    }


    void system_no_args() {
        called() = true;
    }

    void system_check_is_called(entity_manager &em, containers::sparse_array<position> const &) {
        called() = true;
    }

    template <size_t order, class... Args>
    void system_check_order(system_registry<Args...> const &, containers::sparse_array<velocity> &) {
        cr_expect_eq(counter(), order, "Bad call order.");

        ++(counter());
    }
}

#endif /* end of include guard: FUNCTIONS_H */
