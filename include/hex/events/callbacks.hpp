#ifndef hex_mpi_events_callbacks_hpp_
#define hex_mpi_events_callbacks_hpp_

#include <any> // std::any
#include <functional>
#include <typeindex>

#include <hex/events/forward.hpp>
#include <hex/utils/metaprog.hpp>
#include <hex/events/utils.hpp>

namespace hex::events {
    template < typename Callable, typename Event>
    concept EventCallback = std::is_invocable_r_v<void, Callable, Event>;
}

namespace hex::events::callbacks {
    /**
    ** \brief controls the policy for dispatching callbacks.
    */
    enum class dispatch_policy {
        async, /**< Event are to be ran asynchronously. */
        trigger, /**< Event are handled lazily at a later point. */
        sync, /**< Event are handled as soon as they are dispatched, on the calling thread */
    };

    using utils::copy_cvref_t;
    class container;

    /**
    ** \brief Callback handle.
    **
    ** This opaque type is used to identify a callback.
    ** when un-registering it.
    **
    ** It can be copied and move, but cannot be constructed normally.
    */
    class handle {
        friend hex::events::callbacks::container;

        handle(std::type_index ti, std::size_t id): _ti(ti), _id(id) {}

        public:
        handle(handle const &) = default;
        handle(handle &&) = default;

        handle &operator=(handle const &) = default;
        handle &operator=(handle &&) = default;

        friend bool operator==(handle const &lhs, handle const &rhs) {
            return lhs._ti == rhs._ti && lhs._id == rhs._id;
        }

        std::type_index type_index() const {
            return _ti;
        }

        private:
        std::type_index _ti;
        std::size_t _id;
    };

    /**
    ** \brief Contains callback for a given event type in a type-erased way.
    **
    ** This type holds every callbacks for a given Event type. This allows
    ** the \ref hex::events::dispatcher "dispatcher" to holds every events
    ** callbacks in a single container.
    **
    ** \note This is meant for internal use only.
    */
    class container {
        /**
        ** \brief Callback storage type.
        **
        ** Holds a callback for a given event, as well as a copy of its handle.
        **
        ** \note This is meant for internal use only.
        */
        template <typename Event>
        struct functor {
            handle hndl;
            std::function<void(Event)> callback;

            /**
            ** \brief construct the functor.
            */
            template <EventCallback<Event> Callable> functor(handle _hndl, Callable && c)
                : hndl(_hndl), callback(std::forward<Callable>(c)) {}

            /**
            ** \brief Call the held callback, with a copy of the event.
            */
            void operator()(Event e) const {
                callback(e);
            }
        };

        /**
        ** \brief internal container implementation.
        */
        template <typename EventType>
        using container_impl = std::vector<functor<EventType>>;

        /**
        ** \name Helper functions
        */
        /**@{*/
        /**
        ** \brief casts the container implementation back to its non
        ** type-erased form.
        **
        ** \tparam Event Type of the event the callbacks are handling.
        ** \tparam Any Type of the queue. Used to cast with the same set of
        ** cv-qualifiers.
        **
        ** \param container container to cast.
        **
        ** \return Return the cast container.
        */
        template <typename Event, typename Any>
        static copy_cvref_t<container_impl<Event>, Any> _cast_callback(Any &&container) {
            return std::any_cast<copy_cvref_t<container_impl<Event>, Any>>(std::forward<Any>(container));
        }

        /**
        ** \brief unregister the callback corresponding to the handle.
        **
        ** \tparam Event The type of Event the callback are meant for.
        **
        ** \param hndl Callback handle.
        **
        ** \return true if the callback was removed.
        */
        template <typename Event>
        static std::size_t _unregister_impl(container &m, handle hndl) {
            auto &callbacks = _cast_callback<Event>(m._callbacks);

            std::erase_if(callbacks, [&](auto cb){ return cb.hndl == hndl; });

            return callbacks.size();
        }

        using unregister_f = std::size_t(container &, handle);

        /**
        ** \brief dispatch an event stored in a std::any.
        **
        ** This is called when callback::dispatch is called with a
        ** std::any.
        */
        template <typename Event>
        static void _dispatch_any_impl(container const &c, std::any const & ev) {
            return c.dispatch(std::any_cast<Event const &>(ev));
        }
        using dispatch_any_f = void(container const &, std::any const &);
        /**@}*/

        public:
        template <typename Event>
        container(for_event_t<Event>) :
            _unregister(_unregister_impl<Event>),
            _dispatch_any(_dispatch_any_impl<Event>),
            _count(0),
            _callbacks(std::in_place_type<std::vector<functor<Event>>>)
        {}

        container(container const &) = delete;
        container(container &&) = default;

        container &operator=(container const &) = delete;
        container &operator=(container &&) = default;

        /**
        ** \brief dispatch a single event.
        **
        ** Upon dispatch, every callback is called, in the order in which they
        ** were registered.
        **
        ** \tparam Event Type of the event to be dispatched.
        **
        ** \param ev Event to be dispatched.
        */
        template <typename Event>
        void dispatch(Event const &ev) const {
            auto const &callbacks = _cast_callback<Event>(_callbacks);

            for (auto const &f : callbacks)
                f(ev);
        }

        /**
        ** \brief dispatch a type-erased event.
        **
        ** This overload will call the templated overload, after casting the
        ** event to its actual type.
        **
        ** \param ev std::any wrapping an event.
        */
        void dispatch(std::any const &ev) const {
            _dispatch_any(*this, ev);
        }

        /**
        ** \brief register a single callback for a given event type.
        **
        ** \tparam Event Type of event the callback manages.
        ** \tparam Callable Callable type that can handle the event.
        **
        ** \param c Callback to register.
        **
        ** \return \ref hex::events::callbacks::handle "Handle" to the callback.
        */
        template <typename Event, EventCallback<Event> Callable>
        handle register_callback(Callable && c) {
            std::vector<functor<Event>> & callbacks = _cast_callback<Event>(_callbacks);

            functor<Event> &cb = callbacks.emplace_back(
                    handle{typeid(Event), ++_count},
                    std::forward<Callable>(c));

            return cb.hndl;
        }

        /**
        ** \brief remove the callback corresponding to this handle
        **
        ** \param hndl The handle to the callback to remove.
        **
        ** \return Number of elements left.
        */
        std::size_t unregister(handle hndl) {
            return _unregister(*this, hndl);
        }

        private:
        unregister_f * _unregister;
        dispatch_any_f * _dispatch_any;

        std::size_t _count; /**< number of callbacks added. */
        std::any _callbacks; /**< type-erased list of callbacks */
    };
}

#endif /* end of include guard: hex_mpi_events_callbacks_hpp_ */
