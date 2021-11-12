/**
** \file components_registry.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-12 11:09
** \date Last update: 2021-11-12 17:40
*/

#ifndef COMPONENTS_REGISTRY_HPP_
#define COMPONENTS_REGISTRY_HPP_

#include <any>
#include <cstddef>
#include <optional>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>

#include "hex/containers/sparse_array.hpp"
#include "hex/exceptions/already_registered.hpp"
#include "hex/exceptions/unimplemented.hpp"

namespace hex {
    class components_registry {
        public:
            template <class T>
            using container_t = containers::sparse_array<std::decay_t<T>>;

        public:
            template <typename Component>
            container_t<Component> &register_type() {
                auto && [v, ok] = try_register_type<Component>();

                if (!ok)
                    throw exceptions::already_registered(typeid(std::decay_t<Component>).name());

                return v;
            }

            template <typename Component>
            std::tuple<container_t<Component> &, bool> try_register_type() noexcept {
                auto [it, ok] = _registry.try_emplace(std::type_index{typeid(std::decay_t<Component>)}, std::make_any<container_t<Component>>());

                if (ok) {
                    _erasers.emplace_back([](components_registry &r, std::size_t index) {
                        r.remove_at<Component>(index);
                    });
                }

                return std::tie(std::any_cast<container_t<Component> &>(it->second), ok);
            }

            template <typename Component>
            [[nodiscard]]
            container_t<Component> &get() {
                return const_cast<container_t<Component> &>(std::as_const(*this).get<Component>());
            }

            template <typename Component>
            [[nodiscard]]
            container_t<Component> const &get() const {
                return std::any_cast<container_t<Component> const &>(_registry.at(typeid(std::decay_t<Component>)));
            }

            template <typename Component>
            Component & insert_at(std::size_t idx, Component &&c) {
                auto & cont = get<Component>();

                cont.insert_at(idx, std::forward<Component>(c));

                return cont.at(idx).value();
            }

            template <typename Component, class... Params>
            Component & emplace_at(std::size_t idx, Params &&... ps) {
                auto &cont = get<Component>();

                cont.emplace_at(idx, std::forward<Params>(ps)...);

                return cont.at(idx).value();
            }

            template <typename Component>
            void remove_at(std::size_t index) {
                auto &cont = get<Component>();

                if (cont.size() > index)
                    cont.erase_at(index);
            }

            void erase_at(std::size_t index) {
                for (auto &&f : _erasers) {
                    f(*this, index);
                }
            }
        private:
            std::unordered_map<std::type_index, std::any> _registry;

            std::vector<std::function<void(components_registry &, std::size_t)>> _erasers;
    };
}

#endif /* end of include guard: COMPONENTS_REGISTRY_HPP_ */
