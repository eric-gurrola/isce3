// -*- C++ -*-
//
// michael a.g. aïvázis <michael.aivazis@para-sim.com>
// (c) 2003-2018 all rights reserved
//

// code guard
#if !defined(isce_srtm_Tile_h)
#define isce_srtm_Tile_h


// declaration
class isce::srtm::Tile {
    // types
public:
    // the payload type
    typedef int16_t data_type;

    // storage for my indices
    typedef std::array<int, 2> rep_type;
    // my shape specification
    typedef pyre::geometry::index_t<rep_type> index_type;
    typedef pyre::geometry::layout_t<index_type> shape_type;
    // my storage type
    typedef pyre::memory::constview_t storage_type;
    // my grid
    typedef pyre::geometry::grid_t<data_type, shape_type, storage_type> grid_type;

    // meta-methods
public:
    inline Tile(const void * rawdata, int arcsecondsPerPixel=1);

    // interface
    inline auto shape() const;
    inline auto get(index_type index) const;

    // syntactic sugar
    inline auto operator[](index_type index) const;

    // implementation details
private:
    // data members
    grid_type _grid;

    // disable copy semantics
private:
    Tile(const Tile &) = delete;
    Tile & operator=(const Tile &) = delete;
};


#endif

// end of file
