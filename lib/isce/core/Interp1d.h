//
// Author: Brian Hawkins
// Copyright 2019
//

#ifndef ISCE_CORE_INTERP1D_H
#define ISCE_CORE_INTERP1D_H
#pragma once

#include <valarray>

#include "forward.h"

// Declaration
namespace isce {
    namespace core {
        template<typename TK, typename TD>
        TD interp1d(const isce::core::Kernel<TK> &kernel,
                    const std::valarray<TD> &x, double t,
                    bool periodic = false);

        template<typename TK, typename TD>
        TD interp1d(const isce::core::Kernel<TK> &kernel, const TD *x,
                             size_t length, size_t stride, double t,
                             bool periodic = false);
    }
}

/** Interpolate sequence x at point t
 *
 * @param[in] kernel    Kernel function to use for interpolation.
 * @param[in] x         Sequence to interpolate.
 * @param[in] length    Length of sequence.
 * @param[in] stride    Stride between elements of sequence.
 * @param[in] t         Desired time sample (0 <= t <= x.size()-1).
 * @param[in] periodic  Use periodic boundary condition.  Default = false.
 * @returns Interpolated value or 0 if kernel would run off array.
 *
 * Sequence x will be addressed as x[i*stride] for 0 <= i < length.
 * Template parameter TK is kernel element type, TD is data element type.
 */
template<typename TK, typename TD>
TD
isce::core::interp1d(const isce::core::Kernel<TK> &kernel, const TD *x,
                     size_t length, size_t stride, double t, bool periodic);

/** Interpolate sequence x at point t
 *
 * @param[in] kernel Kernel function to use for interpolation.
 * @param[in] x      Sequence to interpolate.
 * @param[in] t      Desired time sample (0 <= t <= x.size()-1).
 * @param[in] periodic  Use periodic boundary condition.  Default = false.
 * @returns Interpolated value or 0 if kernel would run off array.
 *
 * Template parameter TK is kernel element type, TD is data element type.
 */
template<typename TK, typename TD>
TD
isce::core::interp1d(const isce::core::Kernel<TK> &kernel,
                     const std::valarray<TD> &x, double t, bool periodic);

// Get inline implementations
#define ISCE_CORE_INTERP1D_ICC
#include "Interp1d.icc"
#undef ISCE_CORE_INTERP1D_ICC

#endif
