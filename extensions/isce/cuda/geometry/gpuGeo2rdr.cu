//-*- coding: utf-8 -*-
//
// Author: Bryan V. Riel, Joshua Cohen
// Copyright: 2017-2018

#include <isce/core/Ellipsoid.h>

// isce::cuda::core
#include "isce/cuda/core/gpuOrbit.h"
#include "isce/cuda/core/gpuLUT1d.h"

// isce::cuda::geometry
#include "gpuGeometry.h"
#include "gpuGeo2rdr.h"

#include <isce/cuda/except/Error.h>

#define THRD_PER_BLOCK 96 // Number of threads per block (should always %32==0)
#define NULL_VALUE -1000000.0

using isce::core::Vec3;

__global__
void runGeo2rdrBlock(isce::core::Ellipsoid ellps,
                     isce::cuda::core::gpuOrbit orbit,
                     isce::cuda::core::gpuLUT1d<double> doppler,
                     double * x, double * y, double * hgt,
                     float * azoff, float * rgoff,
                     isce::cuda::core::ProjectionBase ** projTopo,
                     size_t lineStart, size_t blockLength, size_t blockWidth,
                     double t0, double r0, size_t numberAzimuthLooks, size_t numberRangeLooks,
                     size_t length, size_t width, double prf, double rangePixelSpacing,
                     double wavelength, double threshold, int numiter, unsigned int * totalconv) {

    // Get the flattened index
    size_t index_flat = blockDim.x * blockIdx.x + threadIdx.x;
    const size_t NPIXELS = blockLength * blockWidth;

    // Only process if a valid pixel (while trying to avoid thread divergence)
    if (index_flat < NPIXELS) {

        // Unravel the flattened pixel index
        const size_t line = index_flat / blockWidth;
        const size_t rbin = index_flat - line * blockWidth;

        // Convert topo XYZ to LLH
        const Vec3 xyz {x[index_flat], y[index_flat], hgt[index_flat]};
        Vec3 llh;
        (*projTopo)->inverse(xyz, llh);

        // Perform geo->rdr iterations
        double aztime, slantRange;
        const double deltaRange = 1.0e-8;
        int geostat = isce::cuda::geometry::geo2rdr(
            llh, ellps, orbit, doppler, &aztime, &slantRange, wavelength, threshold,
            numiter, deltaRange);

        // Compute azimuth time extents
        const double dtaz = numberAzimuthLooks / prf;
        const double tend = t0 + ((length - 1) * dtaz);

        // Compute range extents
        const double dmrg = numberRangeLooks * rangePixelSpacing;
        const double rngend = r0 + ((width - 1) * dmrg);

        // Check if solution is out of bounds
        bool isOutside = false;
        if ((aztime < t0) || (aztime > tend))
            isOutside = true;
        if ((slantRange < r0) || (slantRange > rngend))
            isOutside = true;

        // Save result if valid
        if (!isOutside) {
            rgoff[index_flat] = ((slantRange - r0) / dmrg) - float(rbin);
            azoff[index_flat] = ((aztime - t0) / dtaz) - float(line + lineStart);
        } else {
            rgoff[index_flat] = NULL_VALUE;
            azoff[index_flat] = NULL_VALUE;
        }

        // Update convergence count
        atomicAdd(totalconv, (unsigned int) geostat);
    }
}

// C++ Host code for launching kernel to run topo on current block
void isce::cuda::geometry::
runGPUGeo2rdr(const isce::core::Ellipsoid& ellipsoid,
              const isce::core::Orbit & orbit,
              const isce::core::LUT1d<double> & doppler,
              const std::valarray<double> & x,
              const std::valarray<double> & y,
              const std::valarray<double> & hgt,
              std::valarray<float> & azoff,
              std::valarray<float> & rgoff,
              int topoEPSG, size_t lineStart, size_t blockWidth,
              double t0, double r0, size_t numberAzimuthLooks, size_t numberRangeLooks,
              size_t length, size_t width, double prf, double rangePixelSpacing,
              double wavelength, double threshold, double numiter, unsigned int & totalconv) {

    // Create gpu ISCE objects
    isce::cuda::core::gpuOrbit gpu_orbit(orbit);
    isce::cuda::core::gpuLUT1d<double> gpu_doppler(doppler);

    // Allocate memory on device topo data and results
    double *x_d, *y_d, *hgt_d;
    float *azoff_d, *rgoff_d;
    const size_t nbytes_double = x.size() * sizeof(double);
    const size_t nbytes_float = x.size() * sizeof(float);
    checkCudaErrors(cudaMalloc(&x_d, nbytes_double));
    checkCudaErrors(cudaMalloc(&y_d, nbytes_double));
    checkCudaErrors(cudaMalloc(&hgt_d, nbytes_double));
    checkCudaErrors(cudaMalloc(&azoff_d, nbytes_float));
    checkCudaErrors(cudaMalloc(&rgoff_d, nbytes_float));

    // Copy topo data to device
    checkCudaErrors(cudaMemcpy(x_d, &x[0], nbytes_double, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(y_d, &y[0], nbytes_double, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(hgt_d, &hgt[0], nbytes_double, cudaMemcpyHostToDevice));
    
    // Allocate projection pointer on device
    isce::cuda::core::ProjectionBase **proj_d;
    checkCudaErrors(cudaMalloc(&proj_d, sizeof(isce::cuda::core::ProjectionBase **)));
    createProjection<<<1, 1>>>(proj_d, topoEPSG);

    // Allocate integer for storing convergence results
    unsigned int * totalconv_d;
    checkCudaErrors(cudaMalloc(&totalconv_d, sizeof(unsigned int)));
    checkCudaErrors(cudaMemcpy(totalconv_d, &totalconv, sizeof(unsigned int),
                               cudaMemcpyHostToDevice));

    // Determine grid layout
    dim3 block(THRD_PER_BLOCK);
    const size_t npixel = x.size();
    const int nBlocks = (int) std::ceil((1.0 * npixel) / THRD_PER_BLOCK);
    dim3 grid(nBlocks);

    // Launch kernel
    const size_t blockLength = x.size() / blockWidth;
    runGeo2rdrBlock<<<grid, block>>>(ellipsoid, gpu_orbit, gpu_doppler,
                                     x_d, y_d, hgt_d, azoff_d, rgoff_d, proj_d, lineStart,
                                     blockLength, blockWidth, t0, r0, numberAzimuthLooks,
                                     numberRangeLooks, length, width, prf, rangePixelSpacing, 
                                     wavelength, threshold, numiter, totalconv_d);

    // Check for any kernel errors
    checkCudaErrors(cudaPeekAtLastError());

    // Copy geo2rdr results from device to host
    checkCudaErrors(cudaMemcpy(&azoff[0], azoff_d, nbytes_float, cudaMemcpyDeviceToHost));
    checkCudaErrors(cudaMemcpy(&rgoff[0], rgoff_d, nbytes_float, cudaMemcpyDeviceToHost));
    checkCudaErrors(cudaMemcpy(&totalconv, totalconv_d, sizeof(unsigned int),
                               cudaMemcpyDeviceToHost));

    // Delete projection pointer on device
    deleteProjection<<<1, 1>>>(proj_d);

    // Free memory
    checkCudaErrors(cudaFree(x_d));
    checkCudaErrors(cudaFree(y_d));
    checkCudaErrors(cudaFree(hgt_d));
    checkCudaErrors(cudaFree(azoff_d));
    checkCudaErrors(cudaFree(rgoff_d));
    checkCudaErrors(cudaFree(proj_d));
    checkCudaErrors(cudaFree(totalconv_d));
}

// end of file
