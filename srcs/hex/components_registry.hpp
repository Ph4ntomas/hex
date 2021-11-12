/**
** \file components_registry.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-12 11:09
** \date Last update: 2021-11-12 11:36
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
#include "hex/exceptions/unimplemented.hpp"

namespace hex {
    class components_registry {
        public:
            template <typename Component>
            void register_type() { throw exceptions::unimplemented{"register_type"}; }

            template <typename Component>
            bool try_register_type() noexcept { throw exceptions::unimplemented{"try_register_type"}; }

            template <typename Component>
            [[nodiscard]]
            containers::sparse_array<Component> &get() { throw exceptions::unimplemented{"get"}; }

            template <typename Component>
            [[nodiscard]]
            containers::sparse_array<Component> const &get() const { throw exceptions::unimplemented{"get const"}; }

            template <typename Component>
            Component & insert_at(std::size_t, Component const &) { throw exceptions::unimplemented{"insert_at copy"}; }
            template <typename Component>
            Component & insert_at(std::size_t, Component &&) { throw exceptions::unimplemented{"insert_at move"}; }
            template <typename Component, class... Params>
            Component & emplace_at(std::size_t, Params &&...) { throw exceptions::unimplemented{"emplace_at"}; }

            template <typename Component>
            bool remove_at(std::size_t index) { throw exceptions::unimplemented{"remove_at"}; }

            void erase_at(std::size_t index) { throw exceptions::unimplemented{"erase_at"}; }
        private:
            std::unordered_map<std::type_info, std::any> _registry;

            std::vector<std::function<void(components_registry &, std::size_t)>> _erasers;
    };
}

#endif /* end of include guard: COMPONENTS_REGISTRY_HPP_ */