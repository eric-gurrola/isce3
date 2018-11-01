// -*- C++ -*-
//
// michael a.g. aïvázis <michael.aivazis@para-sim.com>
// (c) 2003-2018 all rights reserved
//

#if !defined(isce_extension_metadata_h)
#define isce_extension_metadata_h


// place everything in my private namespace
namespace isce {
    namespace extension {

        // copyright note
        extern const char * const copyright__name__;
        extern const char * const copyright__doc__;
        PyObject * copyright(PyObject *, PyObject *);

        // version
        extern const char * const version__name__;
        extern const char * const version__doc__;
        PyObject * version(PyObject *, PyObject *);

    } // of namespace extension`
} // of namespace isce

#endif

// end of file
