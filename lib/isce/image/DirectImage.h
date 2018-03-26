// -*- C++ -*-
//
// michael a.g. aïvázis <michael.aivazis@para-sim.com>
// (c) 2003-2018 all rights reserved
//

// code guard
#if !defined(isce_image_DirectImage_h)
#define isce_image_DirectImage_h


// declaration
template <typename pixelT, typename layoutT, typename storageT>
class isce::image::DirectImage : public directgrid_t<pixelT, layoutT, storageT> {
    // types
public:
    // publish my template parameters
    typedef pixelT pixel_type;
    typedef layoutT layout_type;
    typedef storageT storage_type;
    // dependent types
    typedef typename storage_type::uri_type uri_type;
    typedef typename layout_type::index_type index_type;
    typedef typename layout_type::shape_type shape_type;
    typedef typename layout_type::packing_type packing_type;
    // my parts
    typedef pyre::geometry::grid_t<pixel_type, layout_type, storage_type> grid_type;

    // meta-methods
public:
    inline DirectImage(uri_type name, layout_type layout);
};


#endif

// end of file
