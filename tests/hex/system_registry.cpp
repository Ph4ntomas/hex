/**
** \file system_registry.cpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-23 23:06
** \date Last update: 2022-01-02 13:29
*/

#include <criterion/criterion.h>

#include <stdexcept>

#include "./helpers/systems/functions.hpp"
#include "./helpers/systems/functors.hpp"

using namespace hex::test;
namespace fsystems = systems::functions;

#include "hex/components_registry.hpp"
#include "hex/entity_manager.hpp"
#include "hex/system_registry.hpp"

struct position { int x; int y; };
struct velocity { int vx; int vy; };

template <class ...Args>
hex::system_registry<Args...> make_system_registry() {
    auto cr = std::make_shared<hex::components_registry>();
    auto em = std::make_shared<hex::entity_manager>(cr);

    cr->register_type<position>();
    cr->register_type<velocity>();

    for (int i = 0; i < 5; ++i) {
        em->spawn_with(position{ 6, 7 }, velocity{1, 2});
    }

    for (int i = 0; i < 5; ++i) {
        em->spawn_with(position{ 6, 7 });
    }

    for (int i = 0; i < 5; ++i) {
        em->spawn_with(velocity{ 1, 2 });
    }

    return hex::system_registry<Args...>{em, cr};
}

void empty() {}
void empty_em(hex::entity_manager &) {}
void empty_cr(hex::components_registry &) {}
void empty_em_cr(hex::entity_manager &, hex::components_registry &) {}

TestSuite(HexSystemRegistry, .disabled = false);

Test(HexSystemRegistry, 00_build_proper, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();
    auto em = std::make_shared<hex::entity_manager>(cr);

    hex::system_registry s{em, cr};
}

Test(HexSystemRegistry, 00_build_bad_entity_manager_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();
    std::shared_ptr<hex::entity_manager> em = nullptr;

    cr_assert_throw(hex::system_registry s(em, cr); , std::invalid_argument);
}

Test(HexSystemRegistry, 00_build_bad_component_registry_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();
    auto em = std::make_shared<hex::entity_manager>(cr);
    auto cr2 = std::shared_ptr<hex::components_registry>();

    cr_assert_throw(hex::system_registry s(em, cr2);, std::invalid_argument);
}

Test(HexSystemRegistry, 00_move_build, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();
    auto em = std::make_shared<hex::entity_manager>(cr);

    hex::system_registry s{em, cr};

    auto crc = cr.use_count();
    auto emc = em.use_count();

    hex::system_registry s2(std::move(s));

    cr_assert_eq(crc, cr.use_count());
    cr_assert_eq(emc, em.use_count());
}


Test(HexSystemRegistry, 00_move_assign, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();
    auto em = std::make_shared<hex::entity_manager>(cr);

    hex::system_registry s{em, cr};

    auto crc = cr.use_count();
    auto emc = em.use_count();

    auto cr2 = std::make_shared<hex::components_registry>();
    auto em2 = std::make_shared<hex::entity_manager>(cr);
    auto wcr = std::weak_ptr(cr2);
    auto wem = std::weak_ptr(em2);

    hex::system_registry s2{em2, cr2};

    cr2 = nullptr;
    em2 = nullptr;

    cr_expect_not(wcr.expired());
    cr_expect_not(wem.expired());

    s2 = std::move(s);

    cr_assert_eq(crc, cr.use_count());
    cr_assert_eq(emc, em.use_count());
    cr_expect(wcr.expired());
    cr_expect(wem.expired());
}

Test(HexSystemRegistry, 01_sanity_register_should_not_throw, .disabled = false) {
    auto s = make_system_registry();

    s.register_system(empty);
    s.register_system(empty_em);
    s.register_system<hex::entity_manager>(empty_em);
    s.register_system(empty_cr);
    s.register_system(empty_em_cr);
    s.register_system(fsystems::system_no_args);
    s.register_system(systems::functor_no_args());
    s.register_system([](hex::containers::sparse_array<position> &){});
    s.register_system<position, velocity>([](auto &, auto &){});
    s.register_system<position, velocity>([](auto &, auto const &){});
    s.register_system<hex::entity_manager, position, velocity>([](auto &, auto &, auto const &){});
}

Test(HexSystemRegistry, 01_sanity_empty_run_should_not_throw, .disabled = false) {
    auto s = make_system_registry();

    s.run();
}

Test(HexSystemRegistry, register_and_run_one_function, .disabled = false) {
    auto s = make_system_registry();

    s.register_system(fsystems::system_check_is_called);

    cr_expect_not(fsystems::called());

    s.run();

    cr_assert(fsystems::called());
}

Test(HexSystemRegistry, register_and_run_functions, .disabled = false) {
    auto s = make_system_registry();

    s.register_system(fsystems::system_check_is_called);
    s.register_system(fsystems::system_check_order<0>);
    s.register_system(fsystems::system_check_order<1>);
    s.register_system(fsystems::system_check_order<2>);

    cr_expect_not(fsystems::called());
    cr_expect_eq(fsystems::counter(), 0);

    s.run();

    cr_assert(fsystems::called());
    cr_assert_eq(fsystems::counter(), 3);
}

Test(HexSystemRegistry, register_and_run_one_functor, .disabled = false) {
    auto s = make_system_registry();
    systems::functor_check_called fun;

    s.register_system(fun);

    cr_expect_not(fun.was_called);

    s.run();

    cr_assert(fun.was_called);
}

Test(HexSystemRegistry, register_and_run_functors, .disabled = false) {
    auto s = make_system_registry();
    systems::functor_check_order<0> first;
    systems::functor_check_order<1> second;
    systems::functor_check_order<2> third;

    s.register_system(first);
    s.register_system(second);
    s.register_system(third);

    cr_expect_eq(systems::shared().count, 0);

    s.run();

    cr_assert_eq(systems::shared().count, 3);
}

Test(HexSystemRegistry, register_and_run_one_lambda, .disabled = false) {
    auto s = make_system_registry();
    bool ran = false;

    s.register_system<position>([&](auto const &positions){
        cr_assert((std::is_same_v<decltype(positions), hex::containers::sparse_array<position> const &>));
        ran = true;
    });

    s.run();
    cr_assert(ran);
}

Test(HexSystemRegistry, register_and_run_lambdas, .disabled = false) {
    auto s = make_system_registry();

    int counter = 0;

    s.register_system<position>([&](auto const &positions){
        cr_expect_eq(counter, 0, "wrong call order");
        cr_assert((std::is_same_v<decltype(positions), hex::containers::sparse_array<position> const &>));
        ++counter;
    });

    s.register_system<velocity>([&](auto const &velocities){
        cr_expect_eq(counter, 1, "wrong call order");
        cr_assert((std::is_same_v<decltype(velocities), hex::containers::sparse_array<velocity> const &>));
        ++counter;
    });

    s.register_system<position, velocity>([&](auto const &positions, auto &velocities){
        cr_expect_eq(counter, 2, "wrong call order");
        cr_assert((std::is_same_v<decltype(positions), hex::containers::sparse_array<position> const &>));
        cr_assert((std::is_same_v<decltype(velocities), hex::containers::sparse_array<velocity> &>));
        ++counter;
    });

    s.run();

    cr_assert_eq(counter, 3, "Not all lambda were called.");
}

Test(HexSystemRegistry, register_and_run_mixed, .disabled = false) {
    auto s = make_system_registry();
    bool ran = false;
    systems::functor_check_order<0> first;
    systems::functor_check_order<1> second;
    systems::functor_check_order<2> third;

    s.register_system(fsystems::system_check_order<0>);
    s.register_system(fsystems::system_check_order<1>);
    s.register_system(first);
    s.register_system([&](){
            cr_expect_eq(fsystems::counter(), 2);
            cr_expect_eq(systems::shared().count, 1);
    });
    s.register_system(fsystems::system_check_is_called);

    s.register_system<position>([&](auto const &positions){
        cr_assert((std::is_same_v<decltype(positions), hex::containers::sparse_array<position> const &>));
        ran = true;
    });

    s.register_system(fsystems::system_check_order<2>);
    s.register_system(std::move(second));
    s.register_system(third);

    cr_expect_not(fsystems::called());
    cr_expect_eq(fsystems::counter(), 0);
    cr_expect_not(fsystems::called());
    cr_expect_eq(systems::shared().count, 0);

    s.run();

    cr_assert_eq(fsystems::counter(), 3);
    cr_assert_eq(systems::shared().count, 3);
    cr_assert(fsystems::called());
    cr_assert(ran);
}

Test(HexSystemRegistry, register_function_check_args_bad_arg_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();
    auto em = std::make_shared<hex::entity_manager>(cr);
    auto s = hex::system_registry(em, cr);

    cr_assert_throw(s.register_system(hex::check, fsystems::system_check_is_called), hex::exceptions::no_such_component);
    cr_assert_throw(s.register_system(hex::check, systems::functor_check_called{}), hex::exceptions::no_such_component);
    cr_assert_throw(s.register_system<position>(hex::check, [](auto &){}), hex::exceptions::no_such_component);
}

Test(HexSystemRegistry, register_and_run_check_args, .disabled = false) {
    auto s = make_system_registry();
    bool ran = false;
    systems::functor_check_order<0> first;
    systems::functor_check_order<1> second;
    systems::functor_check_order<2> third;

    s.register_system(hex::check, fsystems::system_check_order<0>);
    s.register_system(hex::check, fsystems::system_check_order<1>);
    s.register_system(hex::check, first);
    s.register_system(hex::check, [&](){
            cr_expect_eq(fsystems::counter(), 2);
            cr_expect_eq(systems::shared().count, 1);
    });
    s.register_system(hex::check, fsystems::system_check_is_called);

    s.register_system<position>(hex::check, [&](auto const &positions){
        cr_assert((std::is_same_v<decltype(positions), hex::containers::sparse_array<position> const &>));
        ran = true;
    });

    s.register_system(hex::check, fsystems::system_check_order<2>);
    s.register_system(hex::check, std::move(second));
    s.register_system(hex::check, third);

    cr_expect_not(fsystems::called());
    cr_expect_eq(fsystems::counter(), 0);
    cr_expect_not(fsystems::called());
    cr_expect_eq(systems::shared().count, 0);

    s.run();

    cr_assert_eq(fsystems::counter(), 3);
    cr_assert_eq(systems::shared().count, 3);
    cr_assert(fsystems::called());
    cr_assert(ran);
}

Test(HexSystemRegistry, register_function_register_args_bad_arg, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();
    auto em = std::make_shared<hex::entity_manager>(cr);
    auto s = hex::system_registry(em, cr);

    s.register_system(hex::auto_register, fsystems::system_check_is_called);
    s.register_system(hex::auto_register, systems::functor_check_called{});
    s.register_system<position, velocity>(hex::auto_register, [](auto &, auto &){});

    cr_assert(cr->has<position>());
    cr_assert(cr->has<velocity>());
}

Test(HexSystemRegistry, register_and_run_function_register_args, .disabled = false) {
    auto s = make_system_registry();
    bool ran = false;
    systems::functor_check_order<0> first;
    systems::functor_check_order<1> second;
    systems::functor_check_order<2> third;

    s.register_system(hex::auto_register, fsystems::system_check_order<0>);
    s.register_system(hex::auto_register, fsystems::system_check_order<1>);
    s.register_system(hex::auto_register, first);
    s.register_system(hex::auto_register, [&](){
            cr_expect_eq(fsystems::counter(), 2);
            cr_expect_eq(systems::shared().count, 1);
    });
    s.register_system(fsystems::system_check_is_called);

    s.register_system<position>([&](auto const &positions){
        cr_assert((std::is_same_v<decltype(positions), hex::containers::sparse_array<position> const &>));
        ran = true;
    });

    s.register_system(hex::auto_register, fsystems::system_check_order<2>);
    s.register_system(hex::auto_register, std::move(second));
    s.register_system(hex::auto_register, third);

    cr_expect_not(fsystems::called());
    cr_expect_eq(fsystems::counter(), 0);
    cr_expect_not(fsystems::called());
    cr_expect_eq(systems::shared().count, 0);

    s.run();

    cr_assert_eq(fsystems::counter(), 3);
    cr_assert_eq(systems::shared().count, 3);
    cr_assert(fsystems::called());
    cr_assert(ran);
}

struct arbitratry {
    int val = 10;
};

Test(HexSystemRegistry, run_with_arbitrary_args, .disabled = false) {
    auto s = make_system_registry<arbitratry>();
    auto a = arbitratry{};

    s.register_system<arbitratry>([](auto & arb) { cr_assert((std::is_same_v<arbitratry &, decltype(arb)>)); });

    s.run(a);
}
