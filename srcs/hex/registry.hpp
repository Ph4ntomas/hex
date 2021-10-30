/**
** \file registry.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-10-29 10:43
** \date Last update: 2021-10-29 12:27
*/

#ifndef REGISTRY_HPP_
#define REGISTRY_HPP_

#include <unordered_map>

#define HEX_INTERNALS
#include <hex/internals/fwd_def.hpp>
#undef HEX_INTERNALS

namespace hex {
    class registry {
        public:
            entity_manager &entities() noexcept;

            template <class Component>
            registry &register_component();

            template <class Component>
            registry &add_component(entity_t const &, Component &&component);

            template <class Component>
            registry &remove_component(entity_t const &);

            registry &remove_all_components(entity_t const &);
        private:
            entity_manager _manager;
    };
}

#endif /* end of include guard: REGISTRY_HPP_ */
