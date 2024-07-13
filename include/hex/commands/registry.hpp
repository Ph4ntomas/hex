#ifndef hex_mpi_commands_registry_hpp_
#define hex_mpi_commands_registry_hpp_

#include <any> //std::any
#include <optional> //std::optional, std::nullopt;
#include <unordered_map>

#include <hex/commands/types.hpp>

namespace hex::commands {
    /**
    ** Empty struct that represent a constructible void value.
    */
    struct void_t {};

    /**
    ** Utility to convert void to commands::void_t
    **
    ** This type is meant for internal use. It comes from a need to return a
    ** `std::optional` in hex::commands::refistry::try_call, which are not
    ** constructible from void.
    **
    ** This is wanted as c++23 adds monadic behavior.
    */
    template <typename T>
    struct no_void {
        using type = T;
    };

    template <>
    struct no_void<void> {
        using type = void_t;
    };

    /**
    ** \brief no_void "struct no_void" helper type.
    */
    template <typename T>
    using no_void_t = no_void<T>::type;

    /**
    ** \brief Commands registry.
    **
    ** This type holds commands handler, and allows to call them using a
    ** Command both as a strong-type token to differentiate between commands,
    ** and as the command parameter set.
    **
    ** The commands handler are first registered by a subsystem using
    ** set_handler_for.
    **
    ** A command may or may not, at any point have a registered handler.
    **
    ** They are to be used when a return value is needed, or if a specific
    ** sequence of operation should be respected.
    **
    ** The call command runs int the calling thread. For asynchronous
    ** operation, prefer using Events.
    **
    ** There can only be one handler per command (per registry).
    */
    class registry {
        template <Command C>
        using _command_functor = std::function<typename command_trait<C>::call_signature_t>;

        public:
            /**
            ** Set the handler for a Command type.
            **
            ** \param handler Callable that matches the signature of the Command,
            **
            ** \returns On success, returns a hex::commands::handle that can be used to
            ** unregister the command later on.
            ** On failure std::nullopt is returned.
            */
            template <commands::Command C, commands::CommandHandler<C> Callable>
            std::optional<commands::handle> set_handler_for(Callable && handler) {
                using Trait = commands::command_trait<C>;
                std::type_index idx = typeid(typename Trait::type_signature_t);

                auto [it, ok] = _callbacks
                                .try_emplace(idx,
                                             std::piecewise_construct,
                                             std::tuple{ commands::handle{idx} },
                                             std::tuple{
                                                std::in_place_type<_command_functor<C>>,
                                                std::forward<Callable>(handler)
                                             });

                if (ok)
                    return it->second.first;
                return std::nullopt;
            }

            /**
            ** Unset a handler for a command, using its handle.
            **
            ** This function remove the command handler and its handle from the
            ** pool of commands.
            **
            ** A subsequent call to call_command with the Command associated
            ** with this handle will fail.
            **
            ** \param handle \ref hex::commands::handle "Handle" associated
            ** with a registered command.
            **
            ** \return True if the handle was representing a registered command
            ** handler.
            */
            bool unset_handler(commands::handle const &handle) {
                if (_callbacks.contains(handle._ti) &&
                        _callbacks.at(handle._ti).first == handle) {
                        _callbacks.erase(handle._ti);
                        return true;
                }

                return false;
            }

            /**
            ** Call a command handler.
            **
            ** This function calls a command handler by applying the command
            ** parameter to the handler.
            **
            ** \return Returns the handler return value;
            */
            template <commands::Command C>
            decltype(auto) call(C && command) const {
                using Trait = commands::command_trait<C>;
                using return_type = Trait::result_type;

                auto const &[_, callback] = _callbacks.at(typeid(typename Trait::type_signature_t));
                auto const & fun = std::any_cast<_command_functor<C> const &>(callback);

                return std::apply(fun, std::forward<C>(command));
            }

            /**
            ** Try call a command handler.
            **
            ** This function first check if a handler is associated with a
            ** giver command, calling it if that was the case.
            **
            ** \return Returns the handler return value, or std::nullopt if
            ** the handler was not found.
            **
            ** \note If the command would have returned void, the return value of this
            ** function becomes std::optional<void_t>
            */
            template <commands::Command C>
            std::optional<commands::no_void_t<typename commands::command_trait<C>::result_type>>
            try_call_command(C &&command) const {
                using Trait = commands::command_trait<C>;
                using return_type = Trait::result_type;

                if (!_callbacks.contains(typeid(typename Trait::type_signature_t)))
                    return std::nullopt;

                if constexpr (std::is_same_v<void, return_type>) {
                    call_command(std::forward<C>(command));
                    return commands::void_t{};
                } else
                    return call_command(std::forward<C>(command));
            }

            /**
            ** Check if a handler was registered for a command
            **
            ** \tparam C Command to check.
            **
            ** \return True if the registry has a handler for that command.
            */
            template <commands::Command C>
            bool has_handler_for() {
                using Trait = command_trait<C>;
                return _callbacks.contains(typeid(typename Trait::type_signature_t));
            }
        private:
            std::unordered_map<std::type_index, std::pair<commands::handle, std::any>> _callbacks;
    };
}

#endif /* end of include guard: hex_mpi_commands_registry_hpp_ */
