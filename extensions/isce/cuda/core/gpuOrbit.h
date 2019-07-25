//
// Author: Joshua Cohen
// Copyright 2017
//
// NOTE: This class is the most complicated in the CUDA-specific subset of isceLib because we need 
//       to carefully manage the deep-copying in the constructors (so we don't have to worry about 
//       adding it to every code that uses this class)

#ifndef __ISCE_CUDA_CORE_GPUORBIT_H__
#define __ISCE_CUDA_CORE_GPUORBIT_H__

#include <vector>
#include "isce/core/Orbit.h"

using isce::core::Orbit;
using isce::core::cartesian_t;

namespace isce { 
    //! The isce::cuda namespace
    namespace cuda { 
        namespace core {
    struct gpuOrbit {
        int nVectors;
        double *UTCtime;
        double *position;
        double *velocity;
        // True if copy-constructed from Orbit (on host), false if copy-constructed from gpuOrbit 
        // (on device)
        bool owner;

        CUDA_HOSTDEV gpuOrbit() = delete;
        // Shallow-copy copy constructor only allowed on device, not host, but not allowed to free 
        // own memory (host copy of gpuOrbit is only one allowed)
        CUDA_DEV gpuOrbit(const gpuOrbit &o) : nVectors(o.nVectors), UTCtime(o.UTCtime), 
                                                 position(o.position), velocity(o.velocity), 
                                                 owner(false) {}
        // Advanced "copy constructor only allowed on host (manages deep copies from host to device)
        CUDA_HOST gpuOrbit(const Orbit&);
        CUDA_HOSTDEV gpuOrbit& operator=(const gpuOrbit&) = delete;
        ~gpuOrbit();

        CUDA_DEV inline void getStateVector(int,double&,double*,double*) const;
        CUDA_DEV int interpolateWGS84Orbit(double,double*,double*) const;
        CUDA_DEV int interpolateLegendreOrbit(double,double*,double*) const;
        CUDA_DEV int interpolateSCHOrbit(double,double*,double*) const;
        CUDA_DEV int computeAcceleration(double,double*) const;

        // Host functions to test underlying device functions in a single-threaded context
        CUDA_HOST int interpolateWGS84Orbit_h(double,cartesian_t&,cartesian_t&);
        CUDA_HOST int interpolateLegendreOrbit_h(double,cartesian_t&,cartesian_t&);
        CUDA_HOST int interpolateSCHOrbit_h(double,cartesian_t&,cartesian_t&);
    };

    CUDA_DEV inline void gpuOrbit::getStateVector(
        int idx, double &t, double *pos, double *vel
        ) const {
        // Note we can't really do much in the way of bounds-checking since we can't use the 
        // <stdexcept> library, this is best we have
        bool valid = !((idx < 0) || (idx >= nVectors));
        t = (valid ? UTCtime[idx] : 0.);
        for (int i=0; i<3; i++) {
            pos[i] = (valid ? position[3*idx+i] : 0.);
            vel[i] = (valid ? velocity[3*idx+i] : 0.);
        }
    }
}}}

#endif
