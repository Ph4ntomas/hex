/**
** \file context.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-12-05 16:35
** \date Last update: 2021-12-05 17:55
*/

#ifndef CONTEXT_HPP_
#define CONTEXT_HPP_

#include <memory> // std::make_shared, std::shared_ptr

#include "hex/components_registry.hpp"
#include "hex/entity_manager.hpp"
#include "hex/system_registry.hpp"

namespace hex {
    class context {
        public:
            context() :
                _components{std::make_shared<components_registry>()},
                _entities{std::make_shared<entity_manager>(_components)},
                _systems{std::make_shared<system_registry>(_entities, _components)}
            {}
            context(context const &) = delete;
            context(context &&oth) noexcept = default;

            context &operator=(context const &) = delete;
            context &operator=(context &&rhs) noexcept = default;

            [[nodiscard]] components_registry &components() noexcept { return *_components; }
            [[nodiscard]] entity_manager &entities() noexcept { return *_entities; }
            [[nodiscard]] system_registry &systems() noexcept { return *_systems; }

            [[nodiscard]] components_registry const &components() const noexcept { return *_components; }
            [[nodiscard]] entity_manager const &entities() const noexcept { return *_entities; }
            [[nodiscard]] system_registry const &systems() const noexcept { return *_systems; }

        private:
            std::shared_ptr<components_registry> _components;
            std::shared_ptr<entity_manager> _entities;
            std::shared_ptr<system_registry> _systems;
    };
}

#endif /* end of include guard: CONTEXT_HPP_ */
