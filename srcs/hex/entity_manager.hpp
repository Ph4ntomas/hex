/**
** \file entity_manager.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-10-29 10:18
** \date Last update: 2021-10-29 12:27
*/

#ifndef ENTITY_MANAGER_HPP_
#define ENTITY_MANAGER_HPP_

#include <cstddef>

#include <functional>
#include <vector>

#include <hex/registry.hpp>

namespace hex {
    class entity_manager;

    class entity_t {
        friend entity_manager;

        entity_t(std::size_t id) : _id(id) {}

        public:
            entity_t(entity_t const &) noexcept = default;
            entity_t(entity_t &&) noexcept = default;

            entity_t &operator=(entity_t const &) noexcept = default;
            entity_t &operator=(entity_t &&) noexcept = default;

            operator std::size_t() const noexcept { return _id; }
        private:
            std::size_t _id;
    };

    class entity_manager {
        public:
            explicit entity_manager(registry &);
            entity_manager(entity_manager &&) noexcept = default;

            entity_manager &operator=(entity_manager &&) noexcept = default;

            [[nodiscard]] entity_t spawn();

            template <class... Component>
            entity_t spawn_with(registry &r, Component &&... component);

            entity_manager &remove(entity_t const &e);

        private:
            std::reference_wrapper<registry> _owner;
            std::size_t _max_entity;
            std::vector<entity_t> _graveyard;
    };

#ifndef HEX_FORWARD_DEF
    inline entity_manager::entity_manager(registry &r) : _owner(r), _max_entity(0), _graveyard{} {}

    inline entity_t entity_manager::spawn() {
        if (!_graveyard.empty()) {
            auto e = _graveyard.back();
            _graveyard.pop_back();

            return e;
        }

        return entity_t(_max_entity++);
    }

    template <class... Component>
    inline entity_t entity_manager::spawn_with(registry &r, Component &&... component) {
        auto entity = spawn();

        (r.add_component(entity, std::forward<Component>(component)), ...);

        return entity;
    }

    inline entity_manager &entity_manager::remove(entity_t const &e) {
        _graveyard.push_back(e);

        return *this;
    }
#endif
}

#endif /* end of include guard: ENTITY_MANAGER_HPP_ */
