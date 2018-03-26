// -*- C++ -*-
//
// michael a.g. aïvázis <michael.aivazis@para-sim.com>
// (c) 2003-2018 all rights reserved
//

// code guard
#if !defined(isce_image_Image_h)
#define isce_image_Image_h


// declaration
template <typename pixelT, typename layoutT, typename storageT>
class isce::image::Image : public grid_t<pixelT, layoutT, storageT> {
    // types
public:
    // publish my template parameters
    typedef pixelT pixel_type;
    typedef layoutT layout_type;
    typedef storageT storage_type;
    // dependent types
    typedef typename layout_type::index_type index_type;
    typedef typename layout_type::shape_type shape_type;
    typedef typename layout_type::packing_type packing_type;
    // my parts
    typedef grid_t<pixel_type, layout_type, storage_type> grid_type;

    // meta-methods
public:
    inline Image(layout_type layout, const storage_type & storage);
    inline Image(layout_type layout, storage_type && storage);
};


#endif

// end of file
