/**
** \file functors.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-12-04 15:58
** \date Last update: 2021-12-04 16:14
*/

#ifndef FUNCTORS_H
#define FUNCTORS_H

#include <criterion/criterion.h>

#include <hex/system_registry.hpp>

struct position;
struct velocity;

namespace hex::test::systems {
    struct base_functor {
        int count;
        bool was_called;
    };

    base_functor &shared() {
        thread_local base_functor s{};
        return s;
    }

    struct functor_no_args : base_functor {
        void operator()() { was_called = true; }
    };

    struct functor_check_called : base_functor {
        void operator()(hex::components_registry &) {
            was_called = true;
        }
    };

    template <size_t order>
    struct functor_check_order {
        void operator()(hex::entity_manager &, components_registry const &) {
            cr_expect_eq(shared().count, order);
            ++shared().count;
        }
    };
}

#endif /* end of include guard: FUNCTORS_H */
