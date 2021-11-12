/**
** \file component_registry.cpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-12 11:43
** \date Last update: 2021-11-12 15:13
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

    cr.register_type<Component<int, 0>>();
    cr_assert(true);
}

Test(HexComponentRegistry, register_multiple_types, .disabled = false) {
    hex::components_registry cr;

    cr.register_type<Component<int, 0>>();
    cr.register_type<Component<int, 1>>();
    cr.register_type<Component<int, 2>>();
    cr.register_type<Component<int, 3>>();
    cr_assert(true);
}

Test(HexComponentRegistry, reregister_type_should_throw, .disabled = false) {
    hex::components_registry cr;

    cr.register_type<Component<int, 0>>();
    cr_assert_throw((cr.register_type<Component<int, 0>>()), hex::exceptions::already_registered);
}

Test(HexComponentRegistry, try_register_type, .disabled = true) {
    cr_assert_fail();
}

Test(HexComponentRegistry, try_reregister_multiple_types, .disabled = true) {
    cr_assert_fail();
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
