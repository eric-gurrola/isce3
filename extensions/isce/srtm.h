// -*- C++ -*-
//
// michael a.g. aïvázis <michael.aivazis@para-sim.com>
// (c) 2003-2018 all rights reserved
//

#if !defined(isce_extension_srtm_h)
#define isce_extension_srtm_h


// place everything in my private namespace
namespace isce {
    namespace extension {

        // protect srtm methods
        namespace srtm {
            // srtm availability map: the capsule name
            extern const char * const availabilityMap_capsule;

            // srtm availability map: constructor
            extern const char * const availabilityMap__name__;
            extern const char * const availabilityMap__doc__;
            PyObject * availabilityMap(PyObject *, PyObject *);

            // srtm availability map: summary
            extern const char * const availabilityMapSummary__name__;
            extern const char * const availabilityMapSummary__doc__;
            PyObject * availabilityMapSummary(PyObject *, PyObject *);

            // srtm availability map: read acces
            extern const char * const availabilityMapGet__name__;
            extern const char * const availabilityMapGet__doc__;
            PyObject * availabilityMapGet(PyObject *, PyObject *);

            // srtm availability map: write acces
            extern const char * const availabilityMapSet__name__;
            extern const char * const availabilityMapSet__doc__;
            PyObject * availabilityMapSet(PyObject *, PyObject *);

            // srtm tiles: the capsule name
            extern const char * const tile_capsule;

            // srtm tiles: constructor
            extern const char * const tile__name__;
            extern const char * const tile__doc__;
            PyObject * tile(PyObject *, PyObject *);

            // srtm tiles: shape
            extern const char * const tileShape__name__;
            extern const char * const tileShape__doc__;
            PyObject * tileShape(PyObject *, PyObject *);

            // srtm tiles: read access
            extern const char * const tileGet__name__;
            extern const char * const tileGet__doc__;
            PyObject * tileGet(PyObject *, PyObject *);

        } // of namespace srtm

    } // of namespace extension`
} // of namespace isce


#endif

// end of file
