/**
** \file no_such_component.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-17 16:12
** \date Last update: 2021-12-04 17:15
*/

#ifndef NO_SUCH_COMPONENT_HPP_
#define NO_SUCH_COMPONENT_HPP_

#include <stdexcept>

namespace hex::exceptions {
    class no_such_component : std::out_of_range {
        public:
            using std::out_of_range::out_of_range;
            using std::out_of_range::operator=;
            using std::out_of_range::what;
    };
}

#endif /* end of include guard: NO_SUCH_COMPONENT_HPP_ */
