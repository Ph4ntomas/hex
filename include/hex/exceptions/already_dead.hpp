/**
** \file already_dead.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-14 23:46
** \date Last update: 2021-11-14 23:48
*/

#ifndef ALREADY_DEAD_HPP_
#define ALREADY_DEAD_HPP_

#include <stdexcept>

namespace hex::exceptions {
    class already_dead : public std::invalid_argument {
        public:
            using std::invalid_argument::invalid_argument;
            using std::invalid_argument::operator=;
            using std::invalid_argument::what;
    };
}

#endif /* end of include guard: ALREADY_DEAD_HPP_ */
