// -*- C++ -*-
//
// michael a.g. aïvázis
// orthologue
// (c) 1998-2019 all rights reserved
//

// for the build system
#include <portinfo>
// external dependencies
#include <Python.h>
#include <stdio>

// put everything in my private namespace
namespace isce {

    namespace extension {

    // the module method table
    PyMethodDef module_methods[] = {
        // module metadata
        // the copyright method
        {copyright__name__, copyright, METH_VARARGS, copyright__doc__ },
        // the license
        // {license__name__, license, METH_VARARGS, license__doc__ },
        // the version
        // {version__name__, version, METH_VARARGS, version__doc__ },

        // Ellipsoid
        {"Ellipsoid2", Ellipsoid, METH_VARARGS, "An ellipsoid with given a, e2"},

        // sentinel
        {0, 0, 0, 0}
    };

    // the module documentation string
    const char * const __doc__ = "isce3 extension module";

    // the module definition structure
    PyModuleDef module_definition = {
        // header
        PyModuleDef_HEAD_INIT,
        // the name of the module
        "isce3",
        // the module documentation string
        __doc__,
        // size of the per-interpreter state of the module; -1 if this state is global
        -1,
        // the methods defined in this module
        module_methods
    };
    } //of namespace extension
} // of namespace isce3

// my error handler
static void errorHandler(const char * reason, const char * file, int line, int isce3_errno) {
    // for now, just print the reason for the error
    std::cerr << " ** ** ** ISCE3 extension error: " << reason << std::endl;
}

// initialization function for the module
// *must* be called PyInit_isce3
PyMODINIT_FUNC
PyInit_isce3()
{
    // create the module
    PyObject * module = PyModule_Create(&isce3::module_definition);
    // check whether module creation succeeded and raise an exception if not
    if (!module) {
        std::cout << "Module creation successful" << std::endl;
        return 0;
    } else {
        std::cout << "Module creation failed" << std::endl;
    }

    // return the newly created module
    return module;
  
}

// end of file
