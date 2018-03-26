// -*- C++ -*-
//
// michael a.g. aïvázis <michael.aivazis@para-sim.com>
// (c) 2003-2018 all rights reserved
//

// code guard
#if !defined(isce_srtm_public_h)
#define isce_srtm_public_h

// externals
#include <arpa/inet.h>
// support
#include <pyre/journal.h>
#include <pyre/memory.h>
#include <pyre/geometry.h>

// forward declarations
namespace isce {
    namespace srtm {
        // local type aliases
        // for filenames
        typedef pyre::memory::uri_t uri_t;
        // for describing shapes and regions
        typedef pyre::memory::offset_t offset_t;
        typedef pyre::memory::size_t size_t;

        // forward declarations of the srtm api classes
        class AvailabilityMap;
        class Tile;
    }
}

// the object model
#include "AvailabilityMap.h"
#include "Tile.h"

// namespace additions
namespace isce {
    namespace srtm {
        // tile availability map
        typedef AvailabilityMap map_t;
        // access to individual tile data
        typedef Tile tile_t;
    }
}


// the implementations of the inlines
// the availability map
#define isce_srtm_AvailabilityMap_icc
#include "AvailabilityMap.icc"
#undef isce_srtm_AvailabilityMap_icc

// the srtm tiles
#define isce_srtm_Tile_icc
#include "Tile.icc"
#undef isce_srtm_Tile_icc


#endif

// end of file
