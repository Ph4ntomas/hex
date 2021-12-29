/**
** \file components_registry.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-12 11:09
** \date Last update: 2021-12-29 18:02
*/

#ifndef COMPONENTS_REGISTRY_HPP_
#define COMPONENTS_REGISTRY_HPP_

#include <any> // std::any, std::any_cast, std::make_any
#include <cstddef> // std::size_t
#include <functional> // std::function
#include <typeinfo> // typeid
#include <typeindex> // std::type_index
#include <type_traits> // std::decay_t
#include <unordered_map> // std::unordered_map
#include <utility> // std::as_const, std::forward, std::function

#include "hex/containers/sparse_array.hpp"
#include "hex/exceptions/already_registered.hpp"

namespace hex {
    /**
    ** \brief Manage Components.
    **
    ** The Component Registry is one of the core component of HEX. Its purpose is to store every collection of components,
    ** and provide a type safe way to retrieve those.
    ** Every component must first have its type registered to the registry, so that it can initialize both the storage, and a way to erase the components.
    **
    ** Internally, every components collections are stored in a std::unordered_map, through the use of an std::any to provide type erasure. The components are
    ** any_cast back to their proper type upon retrieval.
    */
    class components_registry {
        public:
            /**
            ** \brief Helper type for a component container.
            **
            ** \tparam T The type of component,
            */
            template <class T>
            using container_t = containers::sparse_array<std::decay_t<T>>;

        public:
            /**
            ** \brief Default constructor.
            **
            ** This constructor initialize the registry internals.
            */
            components_registry() = default;
            components_registry(components_registry const &) = delete;

            /**
            ** \brief Move constructor
            **
            ** Move-construct a registry.
            **
            ** \param [in] cr components_registry to be moved-from.
            **
            ** \pre <b>cr</b> should be in a valid state.
            **
            ** \post The old registry should not be used afterward.
            */
            components_registry(components_registry &&cr) noexcept = default;
            components_registry & operator=(components_registry const &) = delete;

            /**
            ** \brief Move assignment operator
            **
            ** Move-assign a registry.
            **
            ** \param [in] cr components_registry to be moved-from.
            **
            ** \pre <b>cr</b> should be in a valid state.
            **
            ** \post The old registry should not be used afterward.
            ** \post Any components stored by the registry are destroyed in the process.
            */
            components_registry & operator=(components_registry &&cr) noexcept = default;

            /**
            ** \name Collection managment
            */
            /** @{ */
            /**
            ** \brief Register a new component type.
            **
            ** Registering a new type in the registry creates a new collection to store this type, and set up internal utilities to manage components lifetime.
            **
            ** \tparam Component Type to register.
            **
            ** \return Returns a reference to the components' collection.
            **
            ** \throw exceptions::already_registered is thrown if the Component type was already registered.
            **
            ** \pre Component must not have been registered to the registry.
            **
            ** \post Component collection is properly initialized.
            */
            template <typename Component>
            container_t<Component> &register_type() {
                auto && [v, ok] = try_register_type<Component>();

                if (!ok)
                    throw exceptions::already_registered(typeid(std::decay_t<Component>).name());

                return v;
            }

            /**
            ** \brief Try to register a new type.
            **
            ** This function tries to register a new type to the registry.
            ** If the type was already registered, no operations are performed.
            ** If it wasn't, this function initialize internal utilities to manage components lifetime.
            **
            ** \tparam Component Type to register.
            **
            ** \return Returns a pair containing both a reference to the components' collection, and a boolean value that represent whether an action was performed or not.
            */
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

            /**
            ** \brief Check whether a type has been registered or not.
            **
            ** \tparam Component Type to check.
            **
            ** \return True if the type has been registered already.
            */
            template <typename Component>
            [[nodiscard]] bool has() {
                return _registry.find(std::type_index{typeid(std::decay_t<Component>)}) != _registry.end();
            }

            /**
            ** \brief Retrieve a component collection.
            **
            ** \tparam Component Type of component to retrieve.
            **
            ** \return Returns a reference on the component container.
            **
            ** \throw std::out_of_range is thrown if the component wasn't registered beforehand.
            **
            ** \pre The component type must have been registered.
            */
            template <typename Component>
            [[nodiscard]]
            container_t<Component> &get() {
                return const_cast<container_t<Component> &>(std::as_const(*this).get<Component>());
            }

            /**
            ** \brief Retrieve a component collection.
            **
            ** \tparam Component Type of component to retrieve.
            **
            ** \return Returns a reference on the component container.
            **
            ** \throw std::out_of_range is thrown if the component wasn't registered beforehand.
            **
            ** \pre The component type must have been registered.
            */
            template <typename Component>
            [[nodiscard]]
            container_t<Component> const &get() const {
                return std::any_cast<container_t<Component> const &>(_registry.at(typeid(std::decay_t<Component>)));
            }
            /** @} */

            /**
            ** \name Component managment
            */
            /** @{ */

            /**
            ** \brief Insert a component at a given index in its collection.
            **
            ** This function is a short hand for:
            ** <code>
            **  registry.get<Component>().insert_at(idx, std::forward<Component>(c));
            ** </code>
            **
            ** \tparam Component Type of component to retrieve.
            **
            ** \param [in] idx Index at which the component will be inserted.
            ** \param [in] c Component to insert.
            **
            ** \return Returns a reference on the newly inserted component.
            **
            ** \throw std::out_of_range is thrown if the component wasn't registered beforehand.
            **
            ** \pre The component type must have been registered.
            */
            template <typename Component>
            Component & insert_at(std::size_t idx, Component &&c) {
                auto & cont = get<Component>();

                cont.insert_at(idx, std::forward<Component>(c));

                return cont.at(idx).value();
            }

            /**
            ** \brief Emplace a component at a given index in its collection.
            **
            ** The component constructor will be called in place.
            **
            ** This function is a short hand for:
            ** <code>
            **  registry.get<Component>().emplace_at(idx, std::forward<Params>(ps)...);
            ** </code>
            **
            ** \tparam Component Type of component to retrieve.
            ** \tparam Params Types of the component constructor.
            **
            ** \param [in] idx Index at which the component will be inserted.
            ** \param [in] ps Parameter pack that contains the component construtor parameters.
            **
            ** \return Returns a reference on the newly constructed component.
            **
            ** \throw std::out_of_range is thrown if the component wasn't registered beforehand.
            **
            ** \pre The component type must have been registered.
            */
            template <typename Component, class... Params>
            Component & emplace_at(std::size_t idx, Params &&... ps) {
                auto &cont = get<Component>();

                cont.emplace_at(idx, std::forward<Params>(ps)...);

                return cont.at(idx).value();
            }

            /**
            ** \brief Remove the component at the given index.
            **
            ** This function is a short hand for:
            ** <code>
            **  registry.get<Component>().erase_at(idx);
            ** </code>
            **
            ** \tparam Component Type of component to retrieve.
            **
            ** \param [in] index Index at which the component will be inserted.
            **
            ** \throw std::out_of_range is thrown if the component wasn't registered beforehand.
            **
            ** \pre The component type must have been registered.
            */
            template <typename Component>
            void remove_at(std::size_t index) {
                auto &cont = get<Component>();

                if (cont.size() > index)
                    cont.erase_at(index);
            }

            /**
            ** \brief Remove all components at the given index.
            **
            ** This function call container_t::erase_at for all components at a given index. It effectively remove all components for a given entity.
            **
            ** \param [in] index Index at which the component will be inserted.
            */
            void erase_at(std::size_t index) {
                for (auto &&f : _erasers) {
                    f(*this, index);
                }
            }
            /** @} */
        private:
            std::unordered_map<std::type_index, std::any> _registry;

            std::vector<std::function<void(components_registry &, std::size_t)>> _erasers;
    };
}

#endif /* end of include guard: COMPONENTS_REGISTRY_HPP_ */
