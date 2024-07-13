#ifndef hex_utils_meta_hpp_
#define hex_utils_meta_hpp_

#include <type_traits>

namespace hex::utils {
    /**
    ** \brief copy cv-qualifiers and reference from Ref to T
    **
    ** \tparam T target type that will have cv qualifier and ref added.
    ** \tparam Ref reference type, from which to copy cv-qualifiers and ref.
    */
    template <typename T, typename Ref> struct copy_cvref {
        using type = std::remove_cvref_t<T>;
    };

    template <typename T, typename Ref> struct copy_cvref<T, Ref &> {
        using clean_type = std::remove_cvref_t<T>;
        using type = std::add_lvalue_reference_t<clean_type>;
    };

    template <typename T, typename Ref> struct copy_cvref<T, Ref &&> {
        using clean_type = std::remove_cvref_t<T>;
        using type = std::add_rvalue_reference_t<clean_type>;
    };

    template <typename T, typename Ref> struct copy_cvref<T, Ref const &> {
        using clean_type = std::remove_cvref_t<T>;
        using type = std::add_lvalue_reference_t<std::add_const_t<clean_type>>;
    };

    template <typename T, typename Ref> struct copy_cvref<T, Ref const &&> {
        using clean_type = std::remove_cvref_t<T>;
        using type = std::add_rvalue_reference_t<std::add_const_t<clean_type>>;
    };

    template <typename T, typename Ref> struct copy_cvref<T, Ref volatile &> {
        using clean_type = std::remove_cvref_t<T>;
        using type = std::add_lvalue_reference_t<std::add_volatile_t<clean_type>>;
    };

    template <typename T, typename Ref> struct copy_cvref<T, Ref volatile &&> {
        using clean_type = std::remove_cvref_t<T>;
        using type = std::add_rvalue_reference_t<std::add_volatile_t<clean_type>>;
    };

    template <typename T, typename Ref> struct copy_cvref<T, Ref const volatile &> {
        using clean_type = std::remove_cvref_t<T>;
        using type = std::add_lvalue_reference_t<std::add_cv_t<clean_type>>;
    };

    template <typename T, typename Ref> struct copy_cvref<T, Ref const volatile &&> {
        using clean_type = std::remove_cvref_t<T>;
        using type = std::add_rvalue_reference_t<std::add_cv_t<clean_type>>;
    };

    /**
    ** \brief helper type for copy_cvref
    */
    template <typename T, typename Ref>
    using copy_cvref_t = typename copy_cvref<T, Ref>::type;
}

#endif /* end of include guard: hex_utils_meta_hpp_ */
