/**
** \file zip.cpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-12-11 11:56
** \date Last update: 2021-12-11 17:30
*/

#include <criterion/criterion.h>

#include "hex/hex.hpp"

template <typename T, size_t Id = 0>
struct component {
    T t;
    static const size_t ID = Id;
};

TestSuite(HexZip, .description = "Testing Hex's zip pseudo-container.", .disabled = false);

Test(HexZip, 00_SanityCheck_one_container, .description = "Try to build a zip object and use it to iterate over one container.") {
    hex::sparse_array<component<int, 0>> sa;

    for (int i = 0; i < 5; ++i) {
        sa.insert_at(i, {i});
        sa.insert_at(i + 10, { i + 10 });
    }

    size_t count = 0;
    for (auto &&[v] : hex::zip{sa}) {
        cr_assert_not(std::is_const_v<std::remove_reference_t<decltype(v)>>);
        ++count;
    }

    cr_assert_eq(count, 10);
}

Test(HexZip, 00_SanityCheck_one_const_container, .description = "Try to build a zip object and use it to iterate over one container.") {
    hex::sparse_array<component<int, 0>> sa;

    for (int i = 0; i < 5; ++i) {
        sa.insert_at(i, {i});
        sa.insert_at(i + 10, { i + 10 });
    }

    size_t count = 0;
    for (auto &&[v] : hex::zip{std::as_const(sa)}) {
        cr_assert(std::is_const_v<std::remove_reference_t<decltype(v)>>);
        ++count;
    }

    cr_assert_eq(count, 10);
}

Test(HexZip, ProperlySkipBeginning_one_container, .disabled = false) {
    hex::sparse_array<component<int, 0>> sa;

    for (int i = 0; i < 5; ++i) {
        sa.insert_at(i + 10, { i + 10 });
    }

    auto z = hex::zip(sa);
    auto it = z.begin();
    auto && [v] = *it;

    cr_assert_eq(v.t, 10);
}

Test(HexZip, IteratingOverEmptyDoesNotCrash_one_container, .disabled = false) {
    hex::sparse_array<component<int, 0>> sa;

    for (auto [v] : hex::zip{sa}) {}
}

Test(HexZip, PastTheEndIsIncrementable, .disabled = false) {
    hex::sparse_array<component<int, 0>> sa;

    hex::zip z{sa};
    auto it = z.begin();

    cr_expect_eq(it, z.end());
    ++it;
}

Test(HexZip, PastTheEndIteratorsAreAlwaysEqual) {
    hex::sparse_array<component<int, 0>> sa;

    hex::zip z{sa};
    hex::zip z2{sa};
    auto it = z.begin();

    cr_expect_eq(it, z.begin(), "precondition failed. iterator is not equal to itself");

    cr_assert_eq(it, z2.begin());
    ++it;
    cr_assert_eq(it, z2.begin());
}

Test(HexZip, ComparisonWorkForIteratorsFromSameZip) {
    hex::sparse_array<component<int, 0>> sa;
    sa.insert_at(0, {10});
    sa.insert_at(10, {10});

    hex::zip z{sa};
    auto it = z.begin();

    cr_expect_eq(it, z.begin(), "precondition failed. iterator is not equal to itself");
    cr_expect_neq(it, z.end(), "precondition failed. iterator is equal to past-the-end");
    ++it;
    cr_expect_neq(it, z.begin());
    ++it;
    cr_expect_neq(it, z.begin());
}

Test(HexZip, IteratorFromDifferentZipAreNeverEqual) {
    hex::sparse_array<component<int, 0>> sa;
    sa.insert_at(0, {10});
    sa.insert_at(10, {10});

    hex::zip z{sa};
    hex::zip z2{sa};
    auto it = z.begin();

    cr_expect_eq(it, z.begin(), "precondition failed. iterator is not equal to itself");

    cr_assert_neq(it, z2.begin());
    ++it;
    cr_assert_neq(it, z2.begin());
}

Test(HexZip, IteratorAreSwappable) {
    hex::sparse_array<component<int, 0>> sa;
    sa.insert_at(0, {10});
    sa.insert_at(10, {10});

    hex::zip z{sa};
    auto it = z.begin();
    auto it2 = z.begin();

    auto e = z.end();

    using std::swap;
    swap(it, e);

    cr_assert_eq(e, it2);
    cr_assert_eq(it, z.end());
}

Test(HexZip, IterateNonConstCanModifyValue_one_container, .disabled = false) {
    hex::sparse_array<component<int, 0>> sa;

    for (int i = 0; i < 5; ++i) {
        sa.insert_at(i, {i});
        sa.insert_at(i + 10, { i + 10 });
    }

    for (auto &&[v] : hex::zip{sa}) {
        v.t += 1;
    }

    for (int i = 0; i < 5; ++i) {
        cr_assert_eq(sa[i].value().t, i + 1);
        cr_assert_eq(sa[i + 10].value().t, i + 11);
    }
}

Test(HexZip, IterateOverSeveralContainer_all_non_const, .disabled = false) {
    hex::sparse_array<component<int, 0>> s1;
    hex::sparse_array<component<int, 1>> s2;
    hex::sparse_array<component<int, 2>> s3;

    size_t check = 0;

    for (int i = 0; i < 150; ++i) {
        if (!(i % 10))
            s1.insert_at(i, {i});

        if (!(i % 20))
            s2.insert_at(i, {i});

        if (!(i % 15))
            s3.insert_at(i, {i});

        if (!(i % 10) && !(i % 15) && !(i % 20))
            ++check;
    }

    size_t count = 0;
    for (auto [v1, v2, v3] : hex::zip{s1, s2, s3}) {
        ++count;
    }

    cr_assert_eq(count, check);
}

Test(HexZip, IterateOverSeveralContainer_some_non_const, .disabled = false) {
    hex::sparse_array<component<int, 0>> s1;
    hex::sparse_array<component<int, 1>> s2;
    hex::sparse_array<component<int, 2>> s3;

    size_t check = 0;

    for (int i = 0; i < 150; ++i) {
        if (!(i % 10))
            s1.insert_at(i, {i});

        if (!(i % 20))
            s2.insert_at(i, {i});

        if (!(i % 15))
            s3.insert_at(i, {i});

        if (!(i % 10) && !(i % 15) && !(i % 20))
            ++check;
    }

    size_t count = 0;
    for (auto [v1, v2, v3] : hex::zip{s1, std::as_const(s2), s3}) {
        ++count;
    }

    cr_assert_eq(count, check);
}

Test(HexZip, 00_SanityCheck_izip_one_container, .description = "Try to build a zip object and use it to iterate over one container.") {
    hex::sparse_array<component<int, 0>> sa;

    for (int i = 0; i < 5; ++i) {
        sa.insert_at(i, {i});
        sa.insert_at(i + 10, { i + 10 });
    }

    size_t count = 0;
    for (auto &&[i, v] : hex::izip{sa}) {
        cr_assert_not(std::is_const_v<std::remove_reference_t<decltype(v)>>);
        ++count;
    }

    cr_assert_eq(count, 10);
}

Test(HexZip, 00_SanityCheck_izip_no_container) {
    int count = 0;
    for (auto &&[i] : hex::izip{}) {
        cr_expect_eq(count, i);
        ++count;

        if (i == 10)
            break;
    }
}

Test(HexZip, IZipIteratorAreSwappable) {
    hex::sparse_array<component<int, 0>> sa;
    sa.insert_at(0, {10});
    sa.insert_at(10, {10});

    hex::izip z{sa};
    auto it = z.begin();
    auto it2 = z.begin();

    auto e = z.end();

    using std::swap;
    swap(it, e);

    cr_assert_eq(e, it2);
    cr_assert_eq(it, z.end());
}
