/**
** \file entity_manager.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-13 12:29
** \date Last update: 2023-11-19 16:00
*/

#ifndef ENTITY_MANAGER_HPP_
#define ENTITY_MANAGER_HPP_

#include <limits> // std::numeric_limits
#include <cstddef> // std::size_t
#include <cstdint>
#include <memory> // std::shared_ptr
#include <optional> // std::nullopt
#include <stdexcept> // std::invalid_argument
#include <string> // std::string_literals
#include <utility> // std::forward
#include <vector> // std::vector

#include "hex/components_registry.hpp"
#include "hex/containers/sparse_array.hpp"
#include "hex/exceptions/already_dead.hpp"
#include "hex/exceptions/no_such_entity.hpp"

#ifndef HEX_TEST
    #define h_class class
#else
    #define h_class struct
#endif

namespace hex {
    /**
    ** \brief Creates and manage Entities.
    **
    ** The goal of the entity_manager class is to create and manages entities, such that their ID's are always valid,
    ** and recycled.
    **
    ** An entity_manager cannot be copied as it manage a given set of entities. It can however be moved.
    */

    class entity_manager {
        public:
            /**
            ** \brief Hex's entities class.
            **
            ** This class contains both an id and a version. The ID is meant to be used as an index in sparse_array.
            ** The entity_t class is convertible to std::size_t for that reason. If two entities with the same ID are compared,
            ** their respective version will allow to differentiate them.
            */
            h_class entity_t {
                friend entity_manager;

                std::uint32_t id;
                std::uint32_t version;

                inline static constexpr std::size_t max_id = std::numeric_limits<uint32_t>::max();

                public:
                    /**
                    ** \brief Convert the entity to an id usable in a sparse_array.
                    */
                    operator std::size_t() const { return id; }

                    /**
                    ** \brief Equality comparison.
                    **
                    ** Two instance of and entity are equal if both their IDs and versions are equal.
                    */
                    [[nodiscard]] friend bool operator==(entity_t const &l, entity_t const &r) {
                        return l.id == r.id && l.version == r.version;
                    }
                    /**
                    ** \brief Inequality comparison.
                    **
                    ** Two entities are not equal if their id or version number differs
                    */
                    [[nodiscard]] friend bool operator!=(entity_t const &l, entity_t const &r) { return !(l == r); }

                    /**
                    ** \brief Compare two entities.
                    **
                    ** This function first compare the id of the two entities, then their version if their ID are equal.
                    */
                    [[nodiscard]] friend bool operator<(entity_t const &l, entity_t const &r) {
                        return l.id < r.id || (l.id == r.id && l.version < r.version);
                    }

                    /**
                    ** \brief Compare two entities.
                    **
                    ** This function first compare the id of the two entities, then their version if their ID are equal.
                    */
                    [[nodiscard]] friend bool operator>(entity_t const &l, entity_t const &r) {
                        return l.id > r.id || (l.id == r.id && l.version > r.version);
                     }

                    /**
                    ** \brief Compare two entities.
                    **
                    ** This function first compare the id of the two entities, then their version if their ID are equal.
                    */
                    [[nodiscard]] friend bool operator<=(entity_t const &l, entity_t const &r) { return l == r || l < r; }

                    /**
                    ** \brief Compare two entities.
                    **
                    ** This function first compare the id of the two entities, then their version if their ID are equal.
                    */
                    [[nodiscard]] friend bool operator>=(entity_t const &l, entity_t const &r) { return l == r || l > r; }
            };
        public:
            /**
            ** \brief Basic Constructor.
            **
            ** Build the entity_manager and initialize it's internal pointer to the components_registry.
            **
            ** \param [in] registry A std::shared_ptr that allow the entity to spawn entities with specific components directly.
            **
            ** \throw std::invalid_argument Thrown if the registry is null.
            */
            entity_manager(std::shared_ptr<components_registry> const &registry) : _max_id(0), _registry(registry) {
                if (! _registry) throw std::invalid_argument("entity_manager: component registry ptr can't be null.");
            }
            entity_manager(entity_manager const &e) = delete;
            /**
            ** \brief Move constructor.
            */
            entity_manager(entity_manager &&e) noexcept = default;

            entity_manager & operator=(entity_manager const &) = delete;
            /**
            ** \brief Move Assignment operator.
            */
            entity_manager & operator=(entity_manager &&) noexcept = default;

            /**
            ** \name Lifetime management
            */
            /** @{ */
            /**
            ** \brief Spawn an entity.
            **
            ** The entity spawned will either be assigned a new ID, or the id of a previously killed entity will
            ** be reused in which case the version number will be changed. No other operation will be made.
            */
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

            /**
            ** \brief Spawn an entity with a given set of components.
            **
            ** Calling this function is equivalent to calling spawn() followed by adding the components to the entity manually.
            ** Every component is forwarded to the insert_at function of the components_registry.
            **
            ** \tparam Components Types of the components to add to the entity. You don't usually need to specify them.
            ** \param [in] cmps A parameter pack containing every components to forward to components_registry::insert_at.
            */
            template <class... Components>
            entity_t spawn_with(Components &&... cmps) {
                auto e = spawn();

                (_registry->insert_at(e.id, std::forward<Components>(cmps)), ...);

                return e;
            }

            /**
            ** \brief Kill an entity.
            **
            ** This function actually remove any components associated with the given entity, then store the entity
            ** in the _graveyard field, so it can be reused later on.
            **
            ** \param [in] e The entity to destroy
            **
            ** \throw hex::exceptions::already_dead Thrown if the given entity is already dead.
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            void kill(entity_t const &e) {
                auto e2 = _throw_dead_entity(e, "kill");

                _do_kill(e);
            }

            /**
            ** \brief Kill an entity.
            **
            ** This function actually remove any components associated with the given entity id, then store the entity
            ** in the _graveyard field, so it can be reused later on.
            **
            ** \param [in] id The Id of the entity to destroy
            **
            ** \throw hex::exceptions::already_dead Thrown if the given entity is already dead.
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            void kill_at(std::size_t id) {
                auto e = _throw_dead_entity(id, "kill_at");

                _do_kill(e);
            }

            /**
            ** \brief Try to kill an entity.
            **
            ** This function first check if the entity is correct, before killing it.
            **
            ** \param [in] e The entity to destroy
            **
            ** \return True is return if the entity was killed. False is returned otherwise.
            */
            bool try_kill(entity_t const &e) {
                if (_is_bad_entity_id(e.id)) return false;

                auto oe = _live.at(e.id);

                if (oe && oe.value().version == e.version) {
                    _do_kill(oe.value());
                    return true;
                }

                return false;
            }

            /**
            ** \brief Try to kill an entity with a given id.
            **
            ** This function first check if the entity's id is correct, before killing it.
            **
            ** \param [in] id The id of the entity to destroy
            **
            ** \return True is return if the entity was killed. False is returned otherwise.
            */
            bool try_kill_at(std::size_t id) {
                if (_is_bad_entity_id(id)) return false;

                auto oe = _live.at(id);

                if (oe) {
                    _do_kill(oe.value());
                    return true;
                }

                return false;
            }

            /**
            ** \brief Check if the given entity is alive.
            **
            ** \return True is return if the entity is still alive
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            [[nodiscard]] bool is_live(entity_t const &e) const {
                _throw_bad_entity_id(e.id, "is_live");

                auto oe = _live.at(e.id);

                if (oe && oe.value().version == e.version)
                    return true;

                return false;
            }

            /**
            ** \brief Check if the given entity is alive.
            **
            ** \return True is return if the entity is still alive
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            [[nodiscard]] bool is_live(std::size_t id) const {
                _throw_bad_entity_id(id, "is_live");

                return (bool)_live.at(id);
            }
            /** @} */


            /**
            ** \name Component Management
            */
            /** @{ */
            /**
            ** \brief Add a component to a given entity.
            **
            ** This function forward its parameter to components_registry::insert_at passing it the entity's Id.
            **
            ** \tparam Component Type of the component to add. This parameter is usually optional.
            **
            ** \param [in] e Entity to which we want to add the component,
            ** \param [in] cmp Component to add.
            **
            ** \return A reference to the added component.
            **
            ** \throw hex::exceptions::already_dead Thrown if the given entity is already dead.
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            template <class Component>
            Component & add_component(entity_t const &e, Component &&cmp) {
                _throw_dead_entity(e, "add_component");

                return _registry->insert_at(e.id, std::forward<Component>(cmp));
            }

            /**
            ** \brief Add a component to a given entity.
            **
            ** This function forward its parameter to components_registry::insert_at passing it the entity's Id.
            **
            ** \tparam Component Type of the component to add. This parameter is usually optional.
            **
            ** \param [in] id Id of the entity to which we want to add the component,
            ** \param [in] cmp Component to add.
            **
            ** \return A reference to the added component.
            **
            ** \throw hex::exceptions::already_dead Thrown if the given entity is already dead.
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            template <class Component>
            Component & add_component(std::size_t id, Component &&cmp) {
                auto e = _throw_dead_entity(id, "add_component");

                return _registry->insert_at(id, std::forward<Component>(cmp));
            }

            /**
            ** \brief Add a component to a given entity.
            **
            ** This function forward its parameter to components_registry::emplace_at passing it the entity's Id.
            ** This will then call the constructor of the component.
            **
            ** \tparam Component Type of the component to add.
            ** \tparam Args Types of the arguments to forward to the component constructor
            **
            ** \param [in] e Entity to which we want to add the component,
            ** \param [in] as Parameters to call the component constructor with.
            **
            ** \return A reference to the added component.
            **
            ** \throw hex::exceptions::already_dead Thrown if the given entity is already dead.
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            template <class Component, class ... Args>
            Component & emplace_component(entity_t const &e, Args && ... as) {
                _throw_dead_entity(e, "emplace_component");

                return _registry->emplace_at<Component>(e.id, std::forward<Args>(as)...);
            }

            /**
            ** \brief Add a component to a given entity.
            **
            ** This function forward its parameter to components_registry::emplace_at passing it the entity's Id.
            ** This will then call the constructor of the component.
            **
            ** \tparam Component Type of the component to add.
            ** \tparam Args Types of the arguments to forward to the component constructor
            **
            ** \param [in] id Id of the entity to which we want to add the component,
            ** \param [in] as Parameters to call the component constructor with.
            **
            ** \return A reference to the added component.
            **
            ** \throw hex::exceptions::already_dead Thrown if the given entity is already dead.
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            template <class Component, class ... Args>
            Component & emplace_component(std::size_t id, Args && ... as) {
                _throw_dead_entity(id, "emplace_component");

                return _registry->emplace_at<Component>(id, std::forward<Args>(as)...);
            }

            /**
            ** \brief Check that the entity has a given component.
            **
            ** \tparam Type of the component we want to check
            **
            ** \param [in] e Entity to check
            **
            ** \return True if an instance of the component has been added to the entity.
            ** \throw hex::exceptions::already_dead Thrown if the given entity is already dead.
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            template <class Component>
            [[nodiscard]] bool has_component(entity_t const &e) {
                _throw_dead_entity(e, "has_component");
                return _do_has_component<Component>(e.id);
            }

            /**
            ** \brief Check that the entity has a given component.
            **
            ** \tparam Type of the component we want to check
            **
            ** \param [in] id Id of the entity to check
            **
            ** \return True if an instance of the component has been added to the entity.
            ** \throw hex::exceptions::already_dead Thrown if the given entity is already dead.
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            template <class Component>
            [[nodiscard]] bool has_component(std::size_t id) {
                _throw_dead_entity(id, "has_component");
                return _do_has_component<Component>(id);
            }

            /**
            ** \brief Retrieve a component from an entity.
            **
            ** \tparam Type of component we want to retrieve.
            **
            ** \param [in] e Entity to check
            **
            ** \return True if an instance of the component has been added to the entity.
            ** \throw hex::exceptions::already_dead Thrown if the given entity is already dead.
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            template <class Component>
            [[nodiscard]] Component &get_component(entity_t const &e) {
                _throw_dead_entity(e, "get_component");
                return _do_get_component<Component>(e.id);
            }

            /**
            ** \brief Retrieve a component from an entity.
            **
            ** \tparam Type of component we want to retrieve.
            **
            ** \param [in] id Id of the entity to check
            **
            ** \return True if an instance of the component has been added to the entity.
            **
            ** \throw hex::exceptions::already_dead Thrown if the given entity is already dead.
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            template <class Component>
            [[nodiscard]] Component &get_component(std::size_t id) {
                _throw_dead_entity(id, "get_component");
                return _do_get_component<Component>(id);
            }

            /**
            ** \brief Remove an component from an entity.
            **
            ** This function check if the entity is alive, then remove a given component if it isn't.
            ** The component is removed by calling components_registry::remove_at.
            **
            ** \tparam Component Type of component to remove.
            **
            ** \param [in] e Entity to check
            **
            ** \throw hex::exceptions::already_dead Thrown if the given entity is already dead.
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            template <class Component>
            void remove_component(entity_t const &e) {
                _throw_dead_entity(e, "remove_component");

                return _registry->remove_at<Component>(e.id);
            }

            /**
            ** \brief Remove an component from an entity.
            **
            ** This function check if the entity is alive, then remove a given component if it isn't.
            ** The component is removed by calling components_registry::remove_at.
            **
            ** \tparam Component Type of component to remove.
            **
            ** \param [in] id Id of the entity to check
            **
            ** \throw hex::exceptions::already_dead Thrown if the given entity is already dead.
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            template <class Component>
            void remove_component(std::size_t id) {
                _throw_dead_entity(id, "remove_component");

                return _registry->remove_at<Component>(id);
            }
            /** @} */

            /**
            ** \name Miscellanous
            */
            /** @{ */
            /**
            ** \brief Get the entity at the given id.
            **
            ** This function retrieve the live entity at the given id, and return a copy of it, or throws.
            **
            ** \param [in] id The index of the entity we're trying to retrieve.
            **
            ** \return The live entity with the given id.
            **
            ** \throw hex::exceptions::already_dead Thrown if the given entity is already dead.
            ** \throw hex::exceptions::no_such_entity Thrown if the entity is invalid.
            */
            [[nodiscard]] entity_t get_entity(std::size_t id) const {
                return _throw_dead_entity(id, "get_entity");
            }

            /**
            ** \brief Get the entity at the given id.
            **
            ** This function retrieve the live entity at the given id, and return a copy of it, or std::nullopt if the entity was bad or dead.
            **
            ** \param [in] id The index of the entity we're trying to retrieve.
            **
            ** \return The live entity with the given id wrapped in a std::optional, or std::nullopt.
            */
            [[nodiscard]] std::optional<entity_t> try_get_entity(std::size_t id) const {
                if (_is_bad_entity_id(id)) return std::nullopt;

                return _live.at(id);
            }
            /** @} */

        private:
            /**
            ** \internal
            ** \brief Check whether the given entity is invalid.
            **
            ** A entity is considered as invalid if it's id is greater that the biggest Id given by the entity_manager.
            **
            ** \param [in] id Id of the entity to check.
            ** \endinternal
            */
            bool _is_bad_entity_id(std::size_t id) const noexcept {
                return id >= _max_id;
            }

            /**
            ** \internal
            ** \brief Throw is the given entity is invalid.
            **
            ** A entity is considered as invalid if it's id is greater that the biggest Id given by the entity_manager.
            **
            ** \param [in] id Id of the entity to check.
            ** \param [in] from Calling function so the thrown exception can be built properly.
            **
            ** \throw hex::exception::no_such_entity Thrown if the id of the entity is invalid.
            ** \endinternal
            */
            void _throw_bad_entity_id(std::size_t id, std::string const &from) const {
                using namespace std::string_literals;

                if (_is_bad_entity_id(id)) throw hex::exceptions::no_such_entity("[entity_manager] - "s + from + ":  No such entity with id : "s + std::to_string(id));
            }

            /**
            ** \internal
            ** \brief Throw if the given entity is dead.
            **
            ** \param [in] id Id of the entity to check.
            ** \param [in] from Calling function so the thrown exception can be built properly.
            **
            ** \return If the entity is live and has the same version number, it is returned.
            ** \throw hex::exception::already_dead Thrown if the entity is not alive.
            ** \throw hex::exception::no_such_entity Thrown if the id of the entity is invalid.
            ** \endinternal
            */
            entity_t _throw_dead_entity(std::size_t id, std::string const &from) const {
                using namespace std::string_literals;

                _throw_bad_entity_id(id, from);

                auto oe = _live.at(id);
                if (!oe) throw hex::exceptions::already_dead("[entity_manager] - "s + from + ": entity with id "s + std::to_string(id) + " is already_dead."s);

                return oe.value();
            }

            /**
            ** \internal
            ** \brief Throw if the given entity is dead.
            **
            ** \param [in] e The entity to check.
            ** \param [in] from Calling function so the thrown exception can be built properly.
            **
            ** \return If the entity is live and has the same version number, it is returned.
            ** \throw hex::exception::already_dead Thrown if the entity is not alive.
            ** \throw hex::exception::no_such_entity Thrown if the id of the entity is invalid.
            ** \endinternal
            */
            entity_t _throw_dead_entity(entity_t e, std::string const &from) const {
                using namespace std::string_literals;

                auto e2 = _throw_dead_entity(e.id, from);

                if (e.version != e2.version)
                    throw hex::exceptions::already_dead("[entity_manager] - "s + from + ": entity with id "s + std::to_string(e.id) + " and version " + std::to_string(e.version) + " is already dead."s);

                return e2;
            }

            /**
            ** \internal
            ** \brief Actual implementation of all the kill related functions.
            ** \endinternal
            */
            void _do_kill(entity_t e) {
                _registry->erase_at(e.id);
                _live.erase_at(e.id);
                _graveyard.push_back(e);
            }

            /**
            ** \internal
            ** \brief Actual implementation of all the has_component functions.
            ** \endinternal
            */
            template <class Component>
            bool _do_has_component(std::size_t id) {
                auto sa = _registry->get<Component>();

                return !(sa.size() <= id || !sa.at(id));
            }

            /**
            ** \internal
            ** \brief Actual implementation of all the get_component functions.
            ** \endinternal
            */
            template <class Component>
            Component &_do_get_component(std::size_t id) {
                return _registry->get<Component>().at(id).value();
            }

        public:
            /**
            ** \brief Return the biggest index an entity can have, thus how many entities can be created.
            */
            inline static constexpr std::size_t max_entities() noexcept { return entity_t::max_id; }
        private:
            containers::sparse_array<entity_t> _live; /** \brief Internal sparse_array to keep track of live entities. */
            std::vector<entity_t> _graveyard; /** \brief Internal std::vector of dead entities. */
            std::size_t _max_id; /** \brief Track the biggest ID that was given to an entity. */
            std::shared_ptr<components_registry> _registry;
    };

    using entity_t = entity_manager::entity_t;
}

#undef h_class

#endif /* end of include guard: ENTITY_MANAGER_HPP_ */
