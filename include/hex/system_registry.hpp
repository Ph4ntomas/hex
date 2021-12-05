/**
** \file system_registry.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-22 10:59
** \date Last update: 2021-12-05 17:54
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

    struct check_t {};
    static constexpr check_t check{};

    struct auto_register_t {};
    static constexpr auto_register_t auto_register{};

    class system_registry {
        public:
            template <typename... Args>
            using system_fptr_t = void (*)(Args...);

        private:
            using caller_t = std::function<void (system_registry &)>;

        public:
            system_registry(std::shared_ptr<entity_manager> const &em,
                            std::shared_ptr<components_registry> const &cr)
                : _entities{em}, _components{cr} {
                    if (!_entities) throw std::invalid_argument("[system_registry]: Invalid entity_manager.");
                    if (!_components) throw std::invalid_argument("[system_registry]: Invalid components_registry..");
                }
            system_registry(system_registry const &) = delete;
            system_registry(system_registry &&) noexcept = default;

            system_registry &operator=(system_registry const &) = delete;
            system_registry &operator=(system_registry &&) noexcept = default;

            void run() {
                for (auto &f: _systems)
                    f(*this);
            }

            template <typename... Args, class Callable, typename = std::enable_if_t<sizeof...(Args) != 0>>
            void register_system(Callable &&c) {
                return _do_register<false, Args...>(std::forward<Callable>(c));
            }

            template <class Callable>
            void register_system(Callable &&c) {
                using callable_type = std::remove_cv_t<std::remove_reference_t<Callable>>;
                return _do_register(std::forward<Callable>(c), __impl::sys_signature<decltype(&callable_type::operator())>::helper);
            }

            template <typename ...Args>
            void register_system(system_fptr_t<Args...> const f) {
                return _do_register<true, Args...>(f);
            }

            template <typename... Args, class Callable, typename = std::enable_if_t<sizeof...(Args) != 0>>
            void register_system(check_t, Callable &&c) {
                _check_arguments<Args...>();

                return _do_register<false, Args...>(std::forward<Callable>(c));
            }

            template <class Callable>
            void register_system(check_t, Callable &&c) {
                using callable_type = std::remove_cv_t<std::remove_reference_t<Callable>>;
                auto helper = __impl::sys_signature<decltype(&callable_type::operator())>::helper;

                _check_arguments(helper);
                return _do_register(std::forward<Callable>(c), helper);
            }

            template <typename ...Args>
            void register_system(check_t, system_fptr_t<Args...> const f) {
                _check_arguments<Args...>();

                return _do_register<true, Args...>(f);
            }

            template <typename... Args, class Callable, typename = std::enable_if_t<sizeof...(Args) != 0>>
            void register_system(auto_register_t, Callable &&c) {
                _register_arguments<Args...>();

                return _do_register<false, Args...>(std::forward<Callable>(c));
            }

            template <class Callable>
            void register_system(auto_register_t, Callable &&c) {
                using callable_type = std::remove_cv_t<std::remove_reference_t<Callable>>;
                auto helper = __impl::sys_signature<decltype(&callable_type::operator())>::helper;

                _register_arguments(helper);
                return _do_register(std::forward<Callable>(c), helper);
            }

            template <typename ...Args>
            void register_system(auto_register_t, system_fptr_t<Args...> const f) {
                _register_arguments<Args...>();

                return _do_register<true, Args...>(f);
            }
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
