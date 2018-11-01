// -*- C++ -*-
//
// michael a.g. aïvázis <michael.aivazis@para-sim.com>
// (c) 2003-2018 all rights reserved
//

#if !defined(isce_extension_exceptions_h)
#define isce_extension_exceptions_h


// place everything in my private namespace
namespace isce {
    namespace extension {

        // exception registration
        PyObject * registerExceptionHierarchy(PyObject *);

    } // of namespace extension
} // of namespace isce

#endif

// end of file
