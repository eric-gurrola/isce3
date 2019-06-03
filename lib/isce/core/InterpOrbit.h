#pragma once
#ifndef ISCE_CORE_INTERPORBIT_H
#define ISCE_CORE_INTERPORBIT_H

#include "Common.h"
#include "Vector.h"

namespace isce { namespace core {

enum class OrbitInterpMethod : char {
    Hermite,
    Legendre,
    SCH
};

/**
 * Interpolate orbit position and/or velocity at the specified timepoint
 * using the designated interpolation method.
 *
 * @param[in] method interpolation method
 * @param[in] orbit orbit
 * @param[in] t interpolation time
 * @param[out] position interpolated position
 * @param[out] velocity interpolated velocity
 */
template<class Orbit>
CUDA_HOSTDEV
void interpolate(
        OrbitInterpMethod method,
        const Orbit & orbit,
        double t,
        Vec3 * position,
        Vec3 * velocity = nullptr);

/**
 * Interpolate orbit position and/or velocity at the specified timepoint
 * using third-order Hermite polynomial interpolation.
 *
 * @param[in] orbit orbit
 * @param[in] t interpolation time
 * @param[out] position interpolated position
 * @param[out] velocity interpolated velocity
 */
template<class Orbit>
CUDA_HOSTDEV
void hermite_interpolate(
        const Orbit & orbit,
        double t,
        Vec3 * position,
        Vec3 * velocity = nullptr);

/**
 * Interpolate orbit position and/or velocity at the specified timepoint
 * using eighth-order Legendre polynomial interpolation.
 *
 * @param[in] orbit orbit
 * @param[in] t interpolation time
 * @param[out] position interpolated position
 * @param[out] velocity interpolated velocity
 */
template<class Orbit>
CUDA_HOSTDEV
void legendre_interpolate(
        const Orbit & orbit,
        double t,
        Vec3 * position,
        Vec3 * velocity = nullptr);

/**
 * Interpolate orbit position and/or velocity at the specified timepoint
 * using SCH interpolation method.
 *
 * @param[in] orbit orbit
 * @param[in] t interpolation time
 * @param[out] position interpolated position
 * @param[out] velocity interpolated velocity
 */
template<class Orbit>
CUDA_HOSTDEV
void sch_interpolate(
        const Orbit & orbit,
        double t,
        Vec3 * position,
        Vec3 * velocity = nullptr);

}}

#define ISCE_CORE_INTERPORBIT_ICC
#include "InterpOrbit.icc"
#undef ISCE_CORE_INTERPORBIT_ICC

#endif

