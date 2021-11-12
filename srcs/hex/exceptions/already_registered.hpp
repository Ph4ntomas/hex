/**
** \file already_registered.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-12 15:01
** \date Last update: 2021-11-12 15:05
*/

#ifndef ALREADY_REGISTERED_HPP_
#define ALREADY_REGISTERED_HPP_

#include <stdexcept>

namespace hex::exceptions {
    class already_registered : public std::runtime_error {
        public:
            virtual ~already_registered(){}

            using std::runtime_error::runtime_error;
            using std::runtime_error::operator=;
            using std::runtime_error::what;
    };
}

#endif /* end of include guard: ALREADY_REGISTERED_HPP_ */
