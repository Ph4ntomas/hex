/**
** \file context.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-12-05 16:35
** \date Last update: 2022-01-02 13:54
*/

#ifndef CONTEXT_HPP_
#define CONTEXT_HPP_

#include <memory> // std::make_shared, std::shared_ptr

#include "hex/components_registry.hpp"
#include "hex/entity_manager.hpp"
#include "hex/system_registry.hpp"

namespace hex {
    /**
    ** \brief Hex context
    **
    ** Convenience class that construct and hold every part of the hex library.
    **
    ** \tparam SystemRunArgs Arbitrary list of parameters that will be accessible to systems.
    */
    template <class... SystemRunArgs>
    class context {
        public:
            /**
            ** \brief Default contructor.
            */
            context() :
                _components{std::make_shared<components_registry>()},
                _entities{std::make_shared<entity_manager>(_components)},
                _systems{std::make_shared<system_registry<SystemRunArgs...>>(_entities, _components)}
            {}
            context(context const &) = delete;
            /**
            ** \brief Move constructor
            */
            context(context &&oth) noexcept = default;

            context &operator=(context const &) = delete;
            /**
            ** \brief Move-assigment operator
            */
            context &operator=(context &&rhs) noexcept = default;

            /**
            ** \brief Access components registry
            */
            [[nodiscard]] components_registry &components() noexcept { return *_components; }
            /**
            ** \brief Access entity manager
            */
            [[nodiscard]] entity_manager &entities() noexcept { return *_entities; }
            /**
            ** \brief Access system registry
            */
            [[nodiscard]] system_registry<SystemRunArgs...> &systems() noexcept { return *_systems; }

            /**
            ** \brief Access components registry
            */
            [[nodiscard]] components_registry const &components() const noexcept { return *_components; }
            /**
            ** \brief Access entity manager
            */
            [[nodiscard]] entity_manager const &entities() const noexcept { return *_entities; }
            /**
            ** \brief Access system registry
            */
            [[nodiscard]] system_registry<SystemRunArgs...> const &systems() const noexcept { return *_systems; }

        private:
            std::shared_ptr<components_registry> _components;
            std::shared_ptr<entity_manager> _entities;
            std::shared_ptr<system_registry<SystemRunArgs...>> _systems;
    };
}

#endif /* end of include guard: CONTEXT_HPP_ */
