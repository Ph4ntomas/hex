/**
** \file zip.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-12-09 11:08
** \date Last update: 2021-12-29 20:24
*/

#ifndef iterators_zip_hpp__
#define iterators_zip_hpp__

#include <cstddef> // std::ptrdiff_t, std::size_t
#include <iterator> // std::input_iterator_tag
#include <tuple> // std::tuple
#include <type_traits> // std::conjunction
#include <utility> // std::declval, std::forward, swap

#include "hex/meta/type_traits.hpp"
#include "hex/utilities/indexer.hpp"

namespace hex::iterators {
    /**
    ** \cond Internals
    */
    namespace __impl {
        template <class Container>
        struct iterator_helper {
            using iter_t = decltype(std::declval<Container>().begin());
            using value_type = decltype(std::declval<typename iter_t::reference>().value()) &;
            static value_type to_value(typename iter_t::reference &v) { return v.value(); }
        };

        template <>
            struct iterator_helper<utility::indexer_t> {
                using iter_t = decltype(utility::indexer.begin());
                using value_type = utility::indexer_iterator::reference;

                static value_type to_value(value_type v) { return v; }
            };
    }
    /**
    ** \endcond
    */

    /**
    ** \brief Iterate over several containers at once.
    **
    ** This iterator enable the use of range-based for loop or standard algorithm.
    **
    ** It work by storing iterators over specific containers, increasing these all at the same time, and skipping the ones where the optional evaluate to false.
    ** Where dereferenced, the iterator return a tuple of reference, as if the tuple was created by dereferencing every iterator it stores.
    **
    ** \tparam Containers Type of the container to iterate over.
    */
    template <class... Containers>
    class zip_iterator {
        public:
            template <typename Container>
            using iter_t = typename __impl::iterator_helper<Container>::iter_t;
            using iter_tuple = std::tuple<iter_t<Containers>...>;
            using cont_tuple = std::tuple<Containers...>;

            static_assert(sizeof...(Containers) != 0, "Cannot zip no containers.");

            static_assert(std::conjunction_v<
                    std::disjunction<
                        meta::is_optional<typename iter_t<Containers>::value_type>,
                        std::is_same<Containers, utility::indexer_t>
                    >...
            >, "zip_iterator are designed to work with container of optional value.");

            template <typename Container>
            using inner_t = typename __impl::iterator_helper<Container>::value_type;

            using value_type = std::tuple<inner_t<Containers>...>;
            using reference = value_type;
            using pointer = void;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::input_iterator_tag;

        public:
            zip_iterator(std::tuple<iter_t<Containers>...> const &it_tuple, std::size_t max, void *from = nullptr) : _state(it_tuple), _max(max), _idx{0}, _from{from} {
                while (_idx != _max && !_all_set(_idx_seq)) {
                    ++(*this);
                }
            }
            zip_iterator(zip_iterator const &oth) = default;
            zip_iterator(zip_iterator &&) noexcept = default;

            ~zip_iterator() = default;

            zip_iterator &operator=(zip_iterator const &) = default;
            zip_iterator &operator=(zip_iterator &&) noexcept = default;

            zip_iterator &operator++() { _increment(_idx_seq); return *this; }
            zip_iterator operator++(int) { auto r = *this; _increment(_idx_seq); return r; }

            value_type operator*() { return _to_value(_idx_seq); }
            value_type operator->() { return _to_value(_idx_seq); }

            friend bool operator==(zip_iterator const &lhs, zip_iterator const &rhs) { return (lhs._from == rhs._from && lhs._idx == rhs._idx) || (lhs._idx == lhs._max && rhs._idx == rhs._max); }
            friend bool operator!=(zip_iterator const &lhs, zip_iterator const &rhs) { return !(lhs == rhs); }

            void swap(zip_iterator &oth) noexcept(std::is_nothrow_swappable_v<iter_tuple> &&
                                                  std::is_nothrow_swappable_v<std::size_t> &&
                                                  std::is_nothrow_swappable_v<void *>) {
                using std::swap;
                swap(_state, oth._state);
                swap(_max, oth._max);
                swap(_idx, oth._idx);
                swap(_from, oth._from);
            }
        private:
            template <size_t... Idx>
            bool _all_set(std::index_sequence<Idx...>) {
                return (true && ... &&
                        (std::is_same_v<utility::indexer_iterator, std::tuple_element_t<Idx, iter_tuple>> || *std::get<Idx>(_state)));
            }

            template <size_t... Idx>
            void _increment(std::index_sequence<Idx...>) {
                if (_idx != _max) {
                    do {
                        ++_idx;
                        (++(std::get<Idx>(_state)), ...);
                    } while (_idx != _max && !_all_set(_idx_seq));
                }
            }

            template <size_t... Idx>
            value_type _to_value(std::index_sequence<Idx...>) {
                return {__impl::iterator_helper<std::tuple_element_t<Idx, cont_tuple>>::to_value(*std::get<Idx>(_state))...};
            }
        private:
            iter_tuple _state;
            std::size_t _max;
            std::size_t _idx;

            void *_from;

            static constexpr std::index_sequence_for<Containers...> _idx_seq{};
    };

    /**
    ** \related zip_iterator
    */
    template <class... Containers>
    void swap(zip_iterator<Containers...> &lhs, zip_iterator<Containers...> &rhs) noexcept(noexcept(lhs.swap(rhs)))
    { lhs.swap(rhs); }

    /**
    ** \brief Pseudo-container that enable multi-array iteration.
    **
    ** The purpose of this class is to enable the use of ranged-base for loop on several components at once.
    ** The iterator produced skip indices for which at least one of the optional evaluate to false.
    **
    ** \tparam Containers Types of the container we want to iterate upon.
    */
    template <class... Containers>
    class zip {
        public:
            using iterator = zip_iterator<Containers...>;
            using iter_tuple = typename iterator::iter_tuple;
            using size_type = std::size_t;

            /**
            ** \brief Construct a zip from a list of containers
            **
            ** \param containers Parameter pack containing each container to iterate uppon.
            */
            zip(Containers &... containers) : _size(_compute_size(containers...)), _begin(containers.begin()...), _end(_compute_end(_size, containers...)) {}
            zip(zip const &) = default;
            zip(zip &&) noexcept = default;

            /**
            ** \brief Get a zip_iterator to the beginning of this container.
            */
            iterator begin() { return iterator{_begin, _size, this}; }

            /**
            ** \brief Get a zip_iterator to the end of this container.
            */
            iterator end() { return iterator{_end, 0}; }

        private:
            static size_t _compute_size(Containers const &... containers) {
                return std::min({containers.size()...});
            }

            static iter_tuple _compute_end(size_t size, Containers &... containers) {
                size_t sz = _compute_size(containers...);

                return std::tuple{(containers.begin() + sz)...};
            }
        private:
            size_t _size;
            iter_tuple _begin;
            iter_tuple _end;
    };

    /**
    ** \brief Specialized version of zip, that provide index.
    **
    ** \tparam Containers Types of the container we want to iterate upon.
    */
    template <class... Containers>
        class izip : public zip<utility::indexer_t, Containers...> {
            using base_t = zip<utility::indexer_t, Containers...>;

            public:
                izip(Containers &... cs) : base_t{const_cast<utility::indexer_t &>(utility::indexer), cs...} {}
                using base_t::base_t;

                using base_t::operator=;
                using base_t::begin;
                using base_t::end;
        };
}

#endif /* end of include guard: iterators_zip_hpp__ */
