/**
** \file type_traits.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-12-05 17:18
** \date Last update: 2021-12-05 17:21
*/

#ifndef meta_TYPE_TRAITS_HPP_
#define meta_TYPE_TRAITS_HPP_

#include <iterator> //std::iterator_traits, std::input_iterator_tag
#include <optional> //std::optional
#include <type_traits> //std::is_convertible_v, std::false_type, std::true_type

namespace hex::meta {
    template <class Iter>
    using iterator_category_t = typename std::iterator_traits<Iter>::iterator_category;

    template <class Iter>
    using require_input_iterator = std::enable_if_t<std::is_convertible_v<iterator_category_t<Iter>, std::input_iterator_tag>>;

    template <typename>
    struct is_optional : std::false_type {};

    template <typename T>
    struct is_optional<std::optional<T>> : std::true_type {};

    template <typename T>
    static constexpr bool is_optional_v = is_optional<T>::value;
}

#endif /* end of include guard: meta_TYPE_TRAITS_HPP_ */
