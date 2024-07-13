#ifndef hex_mpi_events_utils_hpp_
#define hex_mpi_events_utils_hpp_

namespace hex::events {
    /**
    ** \brief Tag type used to help deducing function template parameter.
    */
    template <typename Event>
    struct for_event_t {};

    /**
    ** \brief value helper for for_event_t.
    */
    template <typename Event>
    static constexpr for_event_t<Event> for_event {};
}

#endif /* end of include guard: hex_mpi_events_utils_hpp_ */
