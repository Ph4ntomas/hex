/**
** \file entity_manager.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-13 12:29
** \date Last update: 2021-11-21 17:24
*/

#ifndef ENTITY_MANAGER_HPP_
#define ENTITY_MANAGER_HPP_

#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

#include "hex/containers/sparse_array.hpp"

#include "hex/exceptions/already_dead.hpp"
#include "hex/exceptions/no_such_entity.hpp"
#include "hex/exceptions/unimplemented.hpp"

#include "hex/components_registry.hpp"

#ifndef HEX_TEST
    #define h_class class
#else
    #define h_class struct
#endif

namespace hex {
    class entity_manager {
        public:
            h_class entity_t {
                friend entity_manager;
                std::size_t id : 32;
                std::size_t version : 32;

                inline static constexpr std::size_t max_id = std::numeric_limits<uint32_t>::max();

                public:
                    operator std::size_t() const { return id; }

                [[nodiscard]] friend bool operator==(entity_t const &l, entity_t const &r) {
                    return l.id == r.id && l.version == r.version;
                }
                [[nodiscard]] friend bool operator!=(entity_t const &l, entity_t const &r) { return !(l == r); }

                [[nodiscard]] friend bool operator<(entity_t const &l, entity_t const &r) {
                    return l.id != r.id && l.id < r.id;
                }

                [[nodiscard]] friend bool operator>(entity_t const &l, entity_t const &r) {
                    return l.id != r.id && l.id > r.id;
                }

                [[nodiscard]] friend bool operator<=(entity_t const &l, entity_t const &r) { return l == r || l < r; }
                [[nodiscard]] friend bool operator>=(entity_t const &l, entity_t const &r) { return l == r || l > r; }
            };
        public:
            entity_manager(std::shared_ptr<components_registry> const &r) : _max_id(0), _registry(r) {
                if (! _registry) throw std::invalid_argument("entity_manager: component registry ptr can't be null.");
            }
            entity_manager(entity_manager const &e) = delete;
            entity_manager(entity_manager &&e) noexcept = default;

            entity_manager & operator=(entity_manager const &) = delete;
            entity_manager & operator=(entity_manager &&) noexcept = default;

            [[nodiscard]] entity_t spawn() { throw exceptions::unimplemented{"spawn"}; }

            template <class... Components>
            entity_t spawn_with(Components &&... cmps) { throw exceptions::unimplemented{"spawn_with"}; }

            void kill(entity_t const &e) { throw exceptions::unimplemented{"kill"}; }
            void kill_at(std::size_t e) { throw exceptions::unimplemented{"kill_at"}; }

            bool try_kill(entity_t const &e) { throw exceptions::unimplemented{"kill"}; }
            bool try_kill_at(std::size_t e) { throw exceptions::unimplemented{"kill_at"}; }

            template <class Component>
            Component & add_component(entity_t const &e, Component &&cmp) { throw exceptions::unimplemented{"add_component entity component"}; }
            template <class Component>
            Component & add_component(std::size_t id, Component &&cmp) { throw exceptions::unimplemented{"add_component id component"}; }

            template <class Component, class ... Args>
            Component & emplace_component(entity_t const &e, Args && ... as) { throw exceptions::unimplemented{"add_component entity args"}; }
            template <class Component, class ... Args>
            Component & emplace_component(std::size_t id, Args && ... as) { throw exceptions::unimplemented{"add_component id args"}; }

            template <class Component>
            bool has_component(entity_t const &s) { throw exceptions::unimplemented{"has_component entity"}; }
            template <class Component>
            bool has_component(std::size_t id) { throw exceptions::unimplemented{"has_component id"}; }

            template <class Component>
            Component &get_component(entity_t const &s) { throw exceptions::unimplemented{"has_component entity"}; }
            template <class Component>
            Component &get_component(std::size_t id) { throw exceptions::unimplemented{"has_component id"}; }

            template <class Component>
            std::optional<Component> &try_get_component(entity_t const &s) { throw exceptions::unimplemented{"has_component entity"}; }
            template <class Component>
            std::optional<Component> &try_get_component(std::size_t id) { throw exceptions::unimplemented{"has_component id"}; }

            template <class Component>
            void remove_component(entity_t const &s) { throw exceptions::unimplemented{"remove_component entity"}; }
            template <class Component>
            void remove_component(std::size_t id) { throw exceptions::unimplemented{"remove_component id"}; }

            [[nodiscard]] bool is_live(entity_t const &e) const { throw exceptions::unimplemented{"entity is_live"}; }
            [[nodiscard]] bool is_live(std::size_t id) const { throw exceptions::unimplemented{"id is_live"}; }

            [[nodiscard]] entity_t get_entity(std::size_t id) const { throw exceptions::unimplemented{"get_entity"}; }
            [[nodiscard]] std::optional<entity_t> try_get_entity(std::size_t id) const { throw exceptions::unimplemented{"try_get_entity"}; }

        public:
            inline static constexpr std::size_t max_entities() noexcept { return entity_t::max_id; }
        private:
            containers::sparse_array<entity_t> _live;
            std::vector<entity_t> _graveyard;
            std::size_t _max_id;
            std::shared_ptr<components_registry> _registry;
    };

    using entity_t = entity_manager::entity_t;
}

#undef h_class

#endif /* end of include guard: ENTITY_MANAGER_HPP_ */
