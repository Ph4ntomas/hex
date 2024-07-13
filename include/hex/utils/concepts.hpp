#ifndef hex_mpi_concepts_hpp_
#define hex_mpi_concepts_hpp_

#include <concepts>

namespace hex::concepts {
    template <typename T, typename ... Of>
    concept one_of = (std::same_as<T, Of> || ...);
}

#endif /* end of include guard: hex_mpi_concepts_hpp_ */
