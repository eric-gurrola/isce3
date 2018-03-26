// -*- C++ -*-
//
// michael a.g. aïvázis <michael.aivazis@para-sim.com>
// (c) 2003-2018 all rights reserved
//

// code guard
#if !defined(isce_image_public_h)
#define isce_image_public_h

// externals
#include <stdexcept>
#include <string>
// support
#include <pyre/journal.h>
#include <pyre/memory.h>
#include <pyre/geometry.h>

// pull useful typedefs
namespace isce {
    namespace image {
        // file names
        typedef pyre::memory::uri_t uri_t;
        // grid layout parts
        template <typename repT> using index_t = pyre::geometry::index_t<repT>;
        template <typename repT> using shape_t = pyre::geometry::shape_t<repT>;
        template <typename indexT> using layout_t = pyre::geometry::layout_t<indexT>;

        // grid
        template <typename cellT, typename layoutT, typename storageT>
        using grid_t = pyre::geometry::grid_t<cellT, layoutT, storageT>;
        // direct grid
        template <typename cellT, typename layoutT, typename directT = pyre::memory::direct_t>
        using directgrid_t = pyre::geometry::directgrid_t<cellT, layoutT, directT>;
    }
}

// forward declarations
namespace isce {
    namespace image {
        // forward declarations of the image api classes
        template <typename pixelT, typename layoutT, typename storageT> class Image;
        template <typename pixelT, typename layoutT, typename storageT> class DirectImage;
    }
}

// type aliases for the above; these are the recommended type names for public access
namespace isce {
    namespace image {
        // image_t: you specify pixel, layout, and storage
        template <typename pixelT, typename layoutT, typename storageT>
        using image_t = Image<pixelT, layoutT, storageT>;

        // directimage_t: specify pixel, layout; storage is memory mapped;
        // instantiate with {constdirect_t> for read-only access
        template <typename pixelT, typename layoutT, typename storageT = pyre::memory::direct_t>
        using directimage_t = DirectImage<pixelT, layoutT, storageT>;
    }
}

// the object model
#include "Image.h"
#include "DirectImage.h"

// the implementations of the inlines
// image
#define isce_image_Image_icc
#include "Image.icc"
#undef isce_image_Image_icc

// memory mapped image
#define isce_image_DirectImage_icc
#include "DirectImage.icc"
#undef isce_image_DirectImage_icc

#endif

// end of file
