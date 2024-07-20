/**
** \file sparse_array.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-10-29 12:28
** \date Last update: 2024-07-20 16:24
*/

#ifndef SPARSE_ARRAY_HPP_
#define SPARSE_ARRAY_HPP_

#include <algorithm> // std::transform
#include <initializer_list> // std::initializer_list
#include <iterator> // std::iterator_traits
#include <memory> // std::allocator
#include <optional> // std::make_optional, std::nullopt, std::nullopt_t, std::optional
#include <stdexcept> // std::out_of_range
#include <type_traits> // std::decay, std::disjunction, std::enable_if, std::is_constructible, std::is_same
#include <vector> // std::vector
#include <utility> // std::forward

#include "hex/meta/type_traits.hpp"

namespace hex::containers {
    /**
    ** \brief Non packed array.
    **
    ** This class is an array of optional values.
    **
    ** \see SparseArrayDeduc
    */
    template <typename T, typename Allocator = std::allocator<std::optional<T>>>
    class sparse_array : std::vector<std::optional<T>, Allocator> {
        static_assert(!std::is_same_v<T, std::nullopt_t>, "Component type cannot be std::nullopt_t");

        using base_t = std::vector<std::optional<T>, Allocator>;
        public:
            using value_type = typename base_t::value_type;
            using allocator_type = typename base_t::allocator_type;
            using size_type = typename base_t::size_type;
            using difference_type = typename base_t::difference_type;
            using reference = typename base_t::reference;
            using const_reference = typename base_t::const_reference;
            using pointer = typename base_t::pointer;
            using const_pointer = typename base_t::const_pointer;
            using iterator = typename base_t::iterator;
            using const_iterator = typename base_t::const_iterator;

        public:
            using base_t::base_t;
            //sparse_array() noexcept(noexcept(Allocator()));
            //explicit sparse_array(Allocator const & alloc) noexcept;
            template <typename U = T, std::enable_if_t<!
                std::disjunction_v<
                    std::is_same<std::nullopt_t, std::decay_t<U>>,
                    std::is_same<std::allocator<value_type>, std::decay_t<U>>,
                    std::is_same<U, value_type>,
                    std::is_constructible<value_type, U&&>
                >, bool> = true>
                sparse_array(size_type count, U const & value, Allocator const &alloc = Allocator())
                : base_t(count, std::make_optional<std::decay_t<U>>(value), alloc) {}

            //explicit sparse_array(size_type count, Allocator const &alloc = Allocator());

            template <class InputIt, std::enable_if_t<!
                std::disjunction_v<
                    std::is_same<std::nullopt_t, typename std::iterator_traits<InputIt>::value_type>,
                    std::is_same<value_type, typename std::iterator_traits<InputIt>::value_type>,
                    std::is_constructible<value_type, typename std::iterator_traits<InputIt>::value_type>
                >, bool> = true, typename = meta::require_input_iterator<InputIt>>
            sparse_array(InputIt first, InputIt last, Allocator const &alloc = Allocator()) : base_t(last - first, alloc) {
                std::transform(first, last, begin(), [](auto const &t){ return std::make_optional<T>(t); });
            }

            template <class InputIt, typename = std::enable_if_t<
                meta::is_optional_v<typename std::iterator_traits<InputIt>::value_type>>,
                     typename = meta::require_input_iterator<InputIt>>
            sparse_array(InputIt first, InputIt last, Allocator const &alloc = Allocator()) : base_t{first, last, alloc} {}

            //sparse_array(sparse_array const & oth) : base_t(oth) {}
            //sparse_array(sparse_array const &, Allocator const &alloc) : base_t(
//)

            //sparse_array(sparse_array &&) noexcept;
            //sparse_array(sparse_array &&, Allocator const & alloc);


            sparse_array(std::initializer_list<value_type> init, Allocator const &alloc = Allocator()) : base_t(init, alloc) {}

            template <typename U = T, std::enable_if_t<!
                std::disjunction_v<
                    std::is_same<U, value_type>,
                    std::is_constructible<value_type, U&&>
                >, bool> = true>
            sparse_array(std::initializer_list<U> init, Allocator const &alloc = Allocator()) : base_t(init.size(), alloc) {
                std::transform(init.begin(), init.end(), begin(), [](auto const &t){ return std::make_optional<T>(t); });
            }

            //sparse_array(std::initializer_list<value_type> init, Allocator const &alloc = Allocator());

            //~sparse_array();

            using base_t::operator=;
            //sparse_array &operator=(sparse_array const &);
            //sparse_array &operator=(sparse_array &&) noexcept;

            sparse_array &operator=(std::initializer_list<value_type> init) { return reinterpret_cast<sparse_array &>(base_t::operator=(init)); }

            template <typename U = T, std::enable_if_t<!
                std::disjunction_v<
                    std::is_same<std::nullopt_t, U>,
                    std::is_same<U, value_type>,
                    std::is_constructible<value_type, U&&>
                >, bool> = true>
            sparse_array &operator=(std::initializer_list<U> init) {
                assign(init);
                return *this;
            }

            using base_t::assign;
            //void assign(size_t count, T const &value);

            template <typename U = T, std::enable_if_t<!
                std::disjunction_v<
                    std::is_same<std::nullopt_t, std::decay_t<U>>,
                    std::is_same<U, value_type>,
                    std::is_constructible<value_type, U&&>
                >, bool> = true>
                void assign(size_t count, U const &v) {
                     base_t::assign(count, std::make_optional<std::decay_t<U>>(v));
                }

            template <class InputIt, std::enable_if_t<!
                std::disjunction_v<
                    std::is_same<std::nullopt_t, typename std::iterator_traits<InputIt>::value_type>,
                    std::is_same<value_type, typename std::iterator_traits<InputIt>::value_type>,
                    std::is_constructible<value_type, typename std::iterator_traits<InputIt>::value_type>
                >, bool> = true, typename = meta::require_input_iterator<InputIt>>
            void assign(InputIt first, InputIt last) {
                size_t dist = last - first;

                base_t::resize(dist);

                std::transform(first, last, base_t::begin(), [](auto &t){ return std::make_optional<T>(t); });
            }

            template <class InputIt, typename = std::enable_if_t<
                meta::is_optional_v<typename std::iterator_traits<InputIt>::value_type>>,
                     typename = meta::require_input_iterator<InputIt>>
            void assign(InputIt first, InputIt last) { base_t::assign(first, last); }

            void assign(std::initializer_list<value_type> init) { base_t::assign(init); }

            template <typename U = T, std::enable_if_t<!
                std::disjunction_v<
                    std::is_same<std::nullopt_t, U>,
                    std::is_same<U, value_type>,
                    std::is_constructible<value_type, U&&>
                >, bool> = true>
            void assign(std::initializer_list<U> init) {
                base_t::resize(init.size());

                std::transform(init.begin(), init.end(), begin(), [](auto const &t){ return std::make_optional<T>(t); });
            }

            using base_t::get_allocator;
            //[[nodiscard]] Allocator get_allocator() const noexcept(std::is_nothrow_copy_constructible_v<Allocator>);

            /**
            ** \name Element access
            */
            /** @{ */
            using base_t::at;
            //[[nodiscard]] reference at(size_type pos);
            //[[nodiscard]] const_reference at(size_type pos) const;

            //using base_t::operator[];
            [[nodiscard]] reference operator[](size_type pos) {
                if (pos >= size())
                    base_t::resize(pos);

                return base_t::operator[](pos);
            }

            [[nodiscard]] const_reference operator[](size_type pos) const {
                if (pos >= size())
                    return _nullopt;

                return base_t::operator[](pos);
            }

            using base_t::data;
            //[[nodiscard]] pointer data() noexcept;
            //[[nodiscard]] const_pointer data() const noexcept;

            template <typename U = T, typename = std::enable_if_t<
                std::disjunction_v<
                    std::is_same<std::decay_t<U>, T>,
                    std::is_same<std::decay_t<U>, value_type>
                >
            >>
            [[nodiscard]] size_type get_index(U const &t) const {
                auto diff = (value_type *)std::addressof(t) - data();

                if (diff < 0 || diff > size())
                    throw std::out_of_range("get_index error: components does not exists in sparse_array.");

                return diff;
            }
            /** @} */

            /**
            ** \name Iterators
            */
            /** @{ */
            using base_t::begin;
            using base_t::cbegin;
            //[[nodiscard]] iterator begin() noexcept;
            //[[nodiscard]] const_iterator begin() const noexcept;
            //[[nodiscard]] const_iterator cbegin() const noexcept;

            using base_t::end;
            using base_t::cend;
            //[[nodiscard]] iterator end() noexcept;
            //[[nodiscard]] const_iterator end() const noexcept;
            //[[nodiscard]] const_iterator cend() const noexcept;
            /** @} */

            /**
            ** \name Capacity
            */
            /** @{ */
            using base_t::empty;
            using base_t::size;
            using base_t::max_size;
            using base_t::reserve;
            using base_t::capacity;
            using base_t::shrink_to_fit;
            //[[nodiscard]] bool empty() const noexcept;
            //[[nodiscard]] size_type size() const noexcept;
            //[[nodiscard]] size_type max_size() const noexcept;
            //void reserve(size_type new_cap);
            //[[nodiscard]] size_type capacity() const noexcept;
            //void shrink_to_fit();
            /** @} */

            /**
            ** \name Modifier
            */
            /** @{ */
            using base_t::clear;
            //void clear() noexcept;

            template <typename U = T>
            iterator insert_at(size_type pos, U && value) {
                _maybe_resize(pos);

                base_t::at(pos) = std::forward<U>(value);
                return base_t::begin() + pos;
            }

            template <class... Args, typename = std::enable_if_t<
                std::is_constructible_v<value_type, Args...>
            >>
            iterator emplace_at(size_type pos, Args &&... args) {
                _maybe_resize(pos);
                auto alloc = get_allocator();

                using traits_t = std::allocator_traits<decltype(alloc)>;

                auto addr = data() + pos;

                traits_t::destroy(alloc, addr);
                traits_t::construct(alloc, addr, std::forward<Args>(args)...);

                return base_t::begin() + pos;
            }

            template <class... Args, std::enable_if_t<
                !std::is_constructible_v<value_type, Args...>, bool
            > = true>
            iterator emplace_at(size_type pos, Args &&... args) {
                _maybe_resize(pos);
                auto alloc = get_allocator();

                using traits_t = std::allocator_traits<decltype(alloc)>;

                auto addr = data() + pos;

                traits_t::destroy(alloc, addr);
                traits_t::construct(alloc, addr, std::in_place, std::forward<Args>(args)...);

                return base_t::begin() + pos;
            }

            void erase_at(size_type pos) {
                at(pos) = std::nullopt;
            }

            using base_t::resize;
            using base_t::swap;
            /** @} */

            //template <typename T_, class Allocator_> friend bool operator==(sparse_array<T_, Allocator_> const &lhs, sparse_array<T_, Allocator_> const &rhs);
            //template <typename T_, class Allocator_> friend bool operator!=(sparse_array<T_, Allocator_> const &lhs, sparse_array<T_, Allocator_> const &rhs);
            //template <typename T_, class Allocator_> friend bool operator<(sparse_array<T_, Allocator_> const &lhs, sparse_array<T_, Allocator_> const &rhs);
            //template <typename T_, class Allocator_> friend bool operator>(sparse_array<T_, Allocator_> const &lhs, sparse_array<T_, Allocator_> const &rhs);
            //template <typename T_, class Allocator_> friend bool operator<=(sparse_array<T_, Allocator_> const &lhs, sparse_array<T_, Allocator_> const &rhs);
            //template <typename T_, class Allocator_> friend bool operator>=(sparse_array<T_, Allocator_> const &lhs, sparse_array<T_, Allocator_> const &rhs);
        private:
            void _maybe_resize(size_type pos) {
                if (pos >= base_t::size())
                    base_t::resize(pos + 1);
            }

            static constexpr std::optional<T> _nullopt{std::nullopt};
    };

    /**
    ** \defgroup SparseArrayDeduc Sparse array deduction guides
    **
    ** These deduction guide are provided to allow deduction from iterators, or values.
    */
    /** @{ */
    template <typename T, typename = std::enable_if_t<!meta::is_optional_v<T>>>
    sparse_array(size_t count, T) -> sparse_array<T>;

    template <typename T,
             typename Allocator = std::allocator<std::optional<T>>,
             typename = std::enable_if_t<!meta::is_optional_v<T>>>
    sparse_array(size_t count, T, Allocator) -> sparse_array<T>;

    template <typename T>
    sparse_array(size_t count, std::optional<T>) -> sparse_array<T>;

    template <typename T, typename Allocator = std::allocator<std::optional<T>>>
    sparse_array(size_t count, std::optional<T>, Allocator) -> sparse_array<T>;

    template <typename InputIt,
             typename T = typename std::iterator_traits<InputIt>::value_type, class Allocator = std::allocator<std::optional<T>>,
             typename = meta::require_input_iterator<InputIt>,
             typename = std::enable_if_t<!meta::is_optional_v<T>>
    >
    sparse_array(InputIt, InputIt) -> sparse_array<T, Allocator>;

    template <typename InputIt,
             typename T = typename std::iterator_traits<InputIt>::value_type, class Allocator = std::allocator<T>,
             typename = meta::require_input_iterator<InputIt>
    >
    sparse_array(InputIt, InputIt) -> sparse_array<typename T::value_type, Allocator>;

    template <typename InputIt,
             typename T = typename std::iterator_traits<InputIt>::value_type, class Allocator = std::allocator<std::optional<T>>,
             typename = meta::require_input_iterator<InputIt>,
             typename = std::enable_if_t<!meta::is_optional_v<T>>
    >
    sparse_array(InputIt, InputIt, Allocator) -> sparse_array<T, Allocator>;

    template <typename InputIt,
             typename T = typename std::iterator_traits<InputIt>::value_type, class Allocator = std::allocator<T>,
             typename = meta::require_input_iterator<InputIt>
    >
    sparse_array(InputIt, InputIt, Allocator) -> sparse_array<typename T::value_type, Allocator>;

    template <typename T, class Allocator = std::allocator<std::optional<T>>>
    sparse_array(std::initializer_list<T>, Allocator) -> sparse_array<T, Allocator>;

    template <typename T, class Allocator = std::allocator<std::optional<T>>>
    sparse_array(std::initializer_list<std::optional<T>>, Allocator) -> sparse_array<T, Allocator>;

    template <typename T, class Allocator = std::allocator<std::optional<T>>>
    sparse_array(std::initializer_list<T>) -> sparse_array<T, Allocator>;

    template <typename T, class Allocator = std::allocator<std::optional<T>>>
    sparse_array(std::initializer_list<std::optional<T>>) -> sparse_array<T, Allocator>;
    /** @} */

    /**
    ** \brief Equality comparison operator
    **
    ** \relates sparse_array
    */
    template <typename T, class Allocator>
    [[nodiscard]] inline bool operator==(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs) {
        return (typename sparse_array<T, Allocator>::base_t const &)lhs == (typename sparse_array<T, Allocator>::base_t const &)rhs;
    }

    /**
    ** \brief Inequality comparison operator
    **
    ** \relates sparse_array
    */
    template <typename T, class Allocator>
    [[nodiscard]] inline bool operator!=(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs) {
        return (typename sparse_array<T, Allocator>::base_t const &)lhs != (typename sparse_array<T, Allocator>::base_t const &)rhs;
    }

    /**
    ** \brief Less-than comparison operator
    **
    ** \relates sparse_array
    */
    template <typename T, class Allocator>
    [[nodiscard]] inline bool operator<(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs) {
        return (typename sparse_array<T, Allocator>::base_t const &)lhs < (typename sparse_array<T, Allocator>::base_t const &)rhs;
    }

    /**
    ** \brief More-than comparison operator
    **
    ** \relates sparse_array
    */
    template <typename T, class Allocator>
    [[nodiscard]] inline bool operator>(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs) {
        return (typename sparse_array<T, Allocator>::base_t const &)lhs > (typename sparse_array<T, Allocator>::base_t const &)rhs;
    }

    /**
    ** \brief Less-than-or-equal comparison operator
    **
    ** \relates sparse_array
    */
    template <typename T, class Allocator>
    [[nodiscard]] inline bool operator<=(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs) {
        return (typename sparse_array<T, Allocator>::base_t const &)lhs <= (typename sparse_array<T, Allocator>::base_t const &)rhs;
    }

    /**
    ** \brief More-than-or-equal comparison operator
    **
    ** \relates sparse_array
    */
    template <typename T, class Allocator>
    [[nodiscard]] inline bool operator>=(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs) {
        return (typename sparse_array<T, Allocator>::base_t const &)lhs >= (typename sparse_array<T, Allocator>::base_t const &)rhs;
    }

    /**
    ** \brief Swap two containers.
    */
    template <typename T, class Allocator>
    void swap(hex::containers::sparse_array<T, Allocator> &lhs, hex::containers::sparse_array<T, Allocator> &rhs) {
        return lhs.swap(rhs);
    }
}

namespace std {
}

#endif /* end of include guard: SPARSE_ARRAY_HPP_ */
