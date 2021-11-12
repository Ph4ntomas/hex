/**
** \file component_registry.cpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-12 11:43
** \date Last update: 2021-11-12 15:22
*/

#include <criterion/criterion.h>

#include <hex/components_registry.hpp>
#include <hex/exceptions/already_registered.hpp>

template <typename T, size_t Id>
struct Component {
    T val;
};

TestSuite(HexComponentRegistry, .description = "Ensure components_registry enable works as expected.", .disabled = false);

Test(HexComponentRegistry, buildEmptyRegistry, .disabled = false) {
    hex::components_registry cr;
    cr_assert(true);
}

Test(HexComponentRegistry, register_type, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<Component<int, 0>>();
    cr_assert_eq(sa.size(), 0);
}

Test(HexComponentRegistry, register_multiple_types, .disabled = false) {
    hex::components_registry cr;

    auto &sa1 = cr.register_type<Component<int, 0>>();
    auto &sa2 = cr.register_type<Component<int, 1>>();
    cr.register_type<Component<int, 2>>();
    cr.register_type<Component<int, 3>>();

    cr_assert_eq(sa1.size(), 0);
    cr_assert_eq(sa2.size(), 0);
}

Test(HexComponentRegistry, reregister_type_should_throw, .disabled = false) {
    hex::components_registry cr;

    cr.register_type<Component<int, 0>>();
    cr_assert_throw((cr.register_type<Component<int, 0>>()), hex::exceptions::already_registered);
}

Test(HexComponentRegistry, try_register_type, .disabled = false) {
    hex::components_registry cr;

    auto &&[v, ok] = cr.try_register_type<Component<int, 0>>();
    cr_assert(ok);
}

Test(HexComponentRegistry, try_register_multiple_types, .disabled = false) {
    hex::components_registry cr;

    auto &&[v1, ok1] = cr.try_register_type<Component<int, 0>>();
    cr_assert(ok1);
    cr_assert_eq(v1.size(), 0);

    auto &&[v2, ok2] = cr.try_register_type<Component<int, 1>>();
    cr_assert(ok2);
    cr_assert_eq(v2.size(), 0);

    auto &&[v3, ok3] = cr.try_register_type<Component<int, 2>>();
    cr_assert(ok3);
    cr_assert_eq(v3.size(), 0);
}

Test(HexComponentRegistry, try_reregister_multiple_types, .disabled = false) {
    hex::components_registry cr;

    auto &&[v1, ok1] = cr.try_register_type<Component<int, 0>>();
    cr_assert(ok1);
    cr_assert_eq(v1.size(), 0);
    v1.insert_at(4, Component<int, 0>{2});
    cr_expect_geq(v1.size(), 5, "Something is wrong with sparse_array");

    auto &&[v2, ok2] = cr.try_register_type<Component<int, 0>>();
    cr_assert_not(ok2);
    cr_assert_eq(std::addressof(v1), std::addressof(v2));
}

}

Test(HexComponentRegistry, get_registered_type, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, const_get_registered_type, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, copy_insert_component, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, copy_insert_override_component, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, copy_insert_unregistered_component_should_throw, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, move_insert_component, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, move_insert_override_component, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, move_insert_unregistered_component_should_throw, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, build_emplace_component, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, build_emplace_override_component, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, build_emplace_unregistered_component_should_throw, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, copy_emplace_component, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, copy_emplace_override_component, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, copy_emplace_unregistered_component_should_throw, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, move_emplace_component, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, move_emplace_override_component, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, move_emplace_unregistered_component_should_throw, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, remove_at_existing_component, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, remove_at_unexisting_component, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, remove_at_unexisting_id, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, erase_at_existing_entity, .disabled = true) {
    cr_assert_fail();
}
