#ifndef hex_mpi_utils_tuple_hpp_
#define hex_mpi_utils_tuple_hpp_

#include <concepts> // same_as, convertible_to
#include <utility> // tuple_element, tuple_size, get, index_sequence

namespace hex::utils {
    template <class T, std::size_t N>
    concept IsTupleElement = requires(T t) {
        typename std::tuple_element_t<N, std::remove_const_t<T>>;
        { get<N>(t) } -> std::convertible_to<const std::tuple_element_t<N, T>&>;
    };

    template <typename T>
    concept TupleLike = !std::is_reference_v<T> &&
        requires {
            typename std::tuple_size<T>::type;
            requires std::same_as<decltype(std::tuple_size_v<T>), size_t>; } &&
        []<std::size_t... I>(std::index_sequence<I...>)
        { return (IsTupleElement<T, I> && ...); }(std::make_index_sequence<std::tuple_size_v<T>>{});
}

#endif /* end of include guard: hex_mpi_utils_tuple_hpp_ */
