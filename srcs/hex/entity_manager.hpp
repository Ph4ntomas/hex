/**
** \file entity_manager.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-13 12:29
** \date Last update: 2021-11-21 20:13
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

            [[nodiscard]] entity_t spawn() {
                entity_t e;

                if (!_graveyard.empty()) {
                    e = _graveyard.back();
                    _graveyard.pop_back();

                    ++e.version;
                } else {
                    e.id = _max_id++;
                    e.version = 0;
                }

                _live.insert_at(e.id, e);
                return e;
            }

            template <class... Components>
            entity_t spawn_with(Components &&... cmps) {
                auto e = spawn();

                (_registry->insert_at(e.id, std::forward<Components>(cmps)), ...);

                return e;
            }

            void kill(entity_t const &e) {
                using namespace std::string_literals;

                auto e2 = _throw_dead_entity(e, "kill");

                _do_kill(e);
            }

            void kill_at(std::size_t id) {
                using namespace std::string_literals;

                auto e = _throw_dead_entity(id, "kill_at");

                _do_kill(e);
            }

            bool try_kill(entity_t const &e) {
                if (_is_bad_entity_id(e.id)) return false;

                auto oe = _live.at(e.id);

                if (oe && oe.value().version == e.version) {
                    _do_kill(oe.value());
                    return true;
                }

                return false;
            }

            bool try_kill_at(std::size_t id) {
                if (_is_bad_entity_id(id)) return false;

                auto oe = _live.at(id);

                if (oe) {
                    _do_kill(oe.value());
                    return true;
                }

                return false;
            }

            template <class Component>
            Component & add_component(entity_t const &e, Component &&cmp) {
                _throw_dead_entity(e, "add_component");

                return _registry->insert_at(e.id, std::forward<Component>(cmp));
            }

            template <class Component>
            Component & add_component(std::size_t id, Component &&cmp) {
                auto e = _throw_dead_entity(id, "add_component");

                return _registry->insert_at(id, std::forward<Component>(cmp));
            }

            template <class Component, class ... Args>
            Component & emplace_component(entity_t const &e, Args && ... as) {
                _throw_dead_entity(e, "emplace_component");

                return _registry->emplace_at<Component>(e.id, std::forward<Args>(as)...);
            }

            template <class Component, class ... Args>
            Component & emplace_component(std::size_t id, Args && ... as) {
                _throw_dead_entity(id, "emplace_component");

                return _registry->emplace_at<Component>(id, std::forward<Args>(as)...);
            }

            template <class Component>
            [[nodiscard]] bool has_component(entity_t const &e) {
                _throw_dead_entity(e, "has_component");
                return _do_has_component<Component>(e.id);
            }

            template <class Component>
            [[nodiscard]] bool has_component(std::size_t id) {
                _throw_dead_entity(id, "has_component");
                return _do_has_component<Component>(id);
            }

            template <class Component>
            [[nodiscard]] Component &get_component(entity_t const &e) {
                _throw_dead_entity(e, "get_component");
                return _do_get_component<Component>(e.id);
            }

            template <class Component>
            [[nodiscard]] Component &get_component(std::size_t id) {
                _throw_dead_entity(id, "get_component");
                return _do_get_component<Component>(id);
            }

            template <class Component>
            void remove_component(entity_t const &e) {
                _throw_dead_entity(e, "remove_component");

                return _registry->remove_at<Component>(e.id);
            }

            template <class Component>
            void remove_component(std::size_t id) {
                _throw_dead_entity(id, "remove_component");

                return _registry->remove_at<Component>(id);
            }

            [[nodiscard]] bool is_live(entity_t const &e) const {
                _throw_bad_entity_id(e.id, "is_live");

                auto oe = _live.at(e.id);

                if (oe && oe.value().version == e.version)
                    return true;

                return false;
            }

            [[nodiscard]] bool is_live(std::size_t id) const {
                _throw_bad_entity_id(id, "is_live");

                return (bool)_live.at(id);
            }

            [[nodiscard]] entity_t get_entity(std::size_t id) const {
                return _throw_dead_entity(id, "get_entity");
            }

            [[nodiscard]] std::optional<entity_t> try_get_entity(std::size_t id) const {
                if (_is_bad_entity_id(id)) return std::nullopt;

                return _live.at(id);
            }

        private:
            bool _is_bad_entity_id(std::size_t id) const noexcept {
                return id >= _max_id;
            }

            void _throw_bad_entity_id(std::size_t id, std::string const &from) const {
                using namespace std::string_literals;

                if (_is_bad_entity_id(id)) throw hex::exceptions::no_such_entity("[entity_manager] - "s + from + ":  No such entity with id : "s + std::to_string(id));
            }

            entity_t _throw_dead_entity(std::size_t id, std::string const &from) const {
                using namespace std::string_literals;

                _throw_bad_entity_id(id, from);

                auto oe = _live.at(id);
                if (!oe) throw hex::exceptions::already_dead("[entity_manager] - "s + from + ": entity with id "s + std::to_string(id) + " is already_dead."s);

                return oe.value();
            }

            entity_t _throw_dead_entity(entity_t e, std::string const &from) const {
                using namespace std::string_literals;

                auto e2 = _throw_dead_entity(e.id, from);

                if (e.version != e2.version)
                    throw hex::exceptions::already_dead("[entity_manager] - "s + from + ": entity with id "s + std::to_string(e.id) + " and version " + std::to_string(e.version) + " is already dead."s);

                return e2;
            }

            void _do_kill(entity_t e) {
                _registry->erase_at(e.id);
                _live.erase_at(e.id);
                _graveyard.push_back(e);
            }

            template <class Component>
            bool _do_has_component(std::size_t id) {
                auto sa = _registry->get<Component>();

                return !(sa.size() <= id || !sa.at(id));
            }

            template <class Component>
            Component &_do_get_component(std::size_t id) {
                return _registry->get<Component>().at(id).value();
            }

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
