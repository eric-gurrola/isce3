//
// Author: Bryan V. Riel
// Copyright 2017-2018
//

#ifndef ISCE_CUDA_CORE_GPUBASIS_H
#define ISCE_CUDA_CORE_GPUBASIS_H

#ifdef __CUDACC__
#define CUDA_HOSTDEV __host__ __device__
#define CUDA_DEV __device__
#define CUDA_HOST __host__
#else
#define CUDA_HOSTDEV
#define CUDA_DEV
#define CUDA_HOST
#endif

// isce::core
#include "isce/core/Basis.h"

// isce::cuda::core
#include "isce/cuda/core/gpuLinAlg.h"

// Declaration
namespace isce {
    namespace cuda {
        namespace core {
            class gpuBasis;
        }
    }
}

/** Simple class to store three-dimensional basis vectors. */
class isce::cuda::core::gpuBasis {

    public:
        /** Default constructor*/
        CUDA_HOSTDEV inline gpuBasis() {
            for (int i = 0; i < 3; ++i) {
                x0[i] = 0.0;
                x1[i] = 0.0;
                x2[i] = 0.0;
            }
        }

        /** Constructor with basis vectors. */
        CUDA_HOSTDEV inline gpuBasis(double * x0, double * x1, double * x2) {
            for (int i = 0; i < 3; ++i) {
                this->x0[i] = x0[i];
                this->x1[i] = x1[i];
                this->x2[i] = x2[i];
            }
        }

        /** Constructor from CPU Basis. */
        CUDA_HOST inline gpuBasis(const isce::core::Basis & basis) {
            for (int i = 0; i < 3; ++i) {
                x0[i] = basis.x0()[i];
                x1[i] = basis.x1()[i];
                x2[i] = basis.x2()[i];
            }
        }

        /** \brief Project a given vector onto basis
         *
         * @param[in] vec 3D vector to project
         * @param[out] res 3D vector output 
         *
         * \f[
         *      res_i = (x_i \cdot vec)
         *  \f] */
        CUDA_DEV inline void project(double * vec, double * res) {
            res[0] = isce::cuda::core::gpuLinAlg::dot(x0, vec);
            res[1] = isce::cuda::core::gpuLinAlg::dot(x1, vec);
            res[2] = isce::cuda::core::gpuLinAlg::dot(x2, vec);
        };

        /** \brief Combine the basis with given weights
         *
         * @param[in] vec 3D vector to use as weights
         * @param[out] res 3D vector output
         *
         * \f[ 
         *      res = \sum_{i=0}^2 vec[i] \cdot x_i
         *  \f]*/
        CUDA_DEV inline void combine(double * vec, double * res) {
            for (int ii = 0; ii < 3; ++ii) {
                res[ii] = vec[0] * x0[ii] + vec[1] * x1[ii] + vec[2] * x2[ii];
            }
        };

        // Make data public to simplify access and to reduce use of temp variables
        // in device code
        double x0[3];
        double x1[3];
        double x2[3];
};

#endif

// end of file
