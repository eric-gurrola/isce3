// -*- C++ -*-
//
// michael a.g. aïvázis <michael.aivazis@para-sim.com>
// (c) 2003-2018 all rights reserved
//

#include <portinfo>
#include <Python.h>
#include <string>

#include "exceptions.h"

namespace isce {
    namespace extension {
        // base class for isce errors
        std::string Error__name__ = "Error";
        PyObject * Error = 0;
    } // of namespace extension
} // of namespace isce


// exception registration
PyObject *
isce::extension::
registerExceptionHierarchy(PyObject * module) {

    std::string stem = "isce.";

    // the base class
    // build its name
    std::string errorName = stem + isce::extension::Error__name__;
    // and the exception object
    isce::extension::Error = PyErr_NewException(errorName.c_str(), 0, 0);
    // increment its reference count so we can pass ownership to the module
    Py_INCREF(isce::extension::Error);
    // register it with the module
    PyModule_AddObject(module,
                       isce::extension::Error__name__.c_str(),
                       isce::extension::Error);

    // and return the module
    return module;
}

// end of file
