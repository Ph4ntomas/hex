/**
** \file hex.hpp
**
** \author Phantomas <phantomas@phantomas.xyz>
** \date Created on: 2021-12-05 16:33
** \date Last update: 2021-12-29 19:23
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

/**
** \brief Hex main namespace.
*/
namespace hex {
    /// Re-expose sparse_array as hex::sparse_array.
    using containers::sparse_array;

    /// Re-expose zip as hex::zip.
    using iterators::zip;

    /// Re-expose izip as hex::izip.
    using iterators::izip;

    /// Re-expose index as hex::indexer.
    using utility::indexer;
}

#endif /* end of include guard: HEX_HPP__ */
