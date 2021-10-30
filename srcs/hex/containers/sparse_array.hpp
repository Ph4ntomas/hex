/**
** \file sparse_array.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-10-29 12:28
** \date Last update: 2021-10-31 17:04
*/

#ifndef SPARSE_ARRAY_HPP_
#define SPARSE_ARRAY_HPP_

#include <algorithm>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

namespace hex::containers {
    template <typename T, typename Allocator = std::allocator<std::optional<T>>>
    class sparse_array : std::vector<std::optional<T>, Allocator> {
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
            template <typename U = T>
            sparse_array(size_type count, U &&value, Allocator const &alloc = Allocator()) : base_t(count, std::optional(std::forward<U>(value)), alloc) {}
            //explicit sparse_array(size_type count, Allocator const &alloc = Allocator());

            //template <class InputIt>
            //sparse_array(InputIt first, InputIt last, Allocator const &alloc = Allocator());

            //sparse_array(sparse_array const & oth) : base_t(oth) {}
            //sparse_array(sparse_array const &, Allocator const &alloc) : base_t(
//)

            //sparse_array(sparse_array &&) noexcept;
            //sparse_array(sparse_array &&, Allocator const & alloc);

            sparse_array(std::initializer_list<T> init, Allocator const &alloc = Allocator()) : base_t(init.size(), alloc) {
                if constexpr (std::is_move_constructible_v<T>)
                    std::transform(init.begin(), init.end(), begin(), [](auto &&t){ return std::optional(std::move(t)); });
                else {
                    std::transform(init.begin(), init.end(), begin(), [](auto const &t){ return std::optional(t); });
                }
            }
            //sparse_array(std::initializer_list<value_type> init, Allocator const &alloc = Allocator());

            //~sparse_array();

            using base_t::operator=;
            //sparse_array &operator=(sparse_array const &);
            //sparse_array &operator=(sparse_array &&) noexcept;

            using base_t::assign;
            //void assign(size_t count, T const &value);
            //template <class InputIt>
            //void assign(InputIt first, InputIt last);
            void assign(std::initializer_list<T> init) {
                resize(init.size());

                if constexpr (std::is_move_constructible_v<T>)
                    std::transform(init.begin(), init.end(), begin(), [](auto &&t){ return std::optional(std::move(t)); });
                else {
                    std::transform(init.begin(), init.end(), begin(), [](auto const &t){ return std::optional(t); });
                }
            }

            using base_t::get_allocator;
            //[[nodiscard]] Allocator get_allocator() const noexcept(std::is_nothrow_copy_constructible_v<Allocator>);

            /**
            ** \name Element access
            ** \@{
            */
            using base_t::at;
            //[[nodiscard]] reference at(size_type pos);
            //[[nodiscard]] const_reference at(size_type pos) const;

            using base_t::operator[];
            //[[nodiscard]] reference operator[](size_type pos);
            //[[nodiscard]] const_reference operator[](size_type pos) const;

            using base_t::data;
            //[[nodiscard]] pointer data() noexcept;
            //[[nodiscard]] const_pointer data() const noexcept;

            [[nodiscard]] size_type get_index(T const &t) {
                auto diff = std::addressof(t) - data();

                if (diff < 0 || diff > size())
                    throw std::out_of_range("get_index error: components does not exists in sparse_array.");

                return diff;
            }
            /**
            ** \@}
            */

            /**
            ** \name Iterators
            ** \@{
            **/
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
            /**
            ** \@}
            */

            /**
            ** \name Capacity
            ** \@{
            **/
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
            /**
            ** \@}
            */

            /**
            ** \name Modifier
            ** \@{
            **/
            using base_t::clear;
            //void clear() noexcept;
            reference insert_at(size_type pos, T const & value) {
                maybe_resize(pos);

                return base_t::at(pos) = value;
            }

            reference insert_at(size_type pos, T && value) {
                maybe_resize(pos);

                return base_t::at(pos) = std::forward<T>(value);
            }

            template <class... Args> iterator emplace_at(size_type pos, Args &&... args) {
                maybe_resize(pos);
                auto alloc = get_allocator();

                using traits_t = std::allocator_traits<decltype(alloc)>;

                auto addr = data() + pos;

                traits_t::destroy(alloc, addr);
                traits_t::construct(alloc, addr, std::forward<Args>(args)...);

            }

            void erase_at(const_iterator pos) {
                at(pos) = std::nullopt;
            }

            using base_t::swap;
            /**
            ** \@}
            */

            friend bool operator==(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs);
            friend bool operator!=(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs);
            friend bool operator<(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs);
            friend bool operator>(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs);
            friend bool operator<=(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs);
            friend bool operator>=(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs);
        private:
            void maybe_resize(size_type pos) {
                if (pos > base_t::size())
                    base_t::resize(pos + 1);
            }
    };

    /**
    ** \name Non-member functions
    ** \@{
    **/
    template <typename T, class Allocator>
    [[nodiscard]] inline bool operator==(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs) {
        return (typename sparse_array<T, Allocator>::base_t const &)lhs == (typename sparse_array<T, Allocator>::base_t const &)rhs;
    }

    template <typename T, class Allocator>
    [[nodiscard]] inline bool operator!=(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs) {
        return (typename sparse_array<T, Allocator>::base_t const &)lhs != (typename sparse_array<T, Allocator>::base_t const &)rhs;
    }

    template <typename T, class Allocator>
    [[nodiscard]] inline bool operator<(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs) {
        return (typename sparse_array<T, Allocator>::base_t const &)lhs < (typename sparse_array<T, Allocator>::base_t const &)rhs;
    }

    template <typename T, class Allocator>
    [[nodiscard]] inline bool operator>(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs) {
        return (typename sparse_array<T, Allocator>::base_t const &)lhs > (typename sparse_array<T, Allocator>::base_t const &)rhs;
    }
    template <typename T, class Allocator>
    [[nodiscard]] inline bool operator<=(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs) {
        return (typename sparse_array<T, Allocator>::base_t const &)lhs <= (typename sparse_array<T, Allocator>::base_t const &)rhs;
    }
    template <typename T, class Allocator>
    [[nodiscard]] inline bool operator>=(sparse_array<T, Allocator> const &lhs, sparse_array<T, Allocator> const &rhs) {
        return (typename sparse_array<T, Allocator>::base_t const &)lhs >= (typename sparse_array<T, Allocator>::base_t const &)rhs;
    }
    /**
    ** \@}
    */
}

namespace std {
    template <typename T, class Allocator>
    void swap(hex::containers::sparse_array<T, Allocator> &lhs, hex::containers::sparse_array<T, Allocator> &rhs) {
        return lhs.swap(rhs);
    }
}

#endif /* end of include guard: SPARSE_ARRAY_HPP_ */
