// -*- C++ -*-
// -*- coding: utf-8 -*-
//
// eric m. gurrola <eric.m.gurrola@jpl.nasa.gov>
// jet propulsion laboratory/caltech
// (c) 2003-2018 all rights reserved
//
//

// code guard
#if !defined(isce_version_h)
#define isce_iversion_h

// support
#include <tuple>
#include <string>

// my declarations
namespace isce {
    // my version is an array of two integers and the git hash
    using version_t = std::tuple<int, int, std::string>;

    // access to the version number of the {isce} library
    version_t version();
}

#endif

// end of file
