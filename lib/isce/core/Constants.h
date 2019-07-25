//
// Author: Joshua Cohen, Bryan Riel
// Copyright 2017-2018
//

#ifndef ISCE_CORE_CONSTANTS_H
#define ISCE_CORE_CONSTANTS_H
#pragma once

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <complex>
#include <cstdint>
#include <gdal.h>
#include <gdal_priv.h>

#include "DenseMatrix.h"

// Macro wrapper to check vector lengths (adds calling function and variable name information to the
// exception)
#define checkVecLen(v,l) isce::core::checkVecLenDebug(v,l,#v,__PRETTY_FUNCTION__)
#define check2dVecLen(v,l,w) isce::core::check2dVecLenDebug(v,l,w,#v,__PRETTY_FUNCTION__)
// Macro wrapper to provide 2D indexing to a 1D array
#define IDX1D(i,j,w) (((i)*(w))+(j))

namespace isce { namespace core {

    /**Enumeration type to indicate coordinate system of orbits*/
    enum orbitType {
        WGS84_ORBIT,
        SCH_ORBIT
    };

    /**Enumeration type to indicate method to use for orbit interpolation*/
    enum orbitInterpMethod {
        HERMITE_METHOD,
        SCH_METHOD,
        LEGENDRE_METHOD
    };

    /**Enumeration type to indicate interpolation method*/
    enum dataInterpMethod {
        SINC_METHOD,
        BILINEAR_METHOD,
        BICUBIC_METHOD,
        NEAREST_METHOD,
        AKIMA_METHOD,
        BIQUINTIC_METHOD
    };

    /** Default sinc parameters */
    const int SINC_HALF = 4;
    const int SINC_LEN = 8;
    const int SINC_ONE = 9;
    const int SINC_SUB = 8192;
    
    /** Semi-major axis for WGS84 */
    const double EarthSemiMajorAxis = 6378137.0;

    /** Eccentricity^2 for WGS84 */
    const double EarthEccentricitySquared = 0.0066943799901;

    /** Speed of light */
    const double SPEED_OF_LIGHT = 299792458.0;

    /** Struct with fixed-length string for serialization */
    struct FixedString {
        char str[50];
    };

    /** Layover and shadow values */
    const short SHADOW_VALUE = 1;
    const short LAYOVER_VALUE = 2;

    /** Precision-promotion to double/complex\<double\>  **/
    template<typename T> struct double_promote;

    /** Template specialization for float */
    template<> struct double_promote<float>  { using type = double; };

    /** Template specialization for double */
    template<> struct double_promote<double> { using type = double; };

    /** Template specialization for complex\<float\> */
    template<> struct double_promote<std::complex<float>>  { using type = std::complex<double>; };

    /** Template specialization for complex\<double\> */
    template<> struct double_promote<std::complex<double>> { using type = std::complex<double>; };
  }
}

#endif
