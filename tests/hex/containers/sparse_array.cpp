#include <criterion/criterion.h>
#include <hex/containers/sparse_array.hpp>

struct ex_component {
    int x;
    int y;
};

struct imov_component {
    int x;
    int y;

    imov_component(int _x, int _y): x(_x), y(_y) {}
    imov_component(imov_component const &c): x(c.x), y(c.y) {};
    imov_component(imov_component &&) = delete;

    imov_component &operator=(imov_component const &) = default;
};

struct icop_component {
    int x;
    int y;

    icop_component(int _x, int _y): x(_x), y(_y) {}
    icop_component(icop_component const &c) = delete;
    icop_component(icop_component &&c): x(c.x), y(c.y) {};

    icop_component &operator=(icop_component &&) = default;
};

bool operator==(ex_component const &lhs, ex_component const &rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator==(icop_component const &lhs, icop_component const &rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator==(imov_component const &lhs, imov_component const &rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

template <class T>
using sparse_allocator = std::allocator<std::optional<T>>;

template <typename T>
struct custom_allocator {
    using value_type = T;

    T * allocate(size_t n) { std::allocator<T> a; return a.allocate(n); }
    void deallocate(T *p, size_t n) { std::allocator<T> a; return a.deallocate(p, n); }
};

template <typename T, typename U = T>
bool operator==(custom_allocator<T> const &lhs, custom_allocator<U> const &rhs) {
    return std::is_same_v<T, U>;
}

TestSuite(HexSparseArray, .description = "Ensure sparse array works as expected.", .disabled = false);

Test(HexSparseArray, 00_buildEmptySparseArray, .disabled = false) {
    {
        auto sa_int = hex::containers::sparse_array<int>{};
        auto sa_cmp = hex::containers::sparse_array<ex_component>{};

        cr_assert_eq(sa_int.size(), 0, "Array of int is not empty.");
        cr_assert_eq(sa_cmp.size(), 0, "Array of struct is not empty.");
    }

    {
        auto sa_int = hex::containers::sparse_array<int>(sparse_allocator<int>{});
        auto sa_cmp = hex::containers::sparse_array<ex_component>(sparse_allocator<ex_component>{});

        cr_assert_eq(sa_int.size(), 0, "Array of int is not empty.");
        cr_assert_eq(sa_cmp.size(), 0, "Array of struct is not empty.");
    }
}

Test(HexSparseArray, 00_buildArrayOf10ValueEmpty, .disabled = false) {
    {
        auto sa_int = hex::containers::sparse_array<int>(10);
        auto sa_cmp = hex::containers::sparse_array<ex_component>(10);

        cr_assert_eq(sa_int.size(), 10, "Array of 10 default element is not built properly.");
        for (auto &oi : sa_int)
            cr_assert_eq(oi, std::nullopt);

        cr_assert_eq(sa_cmp.size(), 10, "Array of 10 default element is not built properly.");
        for (auto &oc : sa_cmp)
            cr_assert_eq(oc, std::nullopt);
    }

    {
        auto sa_int = hex::containers::sparse_array<int>(10, sparse_allocator<int>{});
        auto sa_cmp = hex::containers::sparse_array<ex_component>(10, sparse_allocator<ex_component>{});

        cr_assert_eq(sa_int.size(), 10, "Array of 10 default element is not built properly.");
        for (auto &oi : sa_int)
            cr_assert_eq(oi, std::nullopt);

        cr_assert_eq(sa_cmp.size(), 10, "Array of 10 default element is not built properly.");
        for (auto &oc : sa_cmp)
            cr_assert_eq(oc, std::nullopt);
    }
}

Test(HexSparseArray, 00_buildArrayOf10ValueNullopt, .disabled = false) {
    {
        auto sa_int = hex::containers::sparse_array<int>(10, std::nullopt);
        auto sa_cmp = hex::containers::sparse_array<ex_component>(10, std::nullopt);

        cr_assert_eq(sa_int.size(), 10, "Array of 10 default element is not built properly.");
        for (auto &oi : sa_int)
            cr_assert_eq(oi, std::nullopt);

        cr_assert_eq(sa_cmp.size(), 10, "Array of 10 default element is not built properly.");
        for (auto &oc : sa_cmp)
            cr_assert_eq(oc, std::nullopt);
    }

    {
        auto sa_int = hex::containers::sparse_array<int>(10, std::nullopt, sparse_allocator<int>{});
        auto sa_cmp = hex::containers::sparse_array<ex_component>(10, std::nullopt, sparse_allocator<ex_component>{});

        cr_assert_eq(sa_int.size(), 10, "Array of 10 default element is not built properly.");
        for (auto &oi : sa_int)
            cr_assert_eq(oi, std::nullopt);

        cr_assert_eq(sa_cmp.size(), 10, "Array of 10 default element is not built properly.");
        for (auto &oc : sa_cmp)
            cr_assert_eq(oc, std::nullopt);
    }
}

Test(HexSparseArray, 00_buildArrayOf10ValueSetOptional, .disabled = false) {
    {
        auto sa_int = hex::containers::sparse_array<int>(10, std::optional<int>(6));
        auto sa_cmp = hex::containers::sparse_array<ex_component>(10, std::optional(ex_component{4, 5}));

        cr_assert_eq(sa_int.size(), 10); //, "Array of 10 default element is not built properly.");
        for (auto &oi : sa_int)
            cr_assert_eq(oi, std::optional<int>(6));

        cr_assert_eq(sa_cmp.size(), 10); //, "Array of 10 default element is not built properly.");
        for (auto &oc : sa_cmp)
            cr_assert_eq(oc, std::optional(ex_component{4, 5}));
    }

    {
        auto sa_int = hex::containers::sparse_array<int>(10, std::optional<int>(6), sparse_allocator<int>{});
        auto sa_cmp = hex::containers::sparse_array<ex_component>(10, std::optional(ex_component{4, 5}), sparse_allocator<ex_component>{});

        cr_assert_eq(sa_int.size(), 10); //, "Array of 10 default element is not built properly.");
        for (auto &oi : sa_int)
            cr_assert_eq(oi, std::optional<int>(6));

        cr_assert_eq(sa_cmp.size(), 10); //, "Array of 10 default element is not built properly.");
        for (auto &oc : sa_cmp)
            cr_assert_eq(oc, std::optional(ex_component{4, 5}));
    }
}

Test(HexSparseArray, 00_buildArrayOf10ValueSetValue, .disabled = false) {
    {
        auto sa_int = hex::containers::sparse_array<int>(10, 6);
        auto sa_cmp = hex::containers::sparse_array<ex_component>(10, ex_component{4, 5});

        cr_assert_eq(sa_int.size(), 10, "Array of 10 default element is not built properly.");
        for (auto &oi : sa_int)
            cr_assert_eq(oi, std::optional<int>(6));

        cr_assert_eq(sa_cmp.size(), 10, "Array of 10 default element is not built properly.");
        for (auto &oc : sa_cmp)
            cr_assert_eq(oc, std::optional(ex_component{4, 5}));
    }

    {
        auto sa_int = hex::containers::sparse_array<int>(10, 6, sparse_allocator<int>{});
        auto sa_cmp = hex::containers::sparse_array<ex_component>(10, ex_component{4, 5}, sparse_allocator<ex_component>{});

        cr_assert_eq(sa_int.size(), 10, "Array of 10 default element is not built properly.");
        for (auto &oi : sa_int)
            cr_assert_eq(oi, std::optional<int>(6));

        cr_assert_eq(sa_cmp.size(), 10, "Array of 10 default element is not built properly.");
        for (auto &oc : sa_cmp)
            cr_assert_eq(oc, std::optional(ex_component{4, 5}));
    }
}

Test(HexSparseArray, 00_buildArrayOf10ValueImovValue, .disabled = false) {
    {
        auto sa_cmp = hex::containers::sparse_array<imov_component>(10, imov_component{4, 5});

        cr_assert_eq(sa_cmp.size(), 10, "Array of 10 default element is not built properly.");
        for (auto &oc : sa_cmp)
            cr_assert_eq(oc, std::make_optional<imov_component>(4, 5));
    }

    {
        auto sa_cmp = hex::containers::sparse_array<imov_component>(10, imov_component{4, 5}, sparse_allocator<imov_component>{});

        cr_assert_eq(sa_cmp.size(), 10, "Array of 10 default element is not built properly.");
        for (auto &oc : sa_cmp)
            cr_assert_eq(oc, std::make_optional<imov_component>(4, 5));
    }
}

Test(HexSparseArray, 00_buildArrayFromiteratorsOfOptional, .disabled = false) {
    {
        auto v = std::vector{
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
        };

        auto sa = hex::containers::sparse_array(v.begin(), v.end());

        cr_assert_eq(sa.size(), 10, "List initialization did not yield 10 elements");
        for (auto &oc : sa)
            cr_assert_eq(oc, std::optional<ex_component>({1, 2}));
    }

    {
        auto v = std::vector{
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
        };

        auto sa = hex::containers::sparse_array(v.begin(), v.end(), sparse_allocator<ex_component>{});

        cr_assert_eq(sa.size(), 10, "List initialization did not yield 10 elements");
        for (auto &oc : sa)
            cr_assert_eq(oc, std::optional<ex_component>({1, 2}));
    }
}

Test(HexSparseArray, 00_buildArrayFromiteratorsOfValue, .disabled = false) {
    {
        auto v = std::vector{
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
        };

        auto sa = hex::containers::sparse_array(v.begin(), v.end());

        cr_assert_eq(sa.size(), 10, "List initialization did not yield 10 elements");
        for (auto &oc : sa)
            cr_assert_eq(oc, std::make_optional<imov_component>(1, 2));
    }

    {
        auto v = std::vector{
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
        };

        auto sa = hex::containers::sparse_array(v.begin(), v.end(), sparse_allocator<imov_component>{});

        cr_assert_eq(sa.size(), 10, "List initialization did not yield 10 elements");
        for (auto &oc : sa)
            cr_assert_eq(oc, std::make_optional<imov_component>(1, 2));
    }
}

Test(HexSparseArray, 00_buildArrayOf10ListInitOptional, .disabled = false) {
    {
        auto sa = hex::containers::sparse_array{
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
        };

        cr_assert_eq(sa.size(), 10, "List initialization did not yield 10 elements");
        for (auto &oc : sa)
            cr_assert_eq(oc, std::optional<ex_component>({1, 2}));
    }

    {
        auto sa = hex::containers::sparse_array({
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
            std::optional<ex_component>({1, 2}),
        }, sparse_allocator<ex_component>{});

        cr_assert_eq(sa.size(), 10, "List initialization did not yield 10 elements");
        for (auto &oc : sa)
            cr_assert_eq(oc, std::optional<ex_component>({1, 2}));
    }
}

Test(HexSparseArray, 00_buildArrayOf10ListInitOptionalOfNonMovable, .disabled = false) {
    auto sa = hex::containers::sparse_array{
        std::make_optional<imov_component>(1, 2),
        std::make_optional<imov_component>(1, 2),
        std::make_optional<imov_component>(1, 2),
        std::make_optional<imov_component>(1, 2),
        std::make_optional<imov_component>(1, 2),
        std::make_optional<imov_component>(1, 2),
        std::make_optional<imov_component>(1, 2),
        std::make_optional<imov_component>(1, 2),
        std::make_optional<imov_component>(1, 2),
        std::make_optional<imov_component>(1, 2),
    };

    cr_assert_eq(sa.size(), 10, "List initialization did not yield 10 elements");
    for (auto &oc : sa)
        cr_assert_eq(oc, std::make_optional<imov_component>(1, 2));
}

Test(HexSparseArray, 00_buildArrayOf10ListInitNullopt, .disabled = false) {
    auto sa = hex::containers::sparse_array<int>{ //CTAD would fail without explicit instantiation
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
    };

    cr_assert_eq(sa.size(), 10, "List initialization did not yield 10 elements");
    for (auto &oc : sa)
        cr_assert_eq(oc, std::nullopt);
}

Test(HexSparseArray, 00_buildArrayOf10ListInitCopiable, .disabled = false) {
    {
        auto sa = hex::containers::sparse_array{
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
        };

        cr_assert_eq(sa.size(), 10, "List initialization did not yield 10 elements");
        for (auto &oc : sa)
            cr_assert_eq(oc, std::make_optional<imov_component>(1, 2));
    }

    {
        auto sa = hex::containers::sparse_array({
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
            imov_component{1, 2},
        }, sparse_allocator<imov_component>{});

        cr_assert_eq(sa.size(), 10, "List initialization did not yield 10 elements");
        for (auto &oc : sa)
            cr_assert_eq(oc, std::make_optional<imov_component>(1, 2));
    }
}

//non copiable component can't go in a init list, as initializer_list are implemented as an array of constant values.

Test(HexSparseArray, 00_copyBuildArrayOf10, .disabled = false) {
    auto sa = hex::containers::sparse_array{
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
    };

    hex::containers::sparse_array na = sa;

    cr_assert_eq(na.size(), 10, "copy ctor did not yield 10 elements");
    for (auto &oc : na)
        cr_assert_eq(oc, std::make_optional<imov_component>(1, 2));
}

Test(HexSparseArray, 00_moveBuildArrayOf10, .disabled = false) {
    auto sa = hex::containers::sparse_array{
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
    };

    hex::containers::sparse_array na = std::move(sa);

    cr_assert_eq(na.size(), 10, "Move ctor did not yield 10 elements");
    for (auto &oc : na)
        cr_assert_eq(oc, std::make_optional<imov_component>(1, 2));
}

Test(HexSparseArray, 00_moveBuildArrayOf10NonCopiable, .disabled = false) {
    auto sa = hex::containers::sparse_array<icop_component>{};

    for (int i = 0; i < 10; ++i)
        sa.insert_at(i, {1, 2});

    hex::containers::sparse_array na = std::move(sa);

    cr_assert_eq(na.size(), 10, "Move ctor did not yield 10 elements");
    for (auto &oc : na)
        cr_assert_eq(oc, std::make_optional<icop_component>(1, 2));
}

Test(HexSparseArray, 01_copyOperatorAssignEmpty, .disabled = false) {
    auto sa = hex::containers::sparse_array<imov_component>{};

    hex::containers::sparse_array<imov_component> na{
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
    };
    na = sa;

    cr_assert_eq(na.size(), 0);
}

Test(HexSparseArray, 01_moveOperatorAssignEmpty, .disabled = false) {
    auto sa = hex::containers::sparse_array<icop_component>{};

    for (int i = 0; i < 10; ++i)
        sa.insert_at(i, {1, 2});

    hex::containers::sparse_array<icop_component> na;

    sa = std::move(na);

    cr_assert_eq(sa.size(), 0);
}

Test(HexSparseArray, 01_copyOperatorAssignArrayOf10, .disabled = false) {
    auto sa = hex::containers::sparse_array{
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
    };

    hex::containers::sparse_array<imov_component> na;
    na = sa;

    cr_assert_eq(na.size(), 10, "Move ctor did not yield 10 elements");
    for (auto &oc : na)
        cr_assert_eq(oc, std::make_optional<imov_component>(1, 2));
}

Test(HexSparseArray, 01_moveOperatorAssignArrayOf10, .disabled = false) {
    auto sa = hex::containers::sparse_array<icop_component>{};

    for (int i = 0; i < 10; ++i)
        sa.insert_at(i, {1, 2});

    hex::containers::sparse_array<icop_component> na;
    na = std::move(sa);


    cr_assert_eq(na.size(), 10, "Move ctor did not yield 10 elements");
    for (auto &oc : na)
        cr_assert_eq(oc, std::make_optional<icop_component>(1, 2));
}

Test(HexSparseArray, 01_operatorAssignInitListOfOptional, .disabled = false) {
    auto sa = hex::containers::sparse_array<int>();

    sa = {
        std::optional<int>(0),
        std::optional<int>(1),
        std::optional<int>(2),
        std::optional<int>(3),
        std::optional<int>(4)
    };

    cr_assert_eq(sa.size(), 5);
    for (int i = 0; i < 5; ++i)
        cr_assert_eq(sa[i], std::optional{i});
}

Test(HexSparseArray, 01_operatorAssignInitListOfValue, .disabled = false) {
    auto sa = hex::containers::sparse_array<imov_component>();

    sa = {
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
    };

    cr_assert_eq(sa.size(), 5);
    for (auto &oc : sa)
        cr_assert_eq(oc, std::make_optional<imov_component>(1, 2));
}

Test(HexSparseArray, 02_assign10Nullopt) {
    hex::containers::sparse_array<ex_component> sa;

    sa.assign(10, std::nullopt);

    cr_assert_eq(sa.size(), 10);
    for (auto &oc : sa)
        cr_assert_eq(oc, std::nullopt);
}

Test(HexSparseArray, 02_assign10Optional) {
    hex::containers::sparse_array<ex_component> sa;

    sa.assign(10, std::optional(ex_component{1, 2}));

    cr_assert_eq(sa.size(), 10);

    for(auto &oc : sa)
        cr_assert_eq(oc, std::optional(ex_component{1, 2}));
}

Test(HexSparseArray, 02_assign10Value) {
    hex::containers::sparse_array<imov_component> sa;

    sa.assign(10, imov_component{1, 2});

    cr_assert_eq(sa.size(), 10);

    for(auto &oc : sa)
        cr_assert_eq(oc, std::make_optional<imov_component>(1, 2));
}

Test(HexSparseArray, 02_assignFromiteratorsOfOptional, .disabled = false) {
    auto v = std::vector{
        std::optional<ex_component>({1, 2}),
        std::optional<ex_component>({1, 2}),
        std::optional<ex_component>({1, 2}),
        std::optional<ex_component>({1, 2}),
        std::optional<ex_component>({1, 2}),
        std::optional<ex_component>({1, 2}),
        std::optional<ex_component>({1, 2}),
        std::optional<ex_component>({1, 2}),
        std::optional<ex_component>({1, 2}),
        std::optional<ex_component>({1, 2}),
    };

    hex::containers::sparse_array<ex_component> sa;

    sa.assign(v.begin(), v.end());

    cr_assert_eq(sa.size(), 10, "Assignment from iterator did not yield 10 elements");
    for (auto &oc : sa)
        cr_assert_eq(oc, std::optional<ex_component>({1, 2}));
}

Test(HexSparseArray, 02_assignFromiteratorsOfValue, .disabled = false) {
    auto v = std::vector{
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
    };

    hex::containers::sparse_array<imov_component> sa;
    sa.assign(v.begin(), v.end());

    cr_assert_eq(sa.size(), 10, "Assignment from iterator did not yield 10 elements");
    for (auto &oc : sa)
        cr_assert_eq(oc, std::make_optional<imov_component>(1, 2));
}

Test(HexSparseArray, 02_assignInitListOfOptional, .disabled = false) {
    auto sa = hex::containers::sparse_array<int>();

    sa.assign({
        std::optional<int>(0),
        std::optional<int>(1),
        std::optional<int>(2),
        std::optional<int>(3),
        std::optional<int>(4)
    });

    cr_assert_eq(sa.size(), 5);
    for (int i = 0; i < 5; ++i)
        cr_assert_eq(sa[i], std::optional{i});
}

Test(HexSparseArray, 02_assignInitListOfValue, .disabled = false) {
    auto sa = hex::containers::sparse_array<imov_component>();

    sa.assign({
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
        imov_component{1, 2},
    });

    cr_assert_eq(sa.size(), 5);
    for (auto &oc : sa)
        cr_assert_eq(oc, std::make_optional<imov_component>(1, 2));
}

Test(HexSparseArray, 03_getAllocatorWithDefaultAllocator, .disabled = false) {
    hex::containers::sparse_array<ex_component> sa;

    auto al = sa.get_allocator();

    cr_assert_eq(typeid(sparse_allocator<ex_component>{}), typeid(decltype(al)));
    cr_assert_eq(al, sparse_allocator<ex_component>{});
}

Test(HexSparseArray, 03_getAllocatorWithCustomAllocator, .disabled = false) {
    using Al = custom_allocator<std::optional<ex_component>>;
    hex::containers::sparse_array<ex_component, Al> sa;

    auto al = sa.get_allocator();

    cr_assert_eq(typeid(Al{}), typeid(decltype(al)));
    cr_assert_eq(al, Al{});
}

Test(HexSparseArray, 04_atInBound, .disabled = false) {
    {
        auto val = std::optional(ex_component{3, 4});
        auto val2 = std::optional(ex_component{3, 5});
        auto sa = hex::containers::sparse_array<ex_component>(10, val);

        auto &oc = sa.at(5);

        cr_assert_eq(oc, val);

        oc = val2;

        auto da = sa.data();

        cr_assert_eq(da[5], val2);
    }

    {
        auto val = std::optional(ex_component{3, 4});
        auto const sa = hex::containers::sparse_array<ex_component>(10, val);

        auto oc = sa.at(5);

        cr_assert_eq(oc, val);
    }
}

Test(HexSparseArray, 04_atOutOfBound, .disabled = false) {
    {
        auto val = std::optional(ex_component{3, 4});
        auto sa = hex::containers::sparse_array<ex_component>(3, val);

        cr_assert_throw(sa.at(5), std::out_of_range);
    }

    {
        auto val = std::optional(ex_component{3, 4});
        auto const sa = hex::containers::sparse_array<ex_component>(3, val);

        cr_assert_throw(sa.at(5), std::out_of_range);
    }
}

Test(HexSparseArray, 05_accessOperator, .disabled = false) {
    {
        auto val = std::optional(ex_component{3, 4});
        auto val2 = std::optional(ex_component{3, 5});
        auto sa = hex::containers::sparse_array<ex_component>(10, val);

        auto &oc = sa[5];

        cr_assert_eq(oc, val);

        oc = val2;

        auto da = sa.data();

        cr_assert_eq(da[5], val2);
    }

    {
        auto val = std::optional(ex_component{3, 4});
        auto const sa = hex::containers::sparse_array<ex_component>(10, val);

        auto oc = sa[5];

        cr_assert_eq(oc, val);
    }
}

Test(HexSparseArray, 06_dataReturnsPointerToData, .disabled = false) {
    {
        auto val = std::optional(ex_component{3, 4});
        auto sa = hex::containers::sparse_array<ex_component>(3, val);

        auto da = sa.data();

        cr_assert((std::is_same_v<decltype(da), hex::containers::sparse_array<ex_component>::pointer> ), "Bad type for non const call to data");
        cr_assert_neq(da, nullptr);
        cr_assert_not((std::is_const_v<std::remove_pointer_t<decltype(da)>>), "call to non const data returns const type");
    }

    {
        auto val = std::optional(ex_component{3, 4});
        auto const sa = hex::containers::sparse_array<ex_component>(3, val);

        auto da = sa.data();

        cr_assert((std::is_same_v<decltype(da), hex::containers::sparse_array<ex_component>::const_pointer> ), "Bad type for const call to data");
        cr_assert_neq(da, nullptr);
        cr_assert((std::is_const_v<std::remove_pointer_t<decltype(da)>>), "Call to const data returns non-const value.");
    }
}

Test(HexSparseArray, 07_getIndexValueSanityCheck) {
    hex::containers::sparse_array<int>::value_type opt(10);

    auto &optv = opt.value();

    cr_assert_eq((int *)std::addressof(opt), std::addressof(optv), "sparse_array optionals and their content are assumed to have the same address");
}

Test(HexSparseArray, 07_getIndexInBound, .disabled = false) {
    auto sa = hex::containers::sparse_array(10, std::optional{10});

    auto &oval = sa[5];
    auto &val = oval.value();

    cr_assert_eq(sa.get_index(oval), 5, "index from optional failed");
    cr_assert_eq(sa.get_index(val), 5, "index from direct ref failed");
}

Test(HexSparseArray, 07_getIndexOutOfBound, .disabled = false) {
    auto sa = hex::containers::sparse_array(10, std::optional{10});

    auto const &oval = std::optional(10);
    auto const &val = oval.value();

    cr_assert_throw((void)sa.get_index(oval), std::out_of_range);
    cr_assert_throw((void)sa.get_index(val), std::out_of_range);

    auto da = sa.data();
    auto const &bval = *(da - 1);

    cr_assert_throw((void)sa.get_index(bval), std::out_of_range);

    auto const &aval = *(da + 15);

    cr_assert_throw((void)sa.get_index(aval), std::out_of_range);
}

Test(HexSparseArray, 08_beginWorksAsExpected, .disabled = true) {}
Test(HexSparseArray, 09_cbeginWorksAsExpected, .disabled = true) {}
Test(HexSparseArray, 10_endWorksAsExpected, .disabled = true) {}
Test(HexSparseArray, 11_cendWorksAsExpected, .disabled = true) {}
Test(HexSparseArray, 12_emptyWorksAsExpected, .disabled = true) {}
Test(HexSparseArray, 13_sizeWorksAsExpected, .disabled = true) {}
Test(HexSparseArray, 14_maxSizeWorksAsExpected, .disabled = true) {}
Test(HexSparseArray, 15_reserveWorksAsExpected, .disabled = true) {}
Test(HexSparseArray, 16_capacityWorksAsExpected, .disabled = true) {}
Test(HexSparseArray, 17_shrinkWorksAsExpected, .disabled = true) {}
Test(HexSparseArray, 18_clearWorksAsExpected, .disabled = true) {}

Test(HexSparseArray, 19_CopyInsertAtInbound, .disabled = false) {
    auto sa = hex::containers::sparse_array<imov_component>(10);
    auto sz_pre = sa.size();

    auto val = imov_component{1, 3};
    auto op = std::make_optional<imov_component>(1, 4);

    cr_expect_eq(sa[4], std::nullopt);
    cr_expect_eq(sa[5], std::nullopt);

    auto it_op = sa.insert_at(4, op);
    auto it_val = sa.insert_at(5, val);

    cr_assert_eq(sa[4], op);
    cr_assert_eq(sa[5], val);
    cr_assert_eq(sa.begin() + 4, it_op);
    cr_assert_eq(sa.begin() + 5, it_val);

    cr_assert_eq(sa.size(), sz_pre);
}

Test(HexSparseArray, 19_CopyInsertAtOutOfBound, .disabled = false) {
    auto ini_op = std::make_optional<imov_component>(1, 2);
    auto sa = hex::containers::sparse_array<imov_component>(5, ini_op);

    auto sz_pre = sa.size();
    cr_expect_eq(sz_pre, 5);

    auto val = imov_component{1, 3};
    auto op = std::make_optional<imov_component>(1, 4);

    auto it_val = sa.insert_at(10, val);
    cr_assert_eq(sa.begin() + 10, it_val);

    auto it_op = sa.insert_at(11, op);
    cr_assert_eq(sa.begin() + 11, it_op);

    cr_assert_geq(sa.size(), 12);

    for (int i = 0; i < 5; ++i)
        cr_assert_eq(sa[i], ini_op);

    for (int i = 5; i < 10; ++i)
        cr_assert_eq(sa[i], std::nullopt);

    cr_assert_eq(sa[10], val);
    cr_assert_eq(sa[11], op);
}

Test(HexSparseArray, 19_MoveInsertAtInbound, .disabled = false) {
    auto sa = hex::containers::sparse_array<icop_component>(10);
    auto sz_pre = sa.size();

    auto val = icop_component{1, 3};
    auto op = std::optional(icop_component{1, 4});

    cr_expect_eq(sa[4], std::nullopt);
    cr_expect_eq(sa[5], std::nullopt);

    auto it_op = sa.insert_at(4, std::move(op));
    auto it_val = sa.insert_at(5, std::move(val));

    cr_assert_eq(sa[4], op);
    cr_assert_eq(sa[5], val);

    cr_assert_eq(sa.begin() + 4, it_op);
    cr_assert_eq(sa.begin() + 5, it_val);

    cr_assert_eq(sa.size(), sz_pre);
}

Test(HexSparseArray, 19_MoveInsertAtOutOfBound, .disabled = false) {
    auto ini_op = std::make_optional<icop_component>(1, 2);
    auto sa = hex::containers::sparse_array<icop_component>(5);

    for (int i = 0; i < 5; ++i) {
        sa[i] = std::move(ini_op);
    }

    auto sz_pre = sa.size();
    cr_expect_eq(sz_pre, 5);

    auto val = icop_component{1, 3};
    auto op = std::make_optional<icop_component>(1, 4);

    auto it_val = sa.insert_at(10, std::move(val));
    cr_assert_eq(sa.begin() + 10, it_val);

    auto it_op = sa.insert_at(11, std::move(op));
    cr_assert_eq(sa.begin() + 11, it_op);

    cr_assert_geq(sa.size(), 12);

    for (int i = 0; i < 5; ++i)
        cr_assert_eq(sa[i], ini_op);

    for (int i = 5; i < 10; ++i)
        cr_assert_eq(sa[i], std::nullopt);

    cr_assert_eq(sa[10], val);
    cr_assert_eq(sa[11], op);
}

Test(HexSparseArray, 20_buildEmplaceAtInbound, .disabled = false) {
    auto sa = hex::containers::sparse_array<icop_component>(10);
    auto sz_pre = sa.size();

    auto val = icop_component{1, 3};
    auto op = std::optional(icop_component{1, 4});

    cr_expect_eq(sa[4], std::nullopt);
    cr_expect_eq(sa[5], std::nullopt);

    auto it_op = sa.emplace_at(4, 1, 4);
    auto it_val = sa.emplace_at(5, 1, 3);

    cr_assert_eq(sa[4], op);
    cr_assert_eq(sa[5], val);

    cr_assert_eq(sa.begin() + 4, it_op);
    cr_assert_eq(sa.begin() + 5, it_val);

    cr_assert_eq(sa.size(), sz_pre);
}

Test(HexSparseArray, 20_buildEmplaceAtOutOfBound, .disabled = false) {
    auto ini_op = std::make_optional<icop_component>(1, 2);
    auto sa = hex::containers::sparse_array<icop_component>(5);

    for (int i = 0; i < 5; ++i) {
        sa[i] = std::move(ini_op);
    }

    auto sz_pre = sa.size();
    cr_expect_eq(sz_pre, 5);

    auto val = icop_component{1, 3};
    auto op = std::make_optional<icop_component>(1, 4);

    auto it_val = sa.emplace_at(10, 1, 3);
    cr_assert_eq(sa.begin() + 10, it_val);

    auto it_op = sa.emplace_at(11, 1, 4);
    cr_assert_eq(sa.begin() + 11, it_op);

    cr_assert_geq(sa.size(), 12);

    for (int i = 0; i < 5; ++i)
        cr_assert_eq(sa[i], ini_op);

    for (int i = 5; i < 10; ++i)
        cr_assert_eq(sa[i], std::nullopt);

    cr_assert_eq(sa[10], val);
    cr_assert_eq(sa[11], op);
}

Test(HexSparseArray, 20_CopyEmplaceAtInbound, .disabled = false) {
    auto sa = hex::containers::sparse_array<imov_component>(10);
    auto sz_pre = sa.size();

    auto val = imov_component{1, 3};
    auto op = std::make_optional<imov_component>(1, 4);

    cr_expect_eq(sa[4], std::nullopt);
    cr_expect_eq(sa[5], std::nullopt);

    auto it_op = sa.emplace_at(4, op);
    auto it_val = sa.emplace_at(5, val);

    cr_assert_eq(sa[4], op);
    cr_assert_eq(sa[5], val);
    cr_assert_eq(sa.begin() + 4, it_op);
    cr_assert_eq(sa.begin() + 5, it_val);

    cr_assert_eq(sa.size(), sz_pre);
}

Test(HexSparseArray, 20_CopyEmplaceAtOutOfBound, .disabled = false) {
    auto ini_op = std::make_optional<imov_component>(1, 2);
    auto sa = hex::containers::sparse_array<imov_component>(5, ini_op);

    auto sz_pre = sa.size();
    cr_expect_eq(sz_pre, 5);

    auto val = imov_component{1, 3};
    auto op = std::make_optional<imov_component>(1, 4);

    auto it_val = sa.emplace_at(10, val);
    cr_assert_eq(sa.begin() + 10, it_val);

    auto it_op = sa.emplace_at(11, op);
    cr_assert_eq(sa.begin() + 11, it_op);

    cr_assert_geq(sa.size(), 12);

    for (int i = 0; i < 5; ++i)
        cr_assert_eq(sa[i], ini_op);

    for (int i = 5; i < 10; ++i)
        cr_assert_eq(sa[i], std::nullopt);

    cr_assert_eq(sa[10], val);
    cr_assert_eq(sa[11], op);
}

Test(HexSparseArray, 20_MoveEmplaceAtInbound, .disabled = false) {
    auto sa = hex::containers::sparse_array<icop_component>(10);
    auto sz_pre = sa.size();

    auto val = icop_component{1, 3};
    auto op = std::optional(icop_component{1, 4});

    cr_expect_eq(sa[4], std::nullopt);
    cr_expect_eq(sa[5], std::nullopt);

    auto it_op = sa.emplace_at(4, std::move(op));
    auto it_val = sa.emplace_at(5, std::move(val));

    cr_assert_eq(sa[4], op);
    cr_assert_eq(sa[5], val);

    cr_assert_eq(sa.begin() + 4, it_op);
    cr_assert_eq(sa.begin() + 5, it_val);

    cr_assert_eq(sa.size(), sz_pre);
}

Test(HexSparseArray, 20_MoveEmplaceAtOutOfBound, .disabled = false) {
    auto ini_op = std::make_optional<icop_component>(1, 2);
    auto sa = hex::containers::sparse_array<icop_component>(5);

    for (int i = 0; i < 5; ++i) {
        sa[i] = std::move(ini_op);
    }

    auto sz_pre = sa.size();
    cr_expect_eq(sz_pre, 5);

    auto val = icop_component{1, 3};
    auto op = std::make_optional<icop_component>(1, 4);

    auto it_val = sa.emplace_at(10, std::move(val));
    cr_assert_eq(sa.begin() + 10, it_val);

    auto it_op = sa.emplace_at(11, std::move(op));
    cr_assert_eq(sa.begin() + 11, it_op);

    cr_assert_geq(sa.size(), 12);

    for (int i = 0; i < 5; ++i)
        cr_assert_eq(sa[i], ini_op);

    for (int i = 5; i < 10; ++i)
        cr_assert_eq(sa[i], std::nullopt);

    cr_assert_eq(sa[10], val);
    cr_assert_eq(sa[11], op);
}

Test(HexSparseArray, 21_eraseAt, .disabled = false) {
    auto ini = ex_component{1, 2};
    auto sa = hex::containers::sparse_array<ex_component>(10, ini);

    for (auto &oc : sa)
        cr_expect_eq(oc, std::optional(ex_component{1, 2}), "precondition failed : unexpected value");

    sa.erase_at(5);

    for (int i = 0; i < sa.size(); ++i) {
        if (i != 5)
            cr_assert_eq(sa[i], ini);
        else
            cr_assert_eq(sa[i], std::nullopt);
    }

    cr_assert_throw(sa.erase_at(15), std::out_of_range);
}

Test(HexSparseArray, 22_resizeWorksAsExpected, .disabled = true) {}
Test(HexSparseArray, 23_swapWorksAsExpected, .disabled = true) {}
