// -*- C++ -*-
//
// michael a.g. aïvázis <michael.aivazis@para-sim.com>
// (c) 2003-2018 all rights reserved
//

// code guard
#if !defined(isce_version_h)
#define isce_iversion_h

// support
#include <array>

// my declarations
namespace isce {
    // my version is an array of three integers
    typedef std::array<int, 3> version_t;

    // access to the version number of the {isce} library
    version_t version();
}

#endif

// end of file
