//
// Author: Joshua Cohen
// Copyright 2017
//
// NOTE: This class is the most complicated in the CUDA-specific subset of isceLib because we need 
//       to carefully manage the deep-copying in the constructors (so we don't have to worry about 
//       adding it to every code that uses this class)

#ifndef __ISCE_CORE_CUDA_GPUORBIT_H__
#define __ISCE_CORE_CUDA_GPUORBIT_H__

#include <vector>
#include "Orbit.h"

namespace isce { namespace core { namespace cuda {
    struct gpuOrbit {
        int nVectors;
        double *UTCtime;
        double *position;
        double *velocity;
        // True if copy-constructed from Orbit (on host), false if copy-constructed from gpuOrbit 
        // (on device)
        bool owner;

        __host__ __device__ gpuOrbit() = delete;
        // Shallow-copy copy constructor only allowed on device, not host, but not allowed to free 
        // own memory (host copy of gpuOrbit is only one allowed)
        __device__ gpuOrbit(const gpuOrbit &o) : nVectors(o.nVectors), UTCtime(o.UTCtime), 
                                                 position(o.position), velocity(o.velocity), 
                                                 owner(false) {}
        // Advanced "copy constructor only allowed on host (manages deep copies from host to device)
        __host__ gpuOrbit(const Orbit&);
        __host__ __device__ gpuOrbit& operator=(const gpuOrbit&) = delete;
        ~gpuOrbit();

        __device__ inline void getStateVector(int,double&,double*,double*);
        __device__ int interpolateWGS84Orbit(double,double*,double*);
        __device__ int interpolateLegendreOrbit(double,double*,double*);
        __device__ int interpolateSCHOrbit(double,double*,double*);
        __device__ int computeAcceleration(double,double*);

        // Host functions to test underlying device functions in a single-threaded context
        __host__ int interpolateWGS84Orbit_h(double,std::vector<double>&,std::vector<double>&);
        __host__ int interpolateLegendreOrbit_h(double,std::vector<double>&,std::vector<double>&);
        __host__ int interpolateSCHOrbit_h(double,std::vector<double>&,std::vector<double>&);
    };

    __device__ inline void gpuOrbit::getStateVector(int idx, double &t, double *pos, double *vel) {
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
