// -*- C++ -*-
//
// michael a.g. aïvázis <michael.aivazis@para-sim.com>
// (c) 2003-2018 all rights reserved
//

// configuration
#include <portinfo>

// externals
#include <Python.h>
#include <pyre/journal.h>
#include <valarray>
// support
#include <isce/srtm.h>
// my declarations
#include "srtm.h"

// tile availability map
// the capsule marker
const char * const
isce::extension::srtm::
availabilityMap_capsule = "srtmAvailabilityMap_capsule";

// availability map destructor
static void freeAvailabilityMap(PyObject *);

// availability map constructor
const char * const
isce::extension::srtm::
availabilityMap__name__ = "srtmAvailabilityMap";

const char * const
isce::extension::srtm::
availabilityMap__doc__ = "build a map of the status of SRTM data tiles";

PyObject *
isce::extension::srtm::
availabilityMap(PyObject *, PyObject * args)
{
    // storage for the name of the file
    const char * uri;

    // parse the arguments
    int ok = PyArg_ParseTuple(args, "s:srtmAvailabilityMap", &uri);
    // if something went wrong
    if (!ok) {
        // complain
        return 0;
    }

    // grab the map
    isce::srtm::map_t * map = new isce::srtm::map_t(uri);

    // dress it up and return it
    return PyCapsule_New(map, availabilityMap_capsule, freeAvailabilityMap);
}


// summary
const char * const
isce::extension::srtm::
availabilityMapSummary__name__ = "srtmAvailabilityMapSummary";

const char * const
isce::extension::srtm::
availabilityMapSummary__doc__ = "build a summary of the status of SRTM data tiles";

PyObject *
isce::extension::srtm::
availabilityMapSummary(PyObject *, PyObject * args)
{
    // storage
    size_t range;
    PyObject * capsule;
    // parse the arguments
    int ok = PyArg_ParseTuple(args,
                              "O!k:srtmAvailabilityMapSummary",
                              &PyCapsule_Type, &capsule, &range);
    // if something went wrong
    if (!ok) {
        // complain
        return 0;
    }
    // if the capsule is not valid
    if (!PyCapsule_IsValid(capsule, availabilityMap_capsule)) {
        // set the error string
        PyErr_SetString(PyExc_TypeError, "invalid matrix capsule");
        // and complain
        return 0;
    }

    // grab the map
    auto & map =
        * static_cast<isce::srtm::map_t *>(PyCapsule_GetPointer(capsule, availabilityMap_capsule));

    // allocate storage for the use count
    std::valarray<int> table(range);

    // visit the table
    for (size_t slot=0; slot < map.size(); ++slot) {
        // get the status
        size_t value = map[slot];

#if defined(DEBUG_CHECK_BOUNDS)
        // make sure the value is within the range
        if (value >= range) {
            // open a channel
            pyre::journal::firewall_t firewall("isce.grid.bounds");
            // complain
            firewall
                << pyre::journal::at(__HERE__)
                << "index error: out of range: " << value << " > " << range-1
                << pyre::journal::endl;

        }
#endif

        // update the frequency count
        ++table[value];
    }

    // build the result
    PyObject * result = PyTuple_New(range);
    // visit the table
    for (size_t slot=0; slot<range; ++slot) {
        // make an int to hold the frequency of this status
        PyObject * item = PyLong_FromLong(table[slot]);
        // attach it to the tuple
        PyTuple_SET_ITEM(result, slot, item);
    }

    // all done
    return result;
}


// read access
const char * const
isce::extension::srtm::
availabilityMapGet__name__ = "srtmAvailabilityMapGet";

const char * const
isce::extension::srtm::
availabilityMapGet__doc__ = "build a map of the status of SRTM data tiles";

PyObject *
isce::extension::srtm::
availabilityMapGet(PyObject *, PyObject * args)
{
    // storage
    int lat, lon;
    PyObject * capsule;
    // parse the arguments
    int ok = PyArg_ParseTuple(args,
                                  "O!(ii):srtmAvailabilityMapGet",
                                  &PyCapsule_Type, &capsule,
                                  &lat, &lon);
    // if something went wrong
    if (!ok) {
        // complain
        return 0;
    }
    // if the capsule is not valid
    if (!PyCapsule_IsValid(capsule, availabilityMap_capsule)) {
        // set the error string
        PyErr_SetString(PyExc_TypeError, "invalid matrix capsule");
        // and complain
        return 0;
    }
    // check the latitude
    if (lat > 90 || lat < -90) {
        // set the error string
        PyErr_SetString(PyExc_ValueError, "latitude must be in the range (-90, 90)");
        // and complain
        return 0;
    }
    // check the longitude
    if (lon > 180 || lon < -180) {
        // set the error string
        PyErr_SetString(PyExc_ValueError, "longitude must be in the range (-180, 180)");
        // and complain
        return 0;
    }

    // grab the map
    auto & map =
        * static_cast<isce::srtm::map_t *>(PyCapsule_GetPointer(capsule, availabilityMap_capsule));

    // make an index
    isce::srtm::map_t::index_type
        index { static_cast<size_t>(180+lon), static_cast<size_t>(90-lat)};

    // get the value
    auto value = map[index];

    // make a channel
    pyre::journal::debug_t channel("isce.srtm");
    // show me what's about to happen
    channel
        << pyre::journal::at(__HERE__)
        << "tile@(" << lat << "," << lon << ") <- "
        << "(" << index[0] << ", " << index[1] << ") -> "
        << static_cast<size_t>(value)
        << pyre::journal::endl;

    // dress it up and return it
    return PyLong_FromLong(value);
}


// write access
const char * const
isce::extension::srtm::
availabilityMapSet__name__ = "srtmAvailabilityMapSet";

const char * const
isce::extension::srtm::
availabilityMapSet__doc__ = "build a map of the status of SRTM data tiles";

PyObject *
isce::extension::srtm::
availabilityMapSet(PyObject *, PyObject * args)
{
    // storage
    char value;
    int lat, lon;
    PyObject * capsule;
    // parse the arguments
    int ok = PyArg_ParseTuple(args,
                                  "O!(ii)b:srtmAvailabilityMapSet",
                                  &PyCapsule_Type, &capsule,
                                  &lat, &lon,
                                  &value);
    // if something went wrong
    if (!ok) {
        // complain
        return 0;
    }
    // if the capsule is not valid
    if (!PyCapsule_IsValid(capsule, availabilityMap_capsule)) {
        // set the error string
        PyErr_SetString(PyExc_TypeError, "invalid matrix capsule");
        // and complain
        return 0;
    }
    // check the latitude
    if (lat > 90 || lat < -90) {
        // set the error string
        PyErr_SetString(PyExc_ValueError, "latitude must be in the range (-90, 90)");
        // and complain
        return 0;
    }
    // check the longitude
    if (lon > 180 || lon < -180) {
        // set the error string
        PyErr_SetString(PyExc_ValueError, "longitude must be in the range (-180, 180)");
        // and complain
        return 0;
    }

    // grab the map
    auto & map =
        * static_cast<isce::srtm::map_t *>(PyCapsule_GetPointer(capsule, availabilityMap_capsule));

    // make an index
    isce::srtm::map_t::index_type
        index { static_cast<size_t>(180+lon), static_cast<size_t>(90-lat)};

    // make a channel
    pyre::journal::debug_t channel("isce.srtm");
    // show me what's about to happen
    channel
        << pyre::journal::at(__HERE__)
        << "tile@(" << lat << "," << lon << ") <- "
        << "(" << index[0] << ", " << index[1] << ") <-"
        << static_cast<size_t>(value)
        << pyre::journal::endl;

    // perform the assignment
    map[index] = value;

    // all done
    Py_INCREF(Py_None);
    return Py_None;
}

// destructor
void
freeAvailabilityMap(PyObject * capsule)
{
    // get the capsule signature
    const char * capsule_sig = isce::extension::srtm::tile_capsule;
    // bail out if the capsule is not valid
    if (!PyCapsule_IsValid(capsule, capsule_sig)) return;
    // grab the map
    isce::srtm::map_t * map =
        static_cast<isce::srtm::map_t *>(PyCapsule_GetPointer(capsule, capsule_sig));
    // deallocate
    delete map;
    // and return
    return;
}


// srtm tiles
// the capsule marker
const char * const
isce::extension::srtm::
tile_capsule = "srtmTile_capsule";

// tile destructor
static void freeTile(PyObject *);

// tile constructor
const char * const
isce::extension::srtm::
tile__name__ = "srtmTile";

const char * const
isce::extension::srtm::
tile__doc__ = "access the elevation data in a given SRTM tile";

PyObject *
isce::extension::srtm::
tile(PyObject *, PyObject * args)
{
    // the tile resolution
    int resolution = 1; // in arcseconds per pixel
    // the raw tile data
    PyObject * bytes = 0;

    // parse the arguments
    int ok = PyArg_ParseTuple(
                              args,
                              "O!i:srtmTile",
                              &PyBytes_Type, &bytes,
                              &resolution);
    // if something went wrong
    if (!ok) {
        // complain
        return 0;
    }

    // get the underlying data...

    // N.B.: the python tile must guarantee that the bytes live at least as long as the capsule
    // itself; otherwise, the view constructed by srtm::tile_t looks over bad memory
    const void * rawdata = PyBytes_AsString(bytes);
    // make a tile
    isce::srtm::tile_t * tile =
        new isce::srtm::tile_t(static_cast<isce::srtm::tile_t::const_pointer>(rawdata), resolution);

    // dress it up and return it
    return PyCapsule_New(tile, tile_capsule, freeTile);
}

// shape info
const char * const
isce::extension::srtm::
tileShape__name__ = "srtmTileShape";

const char * const
isce::extension::srtm::
tileShape__doc__ = "get the shape of the tile";

PyObject *
isce::extension::srtm::
tileShape(PyObject *, PyObject * args)
{
    // storage
    PyObject * capsule;
    // parse the arguments
    int ok = PyArg_ParseTuple(args,
                              "O!:srtmTileShape",
                               &PyCapsule_Type, &capsule);
    // if something went wrong
    if (!ok) {
        // complain
        return 0;
    }
    // if the capsule is not valid
    if (!PyCapsule_IsValid(capsule, tile_capsule)) {
        // set the error string
        PyErr_SetString(PyExc_TypeError, "invalid tile capsule");
        // and complain
        return 0;
    }

    // grab the tile
    auto & tile =
        * static_cast<isce::srtm::tile_t *>(PyCapsule_GetPointer(capsule, tile_capsule));

    // make an index
    auto shape = tile.shape().shape();

    // the result
    PyObject * result = PyTuple_New(2);
    // move the values
    PyTuple_SET_ITEM(result, 0, PyLong_FromLong(shape[0]));
    PyTuple_SET_ITEM(result, 1, PyLong_FromLong(shape[1]));
    // all done
    return result;
}

// read access
const char * const
isce::extension::srtm::
tileGet__name__ = "srtmTileGet";

const char * const
isce::extension::srtm::
tileGet__doc__ = "get the elevation at a given (i,j) in the tile";

PyObject *
isce::extension::srtm::
tileGet(PyObject *, PyObject * args)
{
    // storage
    int i, j;
    PyObject * capsule;
    // parse the arguments
    int ok = PyArg_ParseTuple(args,
                                  "O!(ii):srtmTileGet",
                                  &PyCapsule_Type, &capsule,
                                  &i, &j);
    // if something went wrong
    if (!ok) {
        // complain
        return 0;
    }
    // if the capsule is not valid
    if (!PyCapsule_IsValid(capsule, tile_capsule)) {
        // set the error string
        PyErr_SetString(PyExc_TypeError, "invalid tile capsule");
        // and complain
        return 0;
    }

    // grab the tile
    auto & tile =
        * static_cast<isce::srtm::tile_t *>(PyCapsule_GetPointer(capsule, tile_capsule));

    // make an index
    isce::srtm::tile_t::index_type index {i, j};

    // get the value
    auto value = tile[index];

    // make a channel
    pyre::journal::debug_t channel("isce.srtm");
    // show me what's about to happen
    channel
        << pyre::journal::at(__HERE__)
        << "tile@(" << i << "," << j << ") <- "
        << static_cast<size_t>(value)
        << pyre::journal::endl;

    // dress it up and return it
    return PyLong_FromLong(value);
}


// helpers
void
freeTile(PyObject * capsule)
{
    // get the capsule signature
    const char * capsule_sig = isce::extension::srtm::tile_capsule;
    // bail out if the capsule is not valid
    if (!PyCapsule_IsValid(capsule, capsule_sig)) return;
    // grab the tile
    isce::srtm::tile_t * tile =
        static_cast<isce::srtm::tile_t *>(PyCapsule_GetPointer(capsule, capsule_sig));
    // deallocate
    delete tile;
    // and return
    return;
}


// end of file
