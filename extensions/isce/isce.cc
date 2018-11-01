// -*- C++ -*-
//
// michael a.g. aïvázis <michael.aivazis@para-sim.com>
// (c) 2003-2018 all rights reserved
//

// portability
#include <portinfo>
//
#include <Python.h>

// the module method declarations
// administrivia
#include "exceptions.h"
#include "metadata.h"
// srtm
#include "srtm.h"


// put everything in my private namespace
namespace isce {
    namespace extension {
        // the module method table
        PyMethodDef module_methods[] = {
            // administrivia
            // the copyright method
            { copyright__name__, copyright, METH_VARARGS, copyright__doc__ },
            // the version
            { version__name__, version, METH_VARARGS, version__doc__ },

            // srtm methods
            {
                // availability map constructor
                srtm::availabilityMap__name__,
                srtm::availabilityMap,
                METH_VARARGS,
                srtm::availabilityMap__doc__
            },
            {
                // availability map read access
                srtm::availabilityMapGet__name__,
                srtm::availabilityMapGet,
                METH_VARARGS,
                srtm::availabilityMapGet__doc__
            },
            {
                // availability map write access
                srtm::availabilityMapSet__name__,
                srtm::availabilityMapSet,
                METH_VARARGS,
                srtm::availabilityMapSet__doc__
            },
            {
                // availability map status summary
                srtm::availabilityMapSummary__name__,
                srtm::availabilityMapSummary,
                METH_VARARGS,
                srtm::availabilityMapSummary__doc__
            },

            {
                // tile constructor
                srtm::tile__name__,
                srtm::tile,
                METH_VARARGS,
                srtm::tile__doc__
            },
            {
                // tile shape
                srtm::tileShape__name__,
                srtm::tileShape,
                METH_VARARGS,
                srtm::tileShape__doc__
            },
            {
                // tile read access
                srtm::tileGet__name__,
                srtm::tileGet,
                METH_VARARGS,
                srtm::tileGet__doc__
            },
            // sentinel
            { 0, 0, 0, 0 }
        };

        // the module definition
        PyModuleDef module_definition = {
            // header
            PyModuleDef_HEAD_INIT,
            // the name of the module
            "isce",
            // the module documentation string
            "isce extension module",
            // size of the per-interpreter state of the module; -1 if this state is global
            -1,
            // the methods defined in this module
            module_methods
        };

    } // of namespace extension
} // of namespace isce


// initialization function for the module
// *must* be called PyInit_isce
PyMODINIT_FUNC
PyInit_isce()
{
    // create the module
    PyObject * module = PyModule_Create(&isce::extension::module_definition);
    // check whether module creation succeeded
    if (!module) {
        // and raise an exception if not
        return module;
    }
    // otherwise, we have an initialized module
    isce::extension::registerExceptionHierarchy(module);

    // and return the newly created module
    return module;
}

// end of file
