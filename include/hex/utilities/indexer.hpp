/**
** \file indexer.hpp
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-12-11 15:46
** \date Last update: 2021-12-29 20:26
*/

#ifndef utility_indexer_hpp__
#define utility_indexer_hpp__

#include <cstddef> // std::size_t
#include <iterator> // std::input_iterator_tag
#include <limits> // std::numeric_limits
#include <type_traits>

namespace hex::utility {
    /**
    ** \brief Iterator that always return the index. To be used with zip.
    */
    class indexer_iterator {
        public:
            using value_type = std::size_t;
            using reference = value_type;
            using pointer = void;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::input_iterator_tag;


            indexer_iterator() : _state{0}, _is_end(false) {}
            indexer_iterator(bool is_end) : _state{0}, _is_end(is_end) {}

            indexer_iterator(indexer_iterator const &) = default;
            indexer_iterator(indexer_iterator &) noexcept = default;
            ~indexer_iterator() = default;

            indexer_iterator &operator=(indexer_iterator const &) = default;
            indexer_iterator &operator=(indexer_iterator &&) noexcept = default;

            indexer_iterator &operator++() noexcept {
                _is_end = std::numeric_limits<std::size_t>::max() == _state;
                ++_state;
                return *this;
            }

            indexer_iterator operator++(int) noexcept {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            indexer_iterator operator+(difference_type i) {
                auto tmp = *this;
                tmp._state += i;
                tmp._is_end = _is_end || (i < 0 ? _state < tmp._state : _state > tmp._state);

                return tmp;
            }

            friend bool operator==(indexer_iterator const &lhs, indexer_iterator const &rhs) {
                 return lhs._state == rhs._state || (lhs._is_end && rhs._is_end);
            }

            friend bool operator!=(indexer_iterator const &lhs, indexer_iterator const &rhs) {
                return !(lhs == rhs);
            }

            value_type operator*() { return _state; }
            value_type operator->() { return _state; }

            void swap(indexer_iterator &oth) noexcept(std::is_nothrow_swappable_v<std::size_t> && std::is_nothrow_swappable_v<bool>) {
                using std::swap;
                swap(_state, oth._state);
                swap(_is_end, oth._is_end);
            }

        private:
            std::size_t _state;
            bool _is_end;
    };

    /**
    ** \relates indexer_iterator
    */
    void swap(indexer_iterator &lhs, indexer_iterator &rhs) {
        lhs.swap(rhs);
    }

    /**
    ** \brief Pseudo container whose iterators always return their current index. To be used with zip
    **
    ** \see zip
    */
    struct indexer_t {
        std::size_t size() const { return std::numeric_limits<std::size_t>::max(); }
        indexer_iterator begin() { return {}; }
        indexer_iterator end() { return {false}; }

        indexer_iterator begin() const { return {}; }
        indexer_iterator end() const { return {false}; }
    };

    static constexpr indexer_t indexer;
}
#endif /* end of include guard: utility_indexer_hpp__ */
