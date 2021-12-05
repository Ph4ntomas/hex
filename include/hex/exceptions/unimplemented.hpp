/**
** \file unimplemented.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-11-12 11:24
** \date Last update: 2021-11-12 11:26
*/

#ifndef UNIMPLEMENTED_HPP_
#define UNIMPLEMENTED_HPP_

#include <stdexcept>

namespace hex::exceptions {
    struct unimplemented : std::runtime_error {
        using std::runtime_error::runtime_error;
        virtual ~unimplemented() {}
        using std::runtime_error::operator=;
        using std::runtime_error::what;
    };
}

#endif /* end of include guard: UNIMPLEMENTED_HPP_ */
