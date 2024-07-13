#ifndef hex_mpi_commands_types_hpp_
#define hex_mpi_commands_types_hpp_

#include <functional>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <utility>

#include <hex/commands/forward.hpp>

namespace hex::commands {
    /**
    ** \brief Type trait class to provides uniform interface to the properties
    ** of Command types.
    **
    ** \tparam Command The Command type to retrieve properties of.
    **
    ** \par Nested Type
    ** | Nested type | Definition |
    ** | ----------- | ---------- |
    ** | call_signature_t | A function signature |
    ** | result_type | Type return by the command handler |
    ** | recipient_tag | A tag type for disambiguation |
    ** | type_signature_t | A unique type from which a std::type_index will be built |
    **
    ** If Command does not have `call_signature_t` or `recipient_tag_t`, this template
    ** has no members.
    **
    ** If Command has both `call_signature_t` and `recipient_tag` but not
    ** `type_signature_t`, a default `type_signature_t` is used.
    **
    ** The `result_type` is deduced from the `call_signature_t`.
    **
    */
    template <typename Command> struct command_trait;

    /**
    ** \brief Commands Implementation details
    */
    namespace __details {
        template <typename T>
        concept __Command = requires {
            typename T::call_signature_t;
            requires std::is_function_v<typename T::call_signature_t>;
            typename T::recipient_tag;
        };

        template <typename T>
        concept __WithTypeSignature = requires {
            typename T::type_signature_t;
        };

        template <typename T>
        concept __CommandWithTypeSignature = __Command<T> && __WithTypeSignature<T>;

        template <typename T>
        concept __CommandWithoutTypeSignature = __Command<T> && !__WithTypeSignature<T>;

        template <typename T, typename = void>
        struct __command_trait {};

        template <typename T> requires __details::__CommandWithoutTypeSignature<T>
        struct __command_trait<T, void> {
            using call_signature_t = typename T::call_signature_t;
            using recipient_tag = typename T::recipient_tag;
            using type_signature_t = std::tuple<recipient_tag, std::function<call_signature_t>>;
            using result_type = typename std::function<call_signature_t>::result_type;
        };

        template <typename T> requires __details::__CommandWithTypeSignature<T>
        struct __command_trait<T, void> {
            using call_signature_t = typename T::call_signature_t;
            using recipient_tag = typename T::recipient_tag;
            using type_signature_t = typename T::type_signature_t;
            using result_type = typename std::function<call_signature_t>::result_type;
        };
    }

    template <typename Command>
    struct command_trait: __details::__command_trait<Command> {};

    /**
    ** Specifies that a type is compatible with hex MPI commands api.
    **
    ** An arbitrary type T is considered a valid command if command_trait<T>
    ** has the following nested types:
    ** | Type | Note |
    ** | ---- | ---- |
    ** | command_trait<T>::call_signature | A valid function signature |
    ** | command_trait<T>::recipient_tag | Disambiguation tag |
    ** | type_signature_t | Unique type |
    **
    ** On top of this, it should be possible to `std::apply` the command object to a
    ** function with signature `call_signature_t`.
    **
    ** A Command-compatible type is used as a type identifier to find the
    ** associated handler, as well as an argument holder, hence the need to support
    ** std::apply.
    */
    template <typename T>
    concept Command = requires {
        typename command_trait<T>::call_signature_t;
        requires std::is_function_v<typename command_trait<T>::call_signature_t>;
        requires !std::is_reference_v<typename command_trait<T>::call_signature_t>;

        typename command_trait<T>::recipient_tag;
        typename command_trait<T>::type_signature_t;
    } && requires(std::function<typename command_trait<T>::call_signature_t> const &f, T && t) {
        std::apply(f, std::forward<T>(t));
    };

    /**
    ** Concept used as a constraint when registering a command handler.
    **
    ** This concept enforce that for a given command type C, an arbitrary Callable
    ** is a valid handler for that command, by ensuring we can store it in a
    ** std::function whose template parameter is the Command signature.
    */
    template <typename Callable, typename C>
        concept CommandHandler = Command<C> &&
        std::constructible_from<std::function<typename command_trait<C>::call_signature_t>, Callable>;

    template <typename Recipient, typename Signature>
    struct command {};

    /**
    ** Default command implementation.
    **
    ** This is the default command implementation. It provides the necessary
    ** types, and can be applied to a Handler.
    **
    ** This type is designed to be used in a using expression. For example:
    **
    ** \code{.cpp}
    ** struct texture {
    **   using load = hex::mpi::commands::command<texture, size_t(std::string_t)>;
    **   using destroy = hex::mpi::commands::command<texture, void(size_t)>;
    ** };
    ** \endcode
    **
    ** Once defined, a handler can be registered to that command using
    ** hex::commands::registry::set_handler_for
    **
    ** After a command handler has been registered, the command can be called
    ** using hex::commands::registry::call or hex::commands::registry::try_call
    */
    template <typename Recipient, typename R, class... Args>
    struct command<Recipient, R(Args...)> : std::tuple<Args...> {
        using call_signature_t = R(Args...);
        using recipient_tag = Recipient;

        using std::tuple<Args...>::tuple;
    };

    /**
    ** Command handle.
    **
    ** This type is returned by the
    ** \ref hex::commands::registry "command registry" whenever a command is
    ** registered. It's used to un-register a command.
    **
    ** A Command handle can be copied or moved, and compared. However they can
    ** only be constructed by a \ref hex::commands::registry "command registry"
    */
    class handle {
        inline static std::size_t handle_count = 0;
        friend hex::commands::registry;

        handle(std::type_index ti) : _ti(ti), _id(++handle_count) {}

        public:
        handle(handle const &) = default;
        handle(handle &&) = default;

        handle &operator=(handle const &) = default;
        handle &operator=(handle &&) = default;

        friend bool operator==(handle const &lhs, handle const &rhs) {
            return lhs._ti == rhs._ti && lhs._id == rhs._id;
        }

        private:
        std::type_index _ti;
        std::size_t _id;
    };
}

template <typename Recipient, typename R, class... Args>
struct std::tuple_size<hex::commands::command<Recipient, R(Args...)>>
: std::tuple_size<std::tuple<Args...>>
{};

template <size_t I, typename Recipient, typename R, class... Args>
struct std::tuple_element<I, hex::commands::command<Recipient, R(Args...)>>
: std::tuple_element<I, std::tuple<Args...>>
{};

// TODO: Find out if this is needed
//namespace std {
    //template <size_t Idx, typename Recipient, typename R, class... Args>
    //constexpr typename std::tuple_element_t<Idx, hex::commands::command<Recipient, R(Args...)>> &
    //get(hex::commands::command<Recipient, R(Args...)> & c) noexcept {
        //return get<Idx>(static_cast<std::tuple<Args...> &>(c));
    //}

    //template <size_t Idx, typename Recipient, typename R, class... Args>
    //constexpr typename std::tuple_element_t<Idx, hex::commands::command<Recipient, R(Args...)>> const &
    //get(hex::commands::command<Recipient, R(Args...)> const & c) noexcept {
        //return get<Idx>(static_cast<std::tuple<Args...> &>(c));
    //}

    //template <size_t Idx, typename Recipient, typename R, class... Args>
    //constexpr typename std::tuple_element_t<Idx, hex::commands::command<Recipient, R(Args...)>> &&
    //get(hex::commands::command<Recipient, R(Args...)> && c) noexcept {
        //using element_t = std::tuple_element_t<Idx, hex::commands::command<Recipient, R(Args...)>>;
        //return std::forward<element_t>(get<Idx>(static_cast<std::tuple<Args...> &&>(c)));
    //}

    //template <size_t Idx, typename Recipient, typename R, class... Args>
    //constexpr typename std::tuple_element_t<Idx, hex::commands::command<Recipient, R(Args...)>> const &&
    //get(hex::commands::command<Recipient, R(Args...)> const && c) noexcept {
        //using element_t = std::tuple_element_t<Idx, hex::commands::command<Recipient, R(Args...)>>;
        //return std::forward<element_t const>(get<Idx>(static_cast<std::tuple<Args...> const &&>(c)));
    //}
//}

#endif /* end of include guard: hex_mpi_commands_hpp_ */
