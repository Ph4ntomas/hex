/**
** \file system_registry.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2022-01-01 18:34
** \date Last update: 2023-01-31 11:08
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
    template <class ...> class system_registry;

    /**
    ** \cond Internals
    */
    namespace __impl {
        template <class, typename> struct Getter {};

        template <class ...Args> struct Getter<system_registry<Args...>, system_registry<Args...>> {
            inline static system_registry<Args...> &get(
                    system_registry<Args...> &sr,
                    entity_manager &em,
                    components_registry &cr,
                    std::tuple<Args &...> const & args) {
                return sr;
            }
        };

        template <class ...Args> struct Getter<system_registry<Args...>, entity_manager> {
            inline static entity_manager &get(
                    system_registry<Args...> &sr,
                    entity_manager &em,
                    components_registry &cr,
                    std::tuple<Args &...> const &args) {
                return em;
            }
        };

        template <class ...Args> struct Getter<system_registry<Args...>, components_registry> {
            inline static components_registry &get(
                    system_registry<Args...> &sr,
                    entity_manager &em,
                    components_registry &cr,
                    std::tuple<Args &...> const &args) {
                return cr;
            }
        };

        template <class ...Args, class T> struct Getter<system_registry<Args...>, containers::sparse_array<T>> {
            inline static containers::sparse_array<T> &get(
                    system_registry<Args...> &sr,
                    entity_manager &em,
                    components_registry &cr,
                    std::tuple<Args &...> const &args) {
                return cr.get<T>();
            }
        };

        template <class ...Args, class T> struct Getter<system_registry<Args...>, T> {
            inline static T &get(
                    system_registry<Args...> &sr,
                    entity_manager &em,
                    components_registry &cr,
                    std::tuple<Args &...> const &args) {
                return std::get<T &>(args);
            }
        };

        template <typename T, bool> struct base_argument_helper {};

        template <typename T> struct base_argument_helper<T, true> {
            using type = T;
        };

        template <typename T> struct base_argument_helper<T, false> {
            using type = hex::containers::sparse_array<T>;
        };

        template <typename, typename> struct argument_helper {
        };

        template <typename SR, typename T>
        using argument_helper_t = typename argument_helper<SR, T>::type;

        template <class... Args, typename T>
        struct argument_helper<system_registry<Args...>, hex::containers::sparse_array<T>> {
            using type = hex::containers::sparse_array<T>;
        };

        template <class... Args, typename T>
            struct argument_helper<system_registry<Args...>, T> {
                using type = std::conditional_t<
                    std::disjunction_v<std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::remove_cv_t<std::remove_reference_t<Args>>>...>,
                    T,
                    hex::containers::sparse_array<T>
                >;
            };

        template <class... Args, typename T, typename Allocator>
        struct argument_helper<system_registry<Args...>, hex::containers::sparse_array<T, Allocator>> {
            using type = hex::containers::sparse_array<T, Allocator>;
        };

        template <class... Args> struct argument_helper<system_registry<Args...>, hex::entity_manager> {
            using type = hex::entity_manager;
        };

        template <class... Args> struct argument_helper<system_registry<Args...>, hex::components_registry> {
            using type = hex::components_registry;
        };

        template <class... Args>
        struct argument_helper<hex::system_registry<Args...>, hex::system_registry<Args...>> {
            using type = hex::system_registry<Args...>;
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
    ** \tparam Args Additional parameters for the systems that the function run() will be called with.
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
    template <class... Args>
    class system_registry {
        public:
            template <typename... As>
            using system_fptr_t = void (*)(As...);

        private:
            using caller_t = std::function<void (system_registry &, std::tuple<Args &...> const &)>;
            using Self = system_registry;

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
            **
            ** \tparam Args Additional parameters that can be used by the systems
            **
            ** \param [in] as Parameter pack containing additional parameters to supply to the systems.
            */
            void run(Args &... as) {
                auto run_args = std::tie(as...);
                for (auto &f: _systems)
                    f(*this, run_args);
            }
            /** @} */

            /**
            ** \name System registration
            */
            /** @{ */
            /**
            ** \brief System registration with explicit parameter types.
            **
            ** \param [in] c System to register.
            **
            ** \tparam As Callable parameter list.
            ** \tparam Callable Type of the system to register.
            */
            template <typename... As, class Callable, typename = std::enable_if_t<sizeof...(As) != 0>>
            void register_system(Callable &&c) {
                return _do_register<false, As...>(std::forward<Callable>(c));
            }

            /**
            ** \brief System registration with deduced parameter types.
            **
            ** \param [in] c System to register.
            **
            ** \tparam Callable Type of the system.
            */
            template <class Callable>
            void register_system(Callable &&c) {
                using callable_type = std::remove_cv_t<std::remove_reference_t<Callable>>;
                return _do_register(std::forward<Callable>(c), __impl::sys_signature<decltype(&callable_type::operator())>::helper);
            }

            /**
            ** \brief System registration for free function.
            **
            ** \param [in] f System's function pointer.
            **
            ** \tparam As Systems parameters types.
            */
            template <typename ...As>
            void register_system(system_fptr_t<As...> const f) {
                return _do_register<true, As...>(std::move(f));
            }

            /**
            ** \brief System registration with explicit parameter types, and component registration check.
            **
            ** \param [in] t Disambiguation tag.
            ** \param [in] c System to register.
            **
            ** \tparam As Callable parameter list.
            ** \tparam Callable Type of the system to register.
            */
            template <typename... As, class Callable, typename = std::enable_if_t<sizeof...(As) != 0>>
            void register_system([[maybe_unused]]check_t t, Callable &&c) {
                _check_arguments<As...>();

                return _do_register<false, As...>(std::forward<Callable>(c));
            }

            /**
            ** \brief System registration with deduced parameter types, and component regisration check.
            **
            ** \param [in] t Disambiguation tag.
            ** \param [in] c System to register.
            **
            ** \tparam Callable Type of the system.
            */
            template <class Callable>
            void register_system([[maybe_unused]]check_t t, Callable &&c) {
                using callable_type = std::remove_cv_t<std::remove_reference_t<Callable>>;
                auto helper = __impl::sys_signature<decltype(&callable_type::operator())>::helper;

                _check_arguments(helper);
                return _do_register(std::forward<Callable>(c), helper);
            }

            /**
            ** \brief System registration for free function, with component registration check.
            **
            ** \param [in] t Disambiguation tag.
            ** \param [in] f System's function pointer.
            **
            ** \tparam As Systems parameters types.
            */
            template <typename ...As>
            void register_system([[maybe_unused]]check_t t, system_fptr_t<As...> const f) {
                _check_arguments<As...>();

                return _do_register<true, As...>(std::move(f));
            }

            /**
            ** \brief System registration with explicit parameter types, and component auto-registration.
            **
            ** \param [in] t Disambiguation tag.
            ** \param [in] c System to register.
            **
            ** \tparam As Callable parameter list.
            ** \tparam Callable Type of the system to register.
            */
            template <typename... As, class Callable, typename = std::enable_if_t<sizeof...(As) != 0>>
            void register_system([[maybe_unused]]auto_register_t t, Callable &&c) {
                _register_arguments<As...>();

                return _do_register<false, As...>(std::forward<Callable>(c));
            }

            /**
            ** \brief System registration with deduced parameter types and component auto-registration.
            **
            ** \param [in] t Disambiguation tag.
            ** \param [in] c System to register.
            **
            ** \tparam Callable Type of the system.
            */
            template <class Callable>
            void register_system([[maybe_unused]]auto_register_t t, Callable &&c) {
                using callable_type = std::remove_cv_t<std::remove_reference_t<Callable>>;
                auto helper = __impl::sys_signature<decltype(&callable_type::operator())>::helper;

                _register_arguments(helper);
                return _do_register(std::forward<Callable>(c), helper);
            }

            /**
            ** \brief System registration for free function, with component auto-registration.
            **
            ** \param [in] t Disambiguation tag.
            ** \param [in] f System's function pointer.
            **
            ** \tparam As Systems parameters types.
            */
            template <typename ...As>
            void register_system([[maybe_unused]]auto_register_t t, system_fptr_t<As...> const f) {
                _register_arguments<As...>();

                return _do_register<true, As...>(std::move(f));
            }
            /** @} */
        private:
            template <typename Arg>
            auto &_get_arg(std::tuple<Args &...> const & run_args) {
                using _Arg = __impl::argument_helper_t<Self, std::remove_cv_t<std::remove_reference_t<Arg>>>;
                return __impl::Getter<Self, _Arg>::get(*this, *_entities, *_components, run_args);
            }

            template <typename Arg>
            void _check_arg() {
                using _Arg = __impl::remove_sparse_array_t<__impl::argument_helper_t<Self, std::remove_cv_t<std::remove_reference_t<Arg>>>>;
                using namespace std::string_literals;

                if constexpr (std::negation_v<std::disjunction<
                    std::is_same<_Arg, system_registry>,
                    std::is_same<_Arg, entity_manager>,
                    std::is_same<_Arg, components_registry>,
                    std::is_same<_Arg, std::remove_cv_t<std::remove_reference_t<Args>>>...
                >>) {
                    if (!_components->has<_Arg>())
                        throw hex::exceptions::no_such_component(typeid(Arg).name() + " has not been registered."s);
                }
            }

            template <typename Arg>
            void _reg_arg() {
                using _Arg = __impl::remove_sparse_array_t<__impl::argument_helper_t<Self, std::remove_cv_t<std::remove_reference_t<Arg>>>>;

                if constexpr (std::negation_v<std::disjunction<
                    std::is_same<_Arg, system_registry>,
                    std::is_same<_Arg, entity_manager>,
                    std::is_same<_Arg, components_registry>,
                    std::is_same<_Arg, std::remove_cv_t<std::remove_reference_t<Args>>>...
                >>) {
                    _components->try_register_type<_Arg>();
                }
            }

            template <bool constness, typename ...As, typename Callable>
            void _do_register(Callable &&c) {
                struct { Callable sys; } call = { std::forward<Callable>(c) };
                if constexpr (constness)
                    _systems.emplace_back([call = std::move(call)](system_registry &sr, std::tuple<Args &...> const & run_args) {
                        return call.sys(sr._get_arg<As>(run_args)...);
                    });
                else {
                    _systems.emplace_back([call= std::move(call)] (system_registry &sr, std::tuple<Args &...> const & run_args) mutable {
                        return call.sys(sr._get_arg<As>(run_args)...);
                    });
                }
            }

            template <typename Callable, typename... As, bool constness>
            void _do_register(Callable &&c, __impl::sys_args_deduction_helper<void (As...), constness>) {
                return _do_register<constness, As...>(std::forward<Callable>(c));
            }

            template <typename... As>
            void _check_arguments() { (_check_arg<As>(), ...); }

            template <typename... As, bool _>
            void _check_arguments(__impl::sys_args_deduction_helper<void (As...), _>) { _check_arguments<As...>(); }

            template <typename... As>
                void _register_arguments() {
                    (_reg_arg<std::remove_cv_t<std::remove_reference_t<As>>>(), ...);
                }

            template <typename... As, bool _>
            void _register_arguments(__impl::sys_args_deduction_helper<void (As...), _>) { _register_arguments<As...>(); }

        private:
            std::shared_ptr<components_registry> _components;
            std::shared_ptr<entity_manager> _entities;
            std::vector<caller_t> _systems;
    };
}

#endif /* end of include guard: SYSTEM_REGISTRY_HPP_ */
