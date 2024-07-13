#ifndef hex_mpi_events_queues_hpp_
#define hex_mpi_events_queues_hpp_

#include <any> // std::any, std::any_cast
#include <algorithm>
#include <iterator>
#include <optional>
#include <vector>

#include <hex/events/utils.hpp>

#include <hex/utils/concepts.hpp>
#include <hex/utils/metaprog.hpp>

namespace hex::events::queues {
    using utils::copy_cvref_t;
    using concepts::one_of;

    /**
    ** \brief Tag helpers for event_queue
    */
    namespace policies {
        /**
        ** \brief If used, the event_queue::clear function will only remove
        ** handled events.
        */
        struct keep_pending_t {};
        static constexpr keep_pending_t keep_pending;

        /**
        ** \brief If used, the event_queue::clear function will remove all
        ** events.
        */
        struct clear_pending_t {};
        static constexpr clear_pending_t clear_pending;

        template <typename T>
        concept PendingPolicy = one_of<T, keep_pending_t, clear_pending_t>;

        /**
        ** \brief If used, the event_queue::clear function will shrink the
        ** queue's underlying container.
        */
        struct shrink_t {};
        static constexpr shrink_t shrink{};

        /**
        ** \brief If used, the event_queue::clear function will not shrink the
        ** queue's underlying container.
        */
        struct no_shrink_t {};
        static constexpr no_shrink_t no_shrink{};

        template <typename T>
        concept ShrinkPolicy = one_of<T, shrink_t, no_shrink_t>;
    }

    /**
    ** \brief Type-erased event queue.
    **
    ** This type holds all queued events for a single event type.
    **
    ** Events can be \ref event_queue::push "pushed" to the end of the queue,
    ** or polled, using poll_one or poll_all. Once polled, an event is
    ** considered handled, and cannot be retrieved. It will be deleted when
    ** event_queue::clear will be called.
    **
    ** By default, the event_queue::clear remove all events from the event
    ** queue. This behavior can be changed by passing
    ** clear_policies::keep_pending to the queue constructor.
    */
    class event_queue {
        /**
        ** \brief internal container implementation.
        */
        template <typename Event>
        using container_impl = std::vector<Event>;


        /**
        ** \name Helper functions
        */
        /**@{*/
        /**
        ** \brief casts the internal container back to its original type.
        **
        ** \tparam Event Type of the stored events.
        ** \tparam Any Type of the queue. Used to cast with the same set of
        ** cv-qualifiers.
        **
        ** \param queue container to cast.
        **
        ** \return Return the cast container.
        */
        template <typename Event, typename Any>
        static copy_cvref_t<container_impl<Event>, Any> _cast_queue(Any &&queue) {
            return std::any_cast<copy_cvref_t<container_impl<Event>, Any>>(std::forward<Any>(queue));
        }

        /**
        ** \brief clear the queue.
        **
        ** Clearing the queue remove all handled events. If the
        ** keep_on_clear flag is unset, pending events are cleared too.
        **
        ** \tparam Event queue event type.
        **
        ** \param queue event queue to clear.
        */
        template <typename Event, policies::PendingPolicy P, policies::ShrinkPolicy S>
        static void _clear_impl(event_queue &queue) {
            std::vector<Event> &vec = _cast_queue<Event>(queue._queue);

            if constexpr (!std::is_same_v<P, policies::keep_pending_t>) {
                vec.clear();
            } else {
                vec.erase(vec.begin(), vec.begin() + queue._next);
            }

            if constexpr (std::is_same_v<S, policies::shrink_t>) {
                vec.shrink_to_fit();
            }

            queue._next = 0;
        }
        using clear_f = void(event_queue &);

        /**
        ** \brief push an event wrapped in a std::any.
        */
        template <typename Event>
        static void _push_any_impl(event_queue &e, std::any const &ev) {
            e.push(std::any_cast<Event const &>(ev));
        }
        using push_any_f = void(event_queue&, std::any const &ev);
        /**@}*/

        public:
        /**@{*/
        /**
        ** \brief construct an event_queue
        **
        ** \tparam Event type of event that this queue will hold
        **
        ** If clear_policies::keep_pending is passed to the constructor, the
        ** clear function will only remove handled events from the queue
        ** underlying container.
        **
        ** If clear_policies::shrink is passed to the constructor, the clear
        ** function will additionally shrink the underlying container.
        */
        template <typename Event, policies::PendingPolicy P, policies::ShrinkPolicy S>
        event_queue(for_event_t<Event>, P, S) :
            _queue(std::in_place_type<container_impl<Event>>),
            _next(0),
            _clear(_clear_impl<Event, P, S>),
            _push_any(_push_any_impl<Event>)
        {}
        /**@}*/

        /**
        ** \brief deleted copy constructor
        */
        event_queue(event_queue const &) = delete;
        /**
        ** \brief default move constructor.
        */
        event_queue(event_queue &&) = default;

        /**
        ** \brief deleted copy assignment operator.
        */
        event_queue & operator=(event_queue const &) = delete;
        /**
        ** \brief default move assignment operator.
        */
        event_queue & operator=(event_queue &&) = default;

        /**
        ** \brief retrieve a pending event, if any.
        **
        ** \tparam Event Type of the event to retrieve.
        **
        ** \return If any event was in the queue, the first one is returned.
        ** std::nullopt is return if the queue was empty.
        */
        template <typename Event>
        std::optional<Event> poll_one() {
            std::vector<Event> &vec = _cast_queue<Event>(_queue);

            if (vec.size() <= _next)
                return std::nullopt;

            return vec[_next++];
        }

        /**
        ** \brief return all event in this queue, as a vector.
        **
        ** \tparam Event queue events.
        **
        ** \return Return a newly constructed vector, containing all pending
        ** events in the queue.
        */
        template <typename Event>
        std::vector<Event> poll_all() {
            std::vector<Event> &vec = _cast_queue<Event>(_queue);
            std::vector<Event> ret{};

            std::move(vec.begin() + _next, vec.end(), std::back_inserter(ret));

            vec.clear();
            _next = 0;

            return ret;
        }

        /**
        ** \brief append an event to the queue.
        **
        ** \tparam Event type of the pushed event.
        **
        ** \param ev event to append to the queue.
        */
        template <typename Event>
        void push(Event const &ev) {
            std::vector<Event> &vec = _cast_queue<Event>(_queue);

            vec.push_back(ev);
        }

        void push(std::any const &ev) {
            _push_any(*this, ev);
        }

        /**
        ** \brief clear the event queue.
        **
        ** By default, clearing a queue remove all events.
        **
        ** If the queue was constructed with policies::keep_pending,
        ** only events that have been returned by a previous call to poll_one
        ** or poll_all are removed.
        */
        void clear() {
            _clear(*this);
        }

        private:
            std::any _queue;
            std::size_t _next;
            clear_f * _clear;
            push_any_f* _push_any;
    };
}

#endif /* end of include guard: hex_mpi_events_queue_hpp_ */
