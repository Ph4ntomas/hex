/**
** \file hex.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-12-05 16:33
** \date Last update: 2021-12-11 16:29
*/

#ifndef HEX_HPP__
#define HEX_HPP__

#include "hex/containers/sparse_array.hpp"
#include "hex/components_registry.hpp"
#include "hex/entity_manager.hpp"
#include "hex/system_registry.hpp"
#include "hex/context.hpp"
#include "hex/iterators/zip.hpp"
#include "hex/utilities/indexer.hpp"

namespace hex {
    using containers::sparse_array;
    using iterators::zip;
    using iterators::izip;
    using utility::indexer;
}

#endif /* end of include guard: HEX_HPP__ */
