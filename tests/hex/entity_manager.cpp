/**
** \file entity_manager.cpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-14 18:06
** \date Last update: 2023-01-31 11:28
*/

#include <criterion/criterion.h>

#include <vector>

#include "hex/components_registry.hpp"

#define HEX_TEST
#include "hex/entity_manager.hpp"

template <typename... T>
struct component {};

struct position {
    int x;
    int y;
};

template <typename... Ts>
std::shared_ptr<hex::components_registry> make_cr() {
    auto cr = std::make_shared<hex::components_registry>();

    (cr->try_register_type<Ts>(), ...);

    return cr;
}

struct no_default {
    std::string val;
    std::size_t id;

    no_default(std::string v, std::size_t i) : val(v), id(i) {}
    no_default(no_default const &) = default;
    no_default(no_default &&) = default;
};

TestSuite(HexEntityManager, .description = "Ensure Hex's Entity Manager works as expected", .disabled = false);

Test(HexEntityManager, build_proper, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();
    hex::entity_manager em{cr};
}

Test(HexEntityManager, build_null_registry_should_throw, .disabled = false) {
    std::shared_ptr<hex::components_registry> sp;

    cr_assert_throw(hex::entity_manager em{sp};, std::invalid_argument);
}

Test(HexEntityManager, move_build, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();
    std::weak_ptr weak = cr;

    hex::entity_manager em(cr);
    cr = nullptr;

    auto e = em.spawn();

    hex::entity_manager em2 = std::move(em);

    auto e2 = em2.spawn();

    cr_assert(!weak.expired());
    cr_assert_lt(e, e2);
    cr_assert(em2.is_live(e));
}

Test(HexEntityManager, move_assign, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();
    std::weak_ptr<hex::components_registry> weak = cr;

    hex::entity_manager em{cr};

    cr = nullptr;

    cr_assert(!weak.expired());

    auto cr2 = std::make_shared<hex::components_registry>();
    hex::entity_manager em2 = cr2;

    auto e = em2.spawn();

    em = std::move(em2);

    auto e2 = em.spawn();

    cr_assert(weak.expired());
    cr_assert_lt(e, e2);
    cr_assert(em.is_live(e));
}

Test(HexEntityManager, spawn_one, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);

    auto e = em.spawn();
    cr_assert_eq(e.id, 0);
    cr_assert_eq(e.version, 0);
}

Test(HexEntityManager, spawn_multiple, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);

    for (int i = 0; i < 5; ++i) {
        auto e = em.spawn();
        cr_assert_eq(e.id, i);
        cr_assert_eq(e.version, 0);
    }
}

Test(HexEntityManager, spawn_with_one_component_once, .disabled = false) {
    auto cr = make_cr<component<int>>();

    hex::entity_manager em(cr);

    auto e = em.spawn_with(component<int>{});
    cr_assert_eq(e.id, 0);
    cr_assert_eq(e.version, 0);

    auto const &sa = cr->get<component<int>>();
    cr_assert_eq(sa.size(), 1);
}

Test(HexEntityManager, spawn_with_one_component_multiple_entities, .disabled = false) {
    auto cr = make_cr<component<int>>();

    hex::entity_manager em(cr);

    for (int i = 0; i < 5; ++i) {
        auto e = em.spawn_with(component<int>{});
        cr_assert_eq(e.id, i);
        cr_assert_eq(e.version, 0);
    }

    auto const &sa = cr->get<component<int>>();
    cr_assert_eq(sa.size(), 5);
}

Test(HexEntityManager, spawn_with_several_component_once, .disabled = false) {
    auto cr = make_cr<
            component<int>,
            component<float>,
            component<int, int>
         >();

    hex::entity_manager em(cr);

    auto e = em.spawn_with(
            component<int>{},
            component<float>{},
            component<int, int>{});
    cr_assert_eq(e.id, 0);
    cr_assert_eq(e.version, 0);

    auto const &sai = cr->get<component<int>>();
    auto const &saf = cr->get<component<float>>();
    auto const &saii = cr->get<component<int, int>>();

    cr_assert_eq(sai.size(), 1);
    cr_assert_eq(saf.size(), 1);
    cr_assert_eq(saii.size(), 1);
}

Test(HexEntityManager, spawn_with_several_component_multiple_entities, .disabled = false) {
    auto cr = make_cr<
            component<int>,
            component<float>,
            component<int, int>
         >();

    hex::entity_manager em(cr);

    for (int i = 0; i < 5; ++i) {
        auto e = em.spawn_with(
                component<int>{},
                component<float>{},
                component<int, int>{});
        cr_assert_eq(e.id, i);
        cr_assert_eq(e.version, 0);
    }

    auto const &sai = cr->get<component<int>>();
    auto const &saf = cr->get<component<float>>();
    auto const &saii = cr->get<component<int, int>>();

    cr_assert_eq(sai.size(), 5);
    cr_assert_eq(saf.size(), 5);
    cr_assert_eq(saii.size(), 5);
}

Test(HexEntityManager, spawn_entity_then_kill_it, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);
    auto e = em.spawn();
    em.kill(e);
}

Test(HexEntityManager, spawn_entity_then_kill_id_0, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);
    auto e = em.spawn();
    em.kill_at(0);
}

Test(HexEntityManager, spawn_several_entity_then_kill_one, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);
    auto e = em.spawn();
    auto e1 = em.spawn();
    auto e2 = em.spawn();
    em.kill(e1);
}

Test(HexEntityManager, spawn_several_entity_then_kill_one_by_id, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);
    auto e = em.spawn();
    auto e1 = em.spawn();
    auto e2 = em.spawn();
    em.kill_at(1);
}

Test(HexEntityManager, kill_already_dead_entity, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);
    auto e = em.spawn();
    em.kill(e);
    cr_assert_throw(em.kill(e), hex::exceptions::already_dead);

    auto e2 = em.spawn();
    cr_assert_eq(e2.id, 0);
    cr_assert_neq(e.version, e2.version);
    cr_assert_throw(em.kill(e), hex::exceptions::already_dead);
}

Test(HexEntityManager, kill_already_dead_entity_by_id, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);
    auto e = em.spawn();
    em.kill_at(0);
    cr_assert_throw(em.kill_at(0), hex::exceptions::already_dead);
}

Test(HexEntityManager, kill_inhexisting_entities_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);

    cr_assert_throw(em.kill_at(0), hex::exceptions::no_such_entity);
    auto e1 = em.spawn();
    auto e2 = em.spawn();
    auto e3 = em.spawn();

    cr_assert_throw(em.kill_at(5), hex::exceptions::no_such_entity);
}

Test(HexEntityManager, spawn_several_entity_then_try_kill_one, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);
    auto e = em.spawn();
    auto e1 = em.spawn();
    auto e2 = em.spawn();
    cr_assert(em.try_kill(e1));
}

Test(HexEntityManager, spawn_several_entity_then_try_kill_one_by_id, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);
    auto e = em.spawn();
    auto e1 = em.spawn();
    auto e2 = em.spawn();
    cr_assert(em.try_kill_at(1));
}

Test(HexEntityManager, try_kill_already_dead_entity, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);
    auto e = em.spawn();
    em.kill(e);
    cr_assert_not(em.try_kill(e));
}

Test(HexEntityManager, try_kill_already_dead_entity_by_id, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);
    auto e = em.spawn();
    em.kill_at(0);
    cr_assert_not(em.try_kill_at(0));
}

Test(HexEntityManager, try_kill_inhexisting_entities, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);

    cr_assert_not(em.try_kill_at(0));
    auto e1 = em.spawn();
    auto e2 = em.spawn();
    auto e3 = em.spawn();

    cr_assert_not(em.try_kill_at(5));
}

Test(HexEntityManager, kill_entity_remove_components, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);
    auto e = em.spawn_with(position{1, 2});
    em.spawn_with(position{1, 2});
    auto e2 = em.spawn_with(position{1, 2});
    em.spawn_with(position{1, 2});

    cr_expect(cr->get<position>().at(0), "precondition failed");
    cr_expect(cr->get<position>().at(1), "precondition failed");
    cr_expect(cr->get<position>().at(2), "precondition failed");
    cr_expect(cr->get<position>().at(3), "precondition failed");

    em.kill(e);
    em.kill_at(1);
    em.try_kill(e2);
    em.try_kill_at(3);

    cr_assert_not(cr->get<position>().at(0), "kill is not removing components.");
    cr_assert_not(cr->get<position>().at(1), "kill_at is not removing components.");
    cr_assert_not(cr->get<position>().at(2), "try_kill is not removing components.");
    cr_assert_not(cr->get<position>().at(3), "try_kill_at is not removing components.");
}

Test(HexEntityManager, respawn_one_entity_once, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);
    auto e = em.spawn();
    cr_expect_eq(e.id, 0);
    cr_expect_eq(e.version, 0);
    em.kill(e);

    e = em.spawn();
    cr_assert_eq(e.id, 0);
    cr_assert_eq(e.version, 1);
}
Test(HexEntityManager, respawn_one_entity_several_time, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);

    for (int i = 0; i < 5; ++i) {
        auto e = em.spawn();
        cr_expect_eq(e.id, 0);
        cr_expect_eq(e.version, i);
        em.kill(e);
    }
}

Test(HexEntityManager, respawn_several_entities, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);
    std::vector<hex::entity_t> ve(5);

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            ve[j] = em.spawn();
        }

        std::sort(ve.begin(), ve.end());

        for (int j = 0; j < 5; ++j) {
            cr_expect_eq(ve[j].id,j);
            cr_assert_eq(ve[j].version, i);
            em.kill_at(j);
        }
    }
}

Test(HexEntityManager, add_component_by_copy, .disabled = false) {
    auto cr = make_cr<position>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<position>().size(), 0);

    em.add_component(e, p);

    cr_assert_eq(cr->get<position>().size(), 1);
}

Test(HexEntityManager, add_bad_component_by_copy_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw(em.add_component(e, p), std::out_of_range);
}

Test(HexEntityManager, add_component_by_rvalue, .disabled = false) {
    auto cr = make_cr<position>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<position>().size(), 0);

    em.add_component(e, std::move(p));

    cr_assert_eq(cr->get<position>().size(), 1);
}

Test(HexEntityManager, add_bad_component_by_rvalue_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw(em.add_component(e, std::move(p)), std::out_of_range);
}

Test(HexEntityManager, add_component_by_copy_id, .disabled = false) {
    auto cr = make_cr<position>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<position>().size(), 0);

    em.add_component(0, p);

    cr_assert_eq(cr->get<position>().size(), 1);
}

Test(HexEntityManager, add_component_by_copy_bad_id_should_throw, .disabled = false) {
    auto cr = make_cr<position>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<position>().size(), 0);
    cr_assert_throw(em.add_component(12, p), hex::exceptions::no_such_entity);
    cr_assert_eq(cr->get<position>().size(), 0);
}

Test(HexEntityManager, add_bad_component_by_copy_id_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw(em.add_component(0, p), std::out_of_range);
}

Test(HexEntityManager, add_component_by_rvalue_id, .disabled = false) {
    auto cr = make_cr<position>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<position>().size(), 0);

    em.add_component(0, std::move(p));

    cr_assert_eq(cr->get<position>().size(), 1);
}

Test(HexEntityManager, add_bad_component_by_rvalue_id_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw(em.add_component(0, std::move(p)), std::out_of_range);
}

Test(HexEntityManager, add_component_by_rvalue_bad_id_should_throw, .disabled = false) {
    auto cr = make_cr<position>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<position>().size(), 0);
    cr_assert_throw(em.add_component(12, std::move(p)), hex::exceptions::no_such_entity);
    cr_assert_eq(cr->get<position>().size(), 0);
}

Test(HexEntityManager, emplace_component_build, .disabled = false) {
    auto cr = make_cr<no_default>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<no_default>().size(), 0);

    em.emplace_component<no_default>(e, "test", 2);

    cr_assert_eq(cr->get<no_default>().size(), 1);
}

Test(HexEntityManager, emplace_bad_component_build_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw(em.emplace_component<no_default>(e, "test", 2), std::out_of_range);
}

Test(HexEntityManager, emplace_component_by_copy, .disabled = false) {
    auto cr = make_cr<position>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<position>().size(), 0);

    em.emplace_component<position>(e, p);

    cr_assert_eq(cr->get<position>().size(), 1);
}

Test(HexEntityManager, emplace_bad_component_by_copy_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw(em.emplace_component<position>(e, p), std::out_of_range);
}

Test(HexEntityManager, emplace_component_by_rvalue, .disabled = false) {
    auto cr = make_cr<position>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<position>().size(), 0);

    em.emplace_component<position>(e, std::move(p));

    cr_assert_eq(cr->get<position>().size(), 1);
}

Test(HexEntityManager, emplace_bad_component_by_rvalue_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw(em.emplace_component<position>(e, std::move(p)), std::out_of_range);
}

Test(HexEntityManager, emplace_component_build_id, .disabled = false) {
    auto cr = make_cr<no_default>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<no_default>().size(), 0);

    em.emplace_component<no_default>(0, "test", 2);

    cr_assert_eq(cr->get<no_default>().size(), 1);
}

Test(HexEntityManager, emplace_bad_component_build_id_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw(em.emplace_component<no_default>(0, "test", 2), std::out_of_range);
}

Test(HexEntityManager, emplace_component_build_bad_id_should_throw, .disabled = false) {
    auto cr = make_cr<no_default>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<no_default>().size(), 0);
    cr_assert_throw(em.emplace_component<no_default>(12, "test", 2), hex::exceptions::no_such_entity);
    cr_assert_eq(cr->get<no_default>().size(), 0);
}

Test(HexEntityManager, emplace_component_by_copy_id, .disabled = false) {
    auto cr = make_cr<position>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<position>().size(), 0);
    em.emplace_component<position>(0, p);
    cr_assert_eq(cr->get<position>().size(), 1);
}

Test(HexEntityManager, emplace_bad_component_by_copy_id_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw(em.emplace_component<position>(0, p), std::out_of_range);
}

Test(HexEntityManager, emplace_component_by_copy_bad_id_should_throw, .disabled = false) {
    auto cr = make_cr<position>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<position>().size(), 0);
    cr_assert_throw(em.emplace_component<position>(12, p), hex::exceptions::no_such_entity);
    cr_assert_eq(cr->get<position>().size(), 0);
}

Test(HexEntityManager, emplace_component_by_rvalue_id, .disabled = false) {
    auto cr = make_cr<position>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<position>().size(), 0);

    em.emplace_component<position>(0, std::move(p));

    cr_assert_eq(cr->get<position>().size(), 1);
}

Test(HexEntityManager, emplace_bad_component_by_rvalue_id_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw(em.emplace_component<position>(0, std::move(p)), std::out_of_range);
}

Test(HexEntityManager, emplace_component_by_rvalue_bad_id_should_throw, .disabled = false) {
    auto cr = make_cr<position>();

    position p { 1, 2 };
    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_eq(cr->get<position>().size(), 0);
    cr_assert_throw(em.emplace_component<position>(12, std::move(p)), hex::exceptions::no_such_entity);
    cr_assert_eq(cr->get<position>().size(), 0);
}

Test(HexEntityManager, has_component, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn_with(position{1,2});

    cr_assert(em.has_component<position>(e));
}

Test(HexEntityManager, has_unadded_component, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_not(em.has_component<position>(e));
}

Test(HexEntityManager, has_unregistered_component_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw((void)em.has_component<position>(e), std::out_of_range);
}

Test(HexEntityManager, has_component_by_id, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn_with(position{1,2});

    cr_assert(em.has_component<position>(0));
}

Test(HexEntityManager, has_unadded_component_by_id, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_not(em.has_component<position>(0));
}

Test(HexEntityManager, has_unregistered_component_by_id_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw((void)em.has_component<position>(0), std::out_of_range);
}

Test(HexEntityManager, has_component_bad_id_should_throw, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn_with(position{1,2});

    cr_assert_throw((void)em.has_component<position>(10), hex::exceptions::no_such_entity);
}

Test(HexEntityManager, get_component, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn_with(position{1,2});

    auto pos = em.get_component<position>(e);
    cr_assert_eq(pos.x, 1);
    cr_assert_eq(pos.y, 2);
}

Test(HexEntityManager, get_unadded_component_should_throw, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw((void)em.get_component<position>(e), std::out_of_range);
}

Test(HexEntityManager, get_unregistered_component_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw((void)em.get_component<position>(e), std::out_of_range);
}

Test(HexEntityManager, get_component_by_id, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn_with(position{1,2});

    auto pos = em.get_component<position>(0);
    cr_assert_eq(pos.x, 1);
    cr_assert_eq(pos.y, 2);
}

Test(HexEntityManager, get_unadded_component_by_id_should_throw, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw((void)em.get_component<position>(0), std::out_of_range);
}

Test(HexEntityManager, get_unregistered_component_by_id_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw((void)em.get_component<position>(0), std::out_of_range);
}

Test(HexEntityManager, get_component_bad_id_should_throw, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn_with(position{1,2});

    cr_assert_throw((void)em.get_component<position>(10), hex::exceptions::no_such_entity);
}

Test(HexEntityManager, remove_component, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn_with(position{1,2});
    cr_expect(em.has_component<position>(e));
    em.remove_component<position>(e);
    cr_assert_not(em.has_component<position>(e));
}

Test(HexEntityManager, remove_unregistered_component_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw(em.remove_component<position>(e), std::out_of_range);
}

Test(HexEntityManager, remove_unadded_component_should_not_throw, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_no_throw(em.remove_component<position>(e), std::out_of_range);
}

Test(HexEntityManager, remove_component_by_id, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn_with(position{1,2});
    cr_expect(em.has_component<position>(0));
    em.remove_component<position>(0);
    cr_assert_not(em.has_component<position>(0));
}
Test(HexEntityManager, remove_unregistered_component_by_id_should_throw, .disabled = false) {
    auto cr = std::make_shared<hex::components_registry>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw(em.remove_component<position>(0), std::out_of_range);
}
Test(HexEntityManager, remove_unadded_component_by_id_should_not_throw, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_no_throw(em.remove_component<position>(0), std::out_of_range);
}

Test(HexEntityManager, remove_component_bad_id_should_throw, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert_throw(em.remove_component<position>(10), hex::exceptions::no_such_entity);
}

Test(HexEntityManager, is_live_spawned_entity, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert(em.is_live(e));
}

Test(HexEntityManager, is_live_killed_entity, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();
    em.kill(e);

    cr_assert_not(em.is_live(e));
}

Test(HexEntityManager, is_live_spawned_id, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    cr_assert(em.is_live(0));
}

Test(HexEntityManager, is_live_killed_id, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();
    em.kill(e);

    cr_assert_not(em.is_live(0));
}

Test(HexEntityManager, is_live_bad_id_should_throw, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    cr_assert_throw((void)em.is_live(10), hex::exceptions::no_such_entity);
}

Test(HexEntityManager, get_entity_good_id, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    auto e2 = em.get_entity(0);

    cr_assert_eq(e, e2);
}

Test(HexEntityManager, get_entity_dead_id_should_throw, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();
    em.kill(e);

    cr_assert_throw((void)em.get_entity(0), hex::exceptions::already_dead);
}

Test(HexEntityManager, get_entity_bad_id_should_throw, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    cr_assert_throw((void)em.get_entity(10), hex::exceptions::no_such_entity);
}

Test(HexEntityManager, try_get_entity_good_id, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();

    auto e2 = em.try_get_entity(0);

    cr_assert_eq(e, e2.value());
}

Test(HexEntityManager, try_get_entity_dead_id, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    auto e = em.spawn();
    em.kill(e);

    cr_assert_not(em.try_get_entity(0));
}

Test(HexEntityManager, try_get_entity_bad_id, .disabled = false) {
    auto cr = make_cr<position>();

    hex::entity_manager em(cr);

    cr_assert_not(em.try_get_entity(10));
}
