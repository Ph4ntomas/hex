#ifndef hex_mpi_events_dispatcher_hpp_
#define hex_mpi_events_dispatcher_hpp_

#include <future>
#include <optional>
#include <typeindex>
#include <unordered_map>

#include <hex/events/callbacks.hpp>
#include <hex/events/queues.hpp>

namespace hex::events {
    /**
    ** \brief Tags to control event handling flavor
    */
    namespace kind {
        /**
        ** \brief Tag type used to \ref dispatcher::declare "declare" an event
        ** type that should be handled by callbacks.
        */
        template <typename Event>
        struct callback_t {};

        /**
        ** \brief value helper for callback_t.
        */
        template <typename Event>
        static constexpr callback_t<Event> callback{};

        /**
        ** \brief Tag type used to \ref dispatcher::declare "declare" an event
        ** type that should be polled.
        */
        template <typename Event>
        struct polling_t {};

        /**
        ** \brief value helper for polling_t.
        */
        template <typename Event>
        static constexpr polling_t<Event> polling{};
    }

    enum class event_kind {
        callback, /**< Callback based event. */
        polling /**< Polling based event. */
    };

    /**
    ** \brief Event dispatcher.
    */
    class dispatcher {
        public:
            dispatcher() {}
            dispatcher(dispatcher &&other) = default;
            dispatcher & operator=(dispatcher && other) = default;

            /**
            ** \brief Declare a callback-style event.
            **
            ** \tparam Event type of the events.
            **
            ** \param policy event \ref callbacks::dispatch_policy "callback policy"
            */
            template <typename Event>
            bool declare(kind::callback_t<Event>, callbacks::dispatch_policy policy = callbacks::dispatch_policy::async) {
                std::type_index idx = typeid(Event);
                auto [it, ok] = _event_kinds.try_emplace(idx, event_kind::callback);

                if (ok) {
                    _callbacks.try_emplace(idx, for_event<Event>);
                    _callbacks_policies.try_emplace(idx, policy);
                }

                return ok || it->second == event_kind::callback;
            }

            /**@{*/
            /**
            ** \brief Declare a polling-style event.
            **
            ** \tparam Event type of the event.
            ** \tparam P PendingPolicy to be applied on event_queue::clear().
            ** \tparam S ShrinkPolicy to be applied on event_queue::clear().
            **
            ** \return If no error happens, returns true.
            */
            template <typename Event,
                     queues::policies::PendingPolicy P,
                     queues::policies::ShrinkPolicy S>
            bool declare(kind::polling_t<Event>, P p, S s) {
                return _declare_polling(for_event<Event>, p, s);
            }

            template <typename Event>
            bool declare(kind::polling_t<Event>) {
                return _declare_polling(for_event<Event>,
                            queues::policies::clear_pending,
                            queues::policies::no_shrink
                        );
            }

            template <typename Event, queues::policies::PendingPolicy P>
            bool declare(kind::polling_t<Event>, P p) {
                return _declare_polling(for_event<Event>, p, queues::policies::no_shrink);
            }

            template <typename Event, queues::policies::ShrinkPolicy S>
            bool declare(kind::polling_t<Event>, S s) {
                return _declare_polling(for_event<Event>,
                                        queues::policies::clear_pending,
                                        s);
            }
            /**@}*/

            /**
            ** \brief Register a callback for a given event type.
            **
            ** This function setup a Callback style events. When an events will
            ** be dispatched, every callback will be called, in the order in
            ** which they were registered.
            **
            ** \tparam Event Type that will be used when dispatching an event.
            ** \tparam Callback Callback returning void and taking Event as
            ** it's single parameter.
            **
            ** \return An opaque handle, used to un-register the callback, if
            ** the registration occurs.
            */
            template <typename Event, EventCallback<Event> Callback>
            std::optional<hex::events::callbacks::handle> register_callback_for(Callback && cb) {
                std::type_index idx = typeid(Event);

                if (!_event_kinds.contains(idx) || _event_kinds[idx] != event_kind::callback)
                    return std::nullopt;

                auto [it, ok] = _callbacks.try_emplace(typeid(Event), events::for_event<Event>);

                return it->second.template register_callback<Event>(std::forward<Callback>(cb));
            }

            template <typename Event, EventCallback<Event> Callback>
            std::optional<hex::events::callbacks::handle> register_callback_for(Callback && cb, callbacks::dispatch_policy pol) {
                std::type_index idx = typeid(Event);

                if (!declare(kind::callback<Event>, pol)) { return std::nullopt; }

                auto [it, ok] = _callbacks.try_emplace(typeid(Event), events::for_event<Event>);

                return it->second.template register_callback<Event>(std::forward<Callback>(cb));
            }

            /**
            ** \brief un-registers an event callback.
            **
            ** \param handle hex::events::callbacks::handle to the callback to
            ** remove.
            **
            ** \return Return true if a callback was unregistered
            */
            dispatcher & unregister_callback(hex::events::callbacks::handle handle) {
                std::type_index idx = handle.type_index();

                if (!_callbacks.contains(idx)) {
                    return *this;
                }

                auto &container = _callbacks.at(idx);

                container.unregister(handle);

                return *this;
            }

            /**
            ** \brief set the "callback policy" for an event.
            **
            ** \tparam Event event type to set policy of.
            */
            template <typename Event>
            dispatcher & set_policy_for(callbacks::dispatch_policy pol) {
                std::type_index idx = typeid(Event);

                _callbacks_policies[idx] = pol;

                return *this;
            }

            /**
            ** \brief trigger callbacks for an event type.
            **
            ** \tparam Event The event type to trigger callbacks for.
            */
            template <typename Event>
            dispatcher const & trigger(for_event_t<Event>) const {
                std::type_index idx = typeid(Event);

                for (auto const &f : _pending_trigger[idx]) {
                    f.wait();
                }

                _pending_trigger.erase(idx);

                return *this;
            }

            /**
            ** \brief wait all callback events with async policy.
            */
            dispatcher const & wait_async() const {
                for (auto const &f: _pending_callbacks) {
                    f.wait();
                }

                _pending_callbacks.clear();

                return *this;
            }

            /**
            ** \brief dispatch an event.
            **
            ** This function takes an
            **
            ** \tparam Event type of the event to dispatch
            ** \param ev event to dispatch
            */
            template <typename Event>
            dispatcher const & dispatch(Event const &ev) const {
                std::type_index idx = typeid(Event);

                if (!_event_kinds.contains(idx)) { return *this; }

                event_kind k = _event_kinds.at(idx);

                if (k == event_kind::callback && _callbacks.contains(idx)) {
                    _dispatch_callback(ev);
                } else if (auto queue = _queues.find(idx); queue != _queues.end()) {
                    queue->second.push(ev);
                }

                return *this;
            }

            /**
            ** \brief dispatch a type-erased event.
            **
            ** This overload is equivalent to the templated dispatch, with the exception that
            ** the event is wrapped in a std::any.
            ** This is useful if the type of the event is not known at compile-time.
            **
            ** \param ev event to dispatch, wrapped in a std::any.
            */
            dispatcher const & dispatch(std::any const &ev) const {
                std::type_index idx = ev.type();

                if (!_event_kinds.contains(idx)) { return *this; }

                event_kind k = _event_kinds.at(idx);

                if (k == event_kind::callback && _callbacks.contains(idx)) {
                    _dispatch_callback(ev);
                } else if (auto queue = _queues.find(idx); queue != _queues.end()) {
                    queue->second.push(ev);
                }

                return *this;
            }

            /**
            ** \brief poll a single event.
            */
            template <typename Event>
            std::optional<Event> poll() {
                std::type_index idx = typeid(Event);

                auto const &kv = _event_kinds.find(idx);
                if (kv == _event_kinds.end() || kv->second != event_kind::polling) {
                    std::abort();
                    return std::nullopt;
                }

                return _queues.at(idx).poll_one<Event>();
            }

        private:
            /**
            ** \brief dispatch a callback-style event.
            */
            template <typename Event>
            void _dispatch_callback(Event const &ev) const {
                std::type_index idx = typeid(Event);

                auto const &container = _callbacks.at(idx);
                callbacks::dispatch_policy pol = _callbacks_policies.at(idx);

                auto lambda = [&, ev]{ container.dispatch(ev); };

                if (pol == callbacks::dispatch_policy::sync) {
                    container.dispatch(ev);
                } else if (pol == callbacks::dispatch_policy::trigger) {
                    _pending_callbacks.push_back(std::async(std::launch::deferred, lambda));
                } else {
                    _pending_callbacks.push_back(std::async(std::launch::async, lambda));
                }
            }

            /**
            ** \brief Declare an event as polling-style event.
            **
            ** \tparam Event event Type.
            ** \tparam P A PendingPolicy, controlling the behavior of
            ** event_queue::clear()
            ** \tparam S A ShrinkPolicy, controlling the behavior of
            ** event_queue::clear().
            */
            template <typename Event,
                      queues::policies::PendingPolicy P,
                      queues::policies::ShrinkPolicy S>
            bool _declare_polling(for_event_t<Event>, P, S) {
                std::type_index idx = typeid(Event);
                auto [it, ok] = _event_kinds.try_emplace(idx, event_kind::polling);

                if (ok) {
                    _queues.try_emplace(idx, for_event<Event>, P{}, S{});
                }

                return ok || it->second == event_kind::polling;
            }


        private:
            std::unordered_map<std::type_index, event_kind> _event_kinds;

            std::unordered_map<std::type_index, callbacks::container> _callbacks;
            std::unordered_map<std::type_index, callbacks::dispatch_policy> _callbacks_policies;
            mutable std::unordered_map<std::type_index, std::vector<std::future<void>>> _pending_trigger;
            mutable std::vector<std::future<void>> _pending_callbacks;

            mutable std::unordered_map<std::type_index, queues::event_queue> _queues;
    };
}

#endif /* end of include guard: hex_mpi_events_dispatcher_hpp_ */
