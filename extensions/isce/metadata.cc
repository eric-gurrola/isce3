// -*- C++ -*-
//
// michael a.g. aïvázis <michael.aivazis@para-sim.com>
// (c) 2003-2018 all rights reserved
//

// configuration
#include <portinfo>

// externals
#include <Python.h>
#include <isce/version.h>

// my declarations
#include "metadata.h"

// copyright
const char * const
isce::extension::
copyright__name__ = "copyright";

const char * const
isce::extension::
copyright__doc__ = "the project copyright string";

PyObject *
isce::extension::
copyright(PyObject *, PyObject *)
{
    // get the copyright note
    const char * const copyright_note = "isce: (c) 2003-2018 california institute of technology";
    // convert it into a python string and return it
    return Py_BuildValue("s", copyright_note);
}


// version
const char * const
isce::extension::
version__name__ = "version";

const char * const
isce::extension::
version__doc__ = "the project version string";

PyObject *
isce::extension::
version(PyObject *, PyObject *)
{
    // get the version info
    isce::version_t version { isce::version() };
    // make a tuple
    PyObject * result = PyTuple_New(3);
    // add the three fields
    PyTuple_SET_ITEM(result, 0, PyLong_FromLong(version[0]));
    PyTuple_SET_ITEM(result, 1, PyLong_FromLong(version[1]));
    PyTuple_SET_ITEM(result, 2, PyLong_FromLong(version[2]));
    // hand back to the caller
    return result;
}


// end of file
