/**
** \file no_such_entity.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-14 23:53
** \date Last update: 2021-11-14 23:55
*/

#ifndef NO_SUCH_ENTITY_HPP_
#define NO_SUCH_ENTITY_HPP_

#include <stdexcept>

namespace hex::exceptions {
    class no_such_entity : public std::invalid_argument {
        public:
            using std::invalid_argument::invalid_argument;
            using std::invalid_argument::operator=;
            using std::invalid_argument::what;
    };
}

#endif /* end of include guard: NO_SUCH_ENTITY_HPP_ */
