/**
** \file component_registry.cpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-12 11:43
** \date Last update: 2021-11-25 12:28
*/

#include <criterion/criterion.h>

#include <hex/components_registry.hpp>
#include <hex/exceptions/already_registered.hpp>

template <typename T, size_t Id>
struct Component {
    T val;
};

template <typename T, size_t Id, size_t Id2 = Id>
bool operator==(Component<T, Id> const &lhs, Component<T, Id2> const &rhs) {
    return lhs.val == rhs.val && Id == Id2;
}

struct emplace_test {
    emplace_test(int _v1, int _v2): v1(_v1), v2(_v2) {}

    friend bool operator==(emplace_test const &lhs, emplace_test const &rhs) {
        return lhs.v1 == rhs.v1 && lhs.v2 == rhs.v2;
    }
    private:
        int v1;
        int v2;
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

Test(HexComponentRegistry, has_registered_type, .disabled = false) {
    hex::components_registry cr;

    cr.register_type<Component<int, 0>>();

    cr_assert((cr.has<Component<int, 0>>()));
}

Test(HexComponentRegistry, has_unregistered_type, .disabled = false) {
    hex::components_registry cr;

    cr.register_type<Component<int, 0>>();

    cr_assert_not((cr.has<Component<int, 1>>()));
}

Test(HexComponentRegistry, get_registered_type, .disabled = false) {
    hex::components_registry cr;

    cr.register_type<Component<int, 0>>();

    auto &sa = cr.get<Component<int, 0>>();

    cr_assert(!std::is_const_v<std::remove_reference_t<decltype(sa)>>);
    cr_assert_eq(
        typeid(hex::containers::sparse_array<Component<int, 0>>),
        typeid(std::remove_reference_t<decltype(sa)>)
    );
}

Test(HexComponentRegistry, const_get_registered_type, .disabled = false) {
    hex::components_registry cr;

    cr.register_type<Component<int, 0>>();

    auto &sa = std::as_const(cr).get<Component<int, 0>>();

    cr_assert(std::is_const_v<std::remove_reference_t<decltype(sa)>>);
    cr_assert_eq(
        typeid(hex::containers::sparse_array<Component<int, 0>>),
        typeid(std::remove_reference_t<decltype(sa)>)
    );
}


Test(HexComponentRegistry, get_unregistered_type, .disabled = false) {
    hex::components_registry cr;

    cr.register_type<Component<int, 0>>();

    cr_assert_throw(((void)cr.get<Component<int, 1>>()), std::out_of_range);

}

Test(HexComponentRegistry, const_get_unregistered_type, .disabled = false) {
    hex::components_registry cr;

    cr.register_type<Component<int, 0>>();

    cr_assert_throw(((void)std::as_const(cr).get<Component<int, 1>>()), std::out_of_range);
}

Test(HexComponentRegistry, copy_insert_component, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<Component<int, 0>>();

    auto comp = Component<int, 0>{5};
    cr.insert_at(5, comp);

    cr_assert_eq(sa.at(5), std::optional{comp});
}

Test(HexComponentRegistry, copy_insert_override_component, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<Component<int, 0>>();

    auto comp = Component<int, 0>{5};
    auto comp2 = Component<int, 0>{6};
    cr.insert_at(5, comp);
    cr_expect_eq(sa.at(5), std::optional{comp});

    cr.insert_at(5, comp2);
    cr_expect_eq(sa.at(5), std::optional{comp2});
}

Test(HexComponentRegistry, copy_insert_unregistered_component_should_throw, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<Component<int, 0>>();

    auto comp = Component<int, 1>{5};
    cr_assert_throw(cr.insert_at(5, comp), std::out_of_range);
}

Test(HexComponentRegistry, move_insert_component, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<Component<int, 0>>();

    auto comp = Component<int, 0>{5};
    cr.insert_at(5, std::move(comp));

    cr_assert_eq(sa.at(5), std::optional{comp});
}

Test(HexComponentRegistry, move_insert_override_component, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<Component<int, 0>>();

    auto comp = Component<int, 0>{5};
    auto comp2 = Component<int, 0>{6};
    cr.insert_at(5, std::move(comp));
    cr_expect_eq(sa.at(5), std::optional{comp});

    cr.insert_at(5, std::move(comp2));
    cr_expect_eq(sa.at(5), std::optional{comp2});
}

Test(HexComponentRegistry, move_insert_unregistered_component_should_throw, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<Component<int, 0>>();

    auto comp = Component<int, 1>{5};
    cr_assert_throw(cr.insert_at(5, std::move(comp)), std::out_of_range);
}

Test(HexComponentRegistry, build_emplace_component, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<emplace_test>();

    cr.emplace_at<emplace_test>(5, 1, 2);

    cr_assert_eq(sa.at(5), std::make_optional<emplace_test>(1, 2));
}

Test(HexComponentRegistry, build_emplace_override_component, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<emplace_test>();

    auto comp = emplace_test{5, 6};
    auto comp2 = emplace_test{6, 7};
    cr.emplace_at<decltype(comp)>(5, 5, 6);
    cr_expect_eq(sa.at(5), std::make_optional<emplace_test>(5, 6));

    cr.emplace_at<decltype(comp)>(5, 6, 7);
    cr_expect_eq(sa.at(5), std::make_optional<emplace_test>(6, 7));
}

Test(HexComponentRegistry, build_emplace_unregistered_component_should_throw, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<Component<int, 0>>();

    cr_assert_throw(cr.emplace_at<emplace_test>(5, 1, 2), std::out_of_range);
}

Test(HexComponentRegistry, copy_emplace_component, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<Component<int, 0>>();

    auto comp = Component<int, 0>{5};
    cr.emplace_at<decltype(comp)>(5, comp);

    cr_assert_eq(sa.at(5), std::optional{comp});
}

Test(HexComponentRegistry, copy_emplace_override_component, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<Component<int, 0>>();

    auto comp = Component<int, 0>{5};
    auto comp2 = Component<int, 0>{6};
    cr.emplace_at<decltype(comp)>(5, comp);
    cr_expect_eq(sa.at(5), std::optional{comp});

    cr.emplace_at<decltype(comp)>(5, comp2);
    cr_expect_eq(sa.at(5), std::optional{comp2});
}

Test(HexComponentRegistry, copy_emplace_unregistered_component_should_throw, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<Component<int, 0>>();

    auto comp = Component<int, 1>{5};
    cr_assert_throw(cr.emplace_at<decltype(comp)>(5, comp), std::out_of_range);
}

Test(HexComponentRegistry, move_emplace_component, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<Component<int, 0>>();

    auto comp = Component<int, 0>{5};
    cr.emplace_at<decltype(comp)>(5, std::move(comp));

    cr_assert_eq(sa.at(5), std::optional{comp});
}

Test(HexComponentRegistry, move_emplace_override_component, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<Component<int, 0>>();

    auto comp = Component<int, 0>{5};
    auto comp2 = Component<int, 0>{6};
    cr.emplace_at<decltype(comp)>(5, std::move(comp));
    cr_expect_eq(sa.at(5), std::optional{comp});

    cr.emplace_at<decltype(comp)>(5, std::move(comp2));
    cr_expect_eq(sa.at(5), std::optional{comp2});
}

Test(HexComponentRegistry, move_emplace_unregistered_component_should_throw, .disabled = false) {
    hex::components_registry cr;

    auto &sa = cr.register_type<Component<int, 0>>();

    auto comp = Component<int, 1>{5};
    cr_assert_throw(cr.emplace_at<decltype(comp)>(5, std::move(comp)), std::out_of_range);
}

Test(HexComponentRegistry, remove_at_existing_component, .disabled = false) {
    hex::components_registry cr;

    Component<int, 0> comp{5};
    auto &sa = cr.register_type<decltype(comp)>();

    cr.insert_at(5, comp);

    cr_expect_eq(sa.at(5), std::optional{comp});

    cr.remove_at<decltype(comp)>(5);
    cr_assert_eq(sa.at(5), std::nullopt);
}

Test(HexComponentRegistry, remove_at_unexisting_component_should_throw, .disabled = false) {
    hex::components_registry cr;

    Component<int, 0> comp{5};
    Component<int, 1> comp2{5};
    auto &sa = cr.register_type<decltype(comp2)>();

    cr.insert_at(5, comp2);

    cr_expect_eq(sa.at(5), std::optional{comp2});

    cr_assert_throw(cr.remove_at<decltype(comp)>(5), std::out_of_range);
}

Test(HexComponentRegistry, remove_at_unexisting_id, .disabled = false) {
    hex::components_registry cr;

    Component<int, 0> comp{5};
    auto &sa = cr.register_type<decltype(comp)>();

    cr.insert_at(5, comp);

    cr_expect_eq(sa.at(5), std::optional{comp});
    cr_assert_eq(sa.at(4), std::nullopt);

    cr.remove_at<decltype(comp)>(4);
    cr_assert_eq(sa.at(4), std::nullopt);


    auto sz = sa.size();
    cr.remove_at<decltype(comp)>(sa.size() + 10);
    cr_assert_eq(sa.size(), sz);
}

Test(HexComponentRegistry, erase_at_existing_entity, .disabled = false) {
    hex::components_registry cr;

    Component<int, 0> comp{5};
    Component<int, 1> comp2{5};
    Component<int, 2> comp3{5};
    Component<int, 3> comp4{5};

    auto &sa1 = cr.register_type<decltype(comp)>();
    auto &sa2 = cr.register_type<decltype(comp2)>();
    auto &sa3 = cr.register_type<decltype(comp3)>();
    auto &sa4 = cr.register_type<decltype(comp4)>();

    cr.insert_at(5, comp);
    cr.insert_at(5, comp2);
    cr.insert_at(7, comp3);
    cr.insert_at(2, comp4);

    cr_expect_eq(sa1.at(5), std::optional{comp});
    cr_expect_eq(sa2.at(5), std::optional{comp2});
    cr_expect_eq(sa3.at(5), std::nullopt);
    cr_expect_eq(sa3.at(7), std::optional{comp3});
    cr_expect_eq(sa4.at(2), std::optional{comp4});

    auto sz = sa4.size();

    cr.erase_at(5);

    cr_assert_eq(sa4.size(), sz);

    cr_assert_eq(sa1.at(5), std::nullopt);
    cr_assert_eq(sa2.at(5), std::nullopt);
    cr_assert_eq(sa3.at(5), std::nullopt);
    cr_assert_eq(sa3.at(7), std::optional{comp3});
}

Test(HexComponentRegistry, erase_at_non_existing_entity, .disabled = false) {
    hex::components_registry cr;

    Component<int, 0> comp{5};
    Component<int, 1> comp2{5};

    auto &sa1 = cr.register_type<decltype(comp)>();
    auto &sa2 = cr.register_type<decltype(comp2)>();

    cr.insert_at(1, comp);
    cr.insert_at(5, comp2);

    cr_expect_eq(sa1.at(1), std::optional{comp});
    cr_expect_eq(sa2.at(4), std::nullopt);
    cr_expect_eq(sa2.at(5), std::optional{comp2});

    auto sz1 = sa1.size();
    auto sz2 = sa2.size();

    cr.erase_at(2);

    cr_assert_eq(sa1.size(), sz1);
    cr_assert_eq(sa2.size(), sz2);

    cr_assert_eq(sa1.at(1), std::optional{comp});
    cr_assert_eq(sa2.at(4), std::nullopt);
    cr_assert_eq(sa2.at(5), std::optional{comp2});
}
