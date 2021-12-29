/**
** \file system_registry.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-22 10:59
** \date Last update: 2021-12-29 18:32
*/

#ifndef SYSTEM_REGISTRY_HPP_
#define SYSTEM_REGISTRY_HPP_

#include <functional> // std::function
#include <memory> // std::shared_ptr
#include <stdexcept> // std::invalid_argument
#include <string> // std::string_literals;
#include <type_traits> // std::enable_if, std::disjunction, std::is_same, std::negation_v, std::remove_cv_t, std::remove_reference_t
#include <utility> // std::forward
#include <vector> // std::vector

#include "hex/components_registry.hpp"
#include "hex/entity_manager.hpp"
#include "hex/exceptions/unimplemented.hpp"
#include "hex/exceptions/no_such_component.hpp"

namespace hex {
    class system_registry;

    /**
    ** \cond Internals
    */
    namespace __impl {
        template <typename T> struct Getter {};

        template <> struct Getter<system_registry> { inline static system_registry &get(system_registry &sr, entity_manager & em, components_registry &cr) { return sr; } };
        template <> struct Getter<entity_manager> { inline static entity_manager &get(system_registry &sr, entity_manager & em, components_registry &cr) { return em; } };
        template <> struct Getter<components_registry> { inline static components_registry &get(system_registry &sr, entity_manager & em, components_registry &cr) { return cr; } };
        template <typename T> struct Getter<containers::sparse_array<T>> { inline static containers::sparse_array<T> &get(system_registry &sr, entity_manager & em, components_registry &cr) { return cr.get<T>(); } };

        template <typename T> struct argument_helper {
            using type = hex::containers::sparse_array<T>;
        };

        template <typename T>
        using argument_helper_t = typename argument_helper<T>::type;

        template <typename T> struct argument_helper<hex::containers::sparse_array<T>> {
            using type = hex::containers::sparse_array<T>;
        };

        template <typename T, typename Allocator> struct argument_helper<hex::containers::sparse_array<T, Allocator>> {
            using type = hex::containers::sparse_array<T, Allocator>;
        };

        template <> struct argument_helper<hex::entity_manager> {
            using type = hex::entity_manager;
        };

        template <> struct argument_helper<hex::components_registry> {
            using type = hex::components_registry;
        };

        template <> struct argument_helper<hex::system_registry> {
            using type = hex::system_registry;
        };

        template <typename, bool> struct sys_args_deduction_helper {};

        template <typename> struct sys_signature {};

        template <typename ...As>
        struct sys_signature<void(*)(As...)> {
            using type = void (As...);
            static constexpr bool constness = true;
            static constexpr sys_args_deduction_helper<type, constness> helper{};
        };

        template <typename T, typename ...As>
        struct sys_signature<void (T::*)(As...)> {
            using type = void (As...);
            static constexpr bool constness = false;
            static constexpr sys_args_deduction_helper<type, constness> helper{};
        };

        template <typename T, typename ...As>
        struct sys_signature<void (T::*)(As...) const> {
            using type = void (As...);
            static constexpr bool constness = true;
            static constexpr sys_args_deduction_helper<type, constness> helper{};
        };

        template <typename T> struct remove_sparse_array {
            using type = T;
        };

        template <typename T> struct remove_sparse_array<hex::containers::sparse_array<T>> {
            using type = T;
        };

        template <typename T>
        using remove_sparse_array_t = typename remove_sparse_array<T>::type;
    }
    /**
    ** \endcond Internals
    */

    /**
    ** \defgroup SystemRegistryTag Systems registry tag types
    **
    ** check_t and auto_register_t are empty tags classes used to specify additional action to take upon system registrations.
    */
    /** @{ */
    /**
    ** \brief Enable component registration check.
    **
    ** The constant check allow for easy use of this tag type.
    */
    struct check_t {};
    static constexpr check_t check{};

    /**
    ** \brief Enable component auto-registration.
    **
    ** The constant auto_register allow for easy use of this tag type.
    */
    struct auto_register_t {};
    static constexpr auto_register_t auto_register{};
    /** @} */

    /**
    ** \brief Manages systems
    **
    ** The main purpose of this class is to simplify system handling, as well as removing most boilerplate code.
    ** Systems are first registered to it, then called in order upon a call to system_registry::run.
    **
    ** \section system_registration System Registration
    ** System are registered through the use of one of the system_registry::register_system function overload.
    ** Any [Callable](https://en.cppreference.com/w/cpp/named_req/Callable) or free function can be used as a system.
    ** System's arguments are deduced automatically in most cases, so you don't need to specify them.
    **
    ** \subsection registering_lamda Registering lambda
    ** There is a special case meant for lambda registration, to enable the use of `auto` as a lambda parameter type.
    ** If the type of the system follow the [Callable](https://en.cppreference.com/w/cpp/named_req/Callable) named requirement, it is possible to specify parameter types explicitly.
    ** If the type isn't one of system_registry, entity_manager or components_registry, it will be considered as a component type.
    ** If so, the deduced type will be components_registry::container_t<Component>.
    **
    ** \subsection checking_argument Checking arguments type
    ** If check_t is passed to a register_system function, types detected as component will be checked against the components_registry, to allow for an early exception.
    **
    ** \subsection auto_registration Auto-register components
    ** If auto_register_t is passed to one of the register_system functions, types detected as components will be registered in the components_registry.
    **
    ** \section system_call System Call
    ** Upon a call to system_registry::run, every systems are called in the order in which they were registered to the registry.
    ** Components containers are retrieved from the components_registry associated with this class.
    **
    ** \see SystemRegistryTag
    */
    class system_registry {
        public:
            template <typename... Args>
            using system_fptr_t = void (*)(Args...);

        private:
            using caller_t = std::function<void (system_registry &)>;

        public:
            /**
            ** \brief Contruct a system_registry.
            **
            ** This function initialize a new system_registry, storing both the entity_manager and the component_registry so they can be passed to the systems if needed.
            **
            ** \throw std::invalid_argument can be thrown if em or cr are null.
            */
            system_registry(std::shared_ptr<entity_manager> const &em,
                            std::shared_ptr<components_registry> const &cr)
                : _entities{em}, _components{cr} {
                    if (!_entities) throw std::invalid_argument("[system_registry]: Invalid entity_manager.");
                    if (!_components) throw std::invalid_argument("[system_registry]: Invalid components_registry..");
                }
            system_registry(system_registry const &) = delete;
            /**
            ** \brief Move-construct a system_registry.
            */
            system_registry(system_registry &&) noexcept = default;

            system_registry &operator=(system_registry const &) = delete;
            /**
            ** \brief Move assign a system_registry.
            */
            system_registry &operator=(system_registry &&) noexcept = default;

            /**
            ** \name System calling
            */
            /** @{ */
            /**
            ** \brief Call registered system in order.
            */
            void run() {
                for (auto &f: _systems)
                    f(*this);
            }
            /** @{ */

            /**
            ** \name System registration
            */
            /** @{ */
            /**
            ** \brief System registration with explicit parameter types.
            */
            template <typename... Args, class Callable, typename = std::enable_if_t<sizeof...(Args) != 0>>
            void register_system(Callable &&c) {
                return _do_register<false, Args...>(std::forward<Callable>(c));
            }

            /**
            ** \brief System registration with deduced parameter types.
            */
            template <class Callable>
            void register_system(Callable &&c) {
                using callable_type = std::remove_cv_t<std::remove_reference_t<Callable>>;
                return _do_register(std::forward<Callable>(c), __impl::sys_signature<decltype(&callable_type::operator())>::helper);
            }

            /**
            ** \brief System registration for free function.
            */
            template <typename ...Args>
            void register_system(system_fptr_t<Args...> const f) {
                return _do_register<true, Args...>(f);
            }

            /**
            ** \brief System registration with explicit parameter types, and component registration check.
            */
            template <typename... Args, class Callable, typename = std::enable_if_t<sizeof...(Args) != 0>>
            void register_system(check_t, Callable &&c) {
                _check_arguments<Args...>();

                return _do_register<false, Args...>(std::forward<Callable>(c));
            }

            /**
            ** \brief System registration with deduced parameter types, and component regisration check.
            */
            template <class Callable>
            void register_system(check_t, Callable &&c) {
                using callable_type = std::remove_cv_t<std::remove_reference_t<Callable>>;
                auto helper = __impl::sys_signature<decltype(&callable_type::operator())>::helper;

                _check_arguments(helper);
                return _do_register(std::forward<Callable>(c), helper);
            }

            /**
            ** \brief System registration for free function, with component registration check.
            */
            template <typename ...Args>
            void register_system(check_t, system_fptr_t<Args...> const f) {
                _check_arguments<Args...>();

                return _do_register<true, Args...>(f);
            }

            /**
            ** \brief System registration with explicit parameter types, and component auto-registration.
            */
            template <typename... Args, class Callable, typename = std::enable_if_t<sizeof...(Args) != 0>>
            void register_system(auto_register_t, Callable &&c) {
                _register_arguments<Args...>();

                return _do_register<false, Args...>(std::forward<Callable>(c));
            }

            /**
            ** \brief System registration with deduced parameter types and component auto-registration.
            */
            template <class Callable>
            void register_system(auto_register_t, Callable &&c) {
                using callable_type = std::remove_cv_t<std::remove_reference_t<Callable>>;
                auto helper = __impl::sys_signature<decltype(&callable_type::operator())>::helper;

                _register_arguments(helper);
                return _do_register(std::forward<Callable>(c), helper);
            }

            /**
            ** \brief System registration for free function, with component auto-registration.
            */
            template <typename ...Args>
            void register_system(auto_register_t, system_fptr_t<Args...> const f) {
                _register_arguments<Args...>();

                return _do_register<true, Args...>(f);
            }
            /** @} */
        private:
            template <typename Arg>
            auto &_get_arg() {
                using _Arg = __impl::argument_helper_t<std::remove_cv_t<std::remove_reference_t<Arg>>>;
                return __impl::Getter<_Arg>::get(*this, *_entities, *_components);
            }

            template <typename Arg>
            void _check_arg() {
                using _Arg = __impl::remove_sparse_array_t<__impl::argument_helper_t<std::remove_cv_t<std::remove_reference_t<Arg>>>>;
                using namespace std::string_literals;

                if constexpr (std::negation_v<std::disjunction<
                    std::is_same<_Arg, system_registry>,
                    std::is_same<_Arg, entity_manager>,
                    std::is_same<_Arg, components_registry>
                >>) {
                    if (!_components->has<_Arg>())
                        throw hex::exceptions::no_such_component(typeid(Arg).name() + " has not been registered."s);
                }
            }

            template <typename Arg>
            void _reg_arg() {
                using _Arg = __impl::remove_sparse_array_t<__impl::argument_helper_t<std::remove_cv_t<std::remove_reference_t<Arg>>>>;

                _components->try_register_type<_Arg>();
            }

            template <bool constness, typename ...Args, typename Callable>
            void _do_register(Callable &&c) {
                if constexpr (constness)
                    _systems.emplace_back([sys=std::forward<Callable>(c)](system_registry &sr ) {
                        return sys(sr._get_arg<Args>()...);
                    });
                else {
                    _systems.emplace_back([sys=std::forward<Callable>(c)] (system_registry &sr) mutable {
                        return sys(sr._get_arg<Args>()...);
                    });
                }
            }

            template <typename Callable, typename... Args, bool constness>
            void _do_register(Callable &&c, __impl::sys_args_deduction_helper<void (Args...), constness>) {
                return _do_register<constness, Args...>(std::forward<Callable>(c));
            }

            template <typename... Args>
            void _check_arguments() { (_check_arg<Args>(), ...); }

            template <typename... Args, bool _>
            void _check_arguments(__impl::sys_args_deduction_helper<void (Args...), _>) { _check_arguments<Args...>(); }

            template <typename... Args>
            void _register_arguments();

            template <typename... Args, bool _>
            void _register_arguments(__impl::sys_args_deduction_helper<void (Args...), _>) { _register_arguments<Args...>(); }

        private:
            std::shared_ptr<components_registry> _components;
            std::shared_ptr<entity_manager> _entities;
            std::vector<caller_t> _systems;
    };

    template <> inline void system_registry::_reg_arg<system_registry>() {}
    template <> inline void system_registry::_reg_arg<entity_manager>() {}
    template <> inline void system_registry::_reg_arg<components_registry>() {}

    template <typename... Args>
    inline void system_registry::_register_arguments() {
        (_reg_arg<std::remove_cv_t<std::remove_reference_t<Args>>>(), ...);
    }
}

#endif /* end of include guard: SYSTEM_REGISTRY_HPP_ */
