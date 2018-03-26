// -*- C++ -*-
//
// michael a.g. aïvázis <michael.aivazis@para-sim.com>
// (c) 2003-2018 all rights reserved
//

// code guard
#if !defined(isce_srtm_AvailabilityMap_h)
#define isce_srtm_AvailabilityMap_h


// declaration
// this class is a wrapper around the os calls
class isce::srtm::AvailabilityMap {
    // types
public:
    // for my shape
    typedef std::array<size_t, 2> rep_type;
    typedef pyre::geometry::index_t<rep_type> index_type;
    typedef pyre::geometry::layout_t<index_type> shape_type;
    // for my grid
    typedef unsigned char cell_type;
    typedef pyre::geometry::directgrid_t<cell_type, shape_type> grid_type;
    // dependent types
    typedef grid_type::uri_type uri_type;
    typedef grid_type::size_type size_type;


    // meta-methods
public:
    inline AvailabilityMap(uri_type uri);

    // interface
public:
    // size
    inline auto size() const;

    // read and write access using offsets
    inline auto & operator[](size_type offset);
    inline auto operator[](size_type offset) const;

    // read and write access using indices
    inline auto & operator[](const index_type &index);
    inline auto operator[](const index_type & index) const;

    // data
private:
    grid_type _grid;
};

#endif

// end of file
