//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Joshua Cohen, Bryan V. Riel, Liang Yu
// Copyright 2017-2019
//

#include <algorithm>
#include <iostream>
#include <chrono>
#include <cmath>
#include <thrust/transform_reduce.h>
#include <thrust/advance.h>

// isce::core
#include <isce/core/Constants.h>

// isce::cuda::core
#include <isce/cuda/core/gpuPoly2d.h>
#include <isce/cuda/core/gpuLUT1d.h>
#include <isce/cuda/core/Stream.h>

#include <isce/cuda/io/DataStream.h>

#include "ResampSlc.h"
#include "gpuResampSlc.h"

using isce::io::Raster;

using isce::cuda::core::gpuPoly2d;
using isce::cuda::core::gpuInterpolator;
using isce::cuda::core::gpuLUT1d;
using isce::cuda::core::gpuSinc2dInterpolator;

// thrust::host_vector whose data buffer uses page-locked memory
template<typename T>
using pinned_host_vector = thrust::host_vector<T,
        thrust::system::cuda::experimental::pinned_allocator<T>>;

//using isce::cuda::core::Stream;

//using isce::cuda::io::DataStream;

#define THRD_PER_BLOCK 512// Number of threads per block (should always %32==0)
__global__
void removeCarrier(thrust::complex<float> *tile,
        int firstImageRow,
        int outWidth,
        int outLength,
        const gpuPoly2d rgCarrier,
        const gpuPoly2d azCarrier) {

    int iTile = blockDim.x * blockIdx.x + threadIdx.x;

    if (iTile < outWidth*outLength) {
        double phase = fmod(rgCarrier.eval(blockIdx.x+firstImageRow, threadIdx.x)
                + azCarrier.eval(blockIdx.x+firstImageRow, threadIdx.x), 2.0*M_PI);
        tile[iTile] *= thrust::complex<float>(cos(phase), -sin(phase));
    }
}

__global__
void transformTile(const thrust::complex<float> *tile,
                   thrust::complex<float> *chip,
                   thrust::complex<float> *imgOut,
                   const float *rgOffTile,
                   const float *azOffTile,
                   const gpuPoly2d rgCarrier,
                   const gpuPoly2d azCarrier,
                   const gpuLUT1d<double> dopplerLUT,
                   gpuSinc2dInterpolator<thrust::complex<float>> interp,
                   bool flatten,
                   int outWidth,
                   int outLength,
                   int inWidth,
                   int inLength,
                   double startingRange,
                   double rangePixelSpacing,
                   double prf,
                   double wavelength,
                   double refStartingRange,
                   double refRangePixelSpacing,
                   double refWavelength,
                   int chipSize,
                   int rowOffset, 
                   int rowStart) {

    int iTileOut = blockDim.x * blockIdx.x + threadIdx.x;
    int iChip = iTileOut * chipSize * chipSize;                                          
    int chipHalf = chipSize/2;

    if (iTileOut < outWidth*outLength) {
        int i = iTileOut / outWidth;
        int j = iTileOut % outWidth;
        imgOut[iTileOut] = thrust::complex<float>(0., 0.);

        // Unpack offsets
        const float azOff = azOffTile[iTileOut];
        const float rgOff = rgOffTile[iTileOut];

        // Break into fractional and integer parts
        const int intAz = __float2int_rd(i + azOff + rowStart);
        const int intRg = __float2int_rd(j + rgOff);
        const double fracAz = i + azOff - intAz + rowStart;
        const double fracRg = j + rgOff - intRg;
       
        // Check bounds again. Use rowOffset to account tiles where rowStart != firstRowImage
        bool intAzInBounds = !((intAz+rowOffset < chipHalf) || (intAz >= (inLength - chipHalf)));
        bool intRgInBounds = !((intRg < chipHalf) || (intRg >= (inWidth - chipHalf)));

        if (intAzInBounds && intRgInBounds) {
            // evaluate Doppler polynomial
            const double rng = startingRange + j * rangePixelSpacing;
            const double dop = dopplerLUT.eval(rng) * 2 * M_PI / prf;

            // Doppler to be added back. Simultaneously evaluate carrier that needs to
            // be added back after interpolation
            double phase = (dop * fracAz) 
                + rgCarrier.eval(i + azOff, j + rgOff) 
                + azCarrier.eval(i + azOff, j + rgOff);

            // Flatten the carrier phase if requested
            if (flatten) {
                phase += ((4. * (M_PI / wavelength)) * 
                    ((startingRange - refStartingRange) 
                    + (j * (rangePixelSpacing - refRangePixelSpacing)) 
                    + (rgOff * rangePixelSpacing))) + ((4.0 * M_PI 
                    * (refStartingRange + (j * refRangePixelSpacing))) 
                    * ((1.0 / refWavelength) - (1.0 / wavelength)));
            }
            
            // Modulate by 2*PI
            phase = fmod(phase, 2.0*M_PI);
            
            // Read data chip without the carrier phases
            for (int ii = 0; ii < chipSize; ++ii) {
                // Row to read from
                const int chipRow = intAz + ii - chipHalf + rowOffset - rowStart;
                // Carrier phase
                const double phase = dop * (ii - 4.0);
                const thrust::complex<float> cval(cos(phase), -sin(phase));
                // Set the data values after removing doppler in azimuth
                for (int jj = 0; jj < chipSize; ++jj) {
                    // Column to read from
                    const int chipCol = intRg + jj - chipHalf;
                    chip[iChip + ii*chipSize+jj] = tile[chipRow*outWidth+chipCol] * cval;
                }
            }

            // Interpolate chip
            const thrust::complex<float> cval = interp.interpolate(
                chipHalf + fracRg, chipHalf + fracAz, &chip[iChip], chipSize, chipSize
            );

            // Add doppler to interpolated value and save
            imgOut[iTileOut] = cval * thrust::complex<float>(cos(phase), sin(phase));
        }
    }
}


// Alternative generic resamp entry point: use filenames to internally create rasters
void isce::cuda::image::ResampSlc::
resamp(const std::string & inputFilename,          // filename of input SLC
       const std::string & outputFilename,         // filename of output resampled SLC
       const std::string & rgOffsetFilename,       // filename of range offsets
       const std::string & azOffsetFilename,       // filename of azimuth offsets
       int inputBand, bool flatten, bool isComplex, int rowBuffer,
       int chipSize) {

    // Make input rasters
    Raster inputSlc(inputFilename, GA_ReadOnly);
    Raster rgOffsetRaster(rgOffsetFilename, GA_ReadOnly);
    Raster azOffsetRaster(azOffsetFilename, GA_ReadOnly);

    // Make output raster; geometry defined by offset rasters
    const int outLength = rgOffsetRaster.length();
    const int outWidth = rgOffsetRaster.width();
    Raster outputSlc(outputFilename, outWidth, outLength, 1, GDT_CFloat32, "ISCE");

    // Call generic resamp
    resamp(inputSlc, outputSlc, rgOffsetRaster, azOffsetRaster, inputBand, flatten,
           isComplex, rowBuffer, chipSize);
}

// Generic resamp entry point from externally created rasters
void isce::cuda::image::ResampSlc::
resamp(isce::io::Raster & inputSlc, isce::io::Raster & outputSlc,
       isce::io::Raster & rgOffsetRaster, isce::io::Raster & azOffsetRaster,
       int inputBand, bool flatten, bool isComplex, int rowBuffer,
       int chipSize) {

    // Check if data are not complex
    if (!isComplex) {
        std::cout << "Real data interpolation not implemented yet.\n";
        return;
    }
    // Set the band number for input SLC
    _inputBand = inputBand;
    // Cache width of SLC image
    const int inLength = inputSlc.length();
    const int inWidth = inputSlc.width();
    // Cache output length and width from offset images
    const int outLength = rgOffsetRaster.length();
    const int outWidth = rgOffsetRaster.width();

    // Check if reference data is available
    if (!this->haveRefData()) {
        flatten = false;
    }

    // initialize interpolator
    isce::cuda::core::gpuSinc2dInterpolator<thrust::complex<float>> interp(chipSize-1, isce::core::SINC_SUB);

    // Determine number of tiles needed to process image
    const int nTiles = isce::image::_computeNumberOfTiles(outLength, _linesPerTile);
    std::cout << 
        "GPU resampling using " << nTiles << " tiles of " << _linesPerTile 
        << " lines per tile\n";
    // Start timer
    auto timerStart = std::chrono::steady_clock::now();

    // For each full tile of _linesPerTile lines...
    for (int tileCount = 0; tileCount < nTiles; tileCount++) {

        int rowStart = tileCount * _linesPerTile;
        int rowEnd(rowStart + _linesPerTile);
        if (tileCount == (nTiles - 1)) {
            rowEnd = outLength;
        }
        int outLength = rowEnd - rowStart;
        int nOutPixels = outWidth * outLength;

        // initialize range offsets
        thrust::device_vector<float> d_rgOffsets(nOutPixels);
        pinned_host_vector<float> h_rgOffsets(nOutPixels);
        rgOffsetRaster.getBlock(h_rgOffsets.data(), 0, rowStart, outWidth, outLength);
        isce::cuda::core::Stream streamRgOffset;
        checkCudaErrors( cudaMemcpyAsync(d_rgOffsets.data().get(), h_rgOffsets.data(),
                    nOutPixels*sizeof(float), cudaMemcpyHostToDevice, streamRgOffset.get()) );

        /*
        isce::cuda::core::Stream streamRgOffset;
        isce::cuda::io::RasterDataStream datastreamRgOffset(&rgOffsetRaster, streamRgOffset);
        datastreamRgOffset.load(d_rgOffsets.data().get(), 0, rowStart, outWidth, outLength);
        */

        // initialize azimuth offsets
        thrust::device_vector<float> d_azOffsets(nOutPixels);
        pinned_host_vector<float> h_azOffsets(nOutPixels);
        azOffsetRaster.getBlock(h_azOffsets.data(), 0, rowStart, outWidth, outLength);
        isce::cuda::core::Stream streamAzOffset;
        checkCudaErrors( cudaMemcpyAsync(d_azOffsets.data().get(), h_azOffsets.data(),
                    nOutPixels*sizeof(float), cudaMemcpyHostToDevice, streamRgOffset.get()) );

        /*
        isce::cuda::core::Stream streamAzOffset;
        isce::cuda::io::RasterDataStream datastreamAzOffset(&azOffsetRaster, streamAzOffset);
        datastreamAzOffset.load(d_azOffsets.data().get(), 0, rowStart, outWidth, outLength);
        */

        // prepare SLC
        // Compute minimum row index needed from input image
        int firstImageRow(outLength - 1);
        bool haveOffsets = false;
        int chipHalf = chipSize/2;
        for (int i = 0; i < std::min(rowBuffer, outLength); ++i) {
            for (int j = 0; j < outWidth; ++j) {
                // Get azimuth offset for pixel
                const double azOff = d_azOffsets[i,j];
                // Skip null values
                if (azOff < -5.0e5 || std::isnan(azOff)) {
                    continue;
                } else {
                    haveOffsets = true;
                }
                // Calculate corresponding minimum line index of input image
                const int imageLine = static_cast<int>(i + azOff + rowStart - chipHalf);
                // Update minimum row index
                firstImageRow = std::min(firstImageRow, imageLine);
            }
        }
        if (haveOffsets) {
            firstImageRow = std::max(firstImageRow, 0);
        } else {
            firstImageRow = 0;
        }

        // Compute maximum row index needed from input image
        int lastImageRow(0);
        haveOffsets = false;
        for (int i = std::max(outLength - rowBuffer, 0); i < outLength; ++i) {
            for (int j = 0; j < outWidth; ++j) {
                // Get azimuth offset for pixel
                const double azOff = d_azOffsets[i,j];
                // Skip null values 
                if (azOff < -5.0e5 || std::isnan(azOff)) {
                    continue;
                } else {
                    haveOffsets = true;
                }
                // Calculate corresponding minimum line index of input image
                const int imageLine = static_cast<int>(i + azOff + rowStart + chipHalf);
                // Update maximum row index
                lastImageRow = std::max(lastImageRow, imageLine);
            }
        }
        if (haveOffsets) {
            lastImageRow = std::min(lastImageRow + 1, inLength);
        } else {
            lastImageRow = inLength;
        }

        // Get corresponding image indices
        // replace below with stream init
        std::cout << "Reading in image data for tile " << tileCount << std::endl;
        int nInPixels = inWidth * (lastImageRow-firstImageRow);
        thrust::device_vector<thrust::complex<float>> d_slc(nInPixels);
        pinned_host_vector<thrust::complex<float>> h_slc(nInPixels);
        inputSlc.getBlock(h_slc.data(), 0, firstImageRow, inWidth, lastImageRow-firstImageRow, _inputBand);
        isce::cuda::core::Stream streamSlc;
        checkCudaErrors( cudaMemcpyAsync(d_slc.data().get(), h_slc.data(),
                    nInPixels*sizeof(thrust::complex<float>), cudaMemcpyHostToDevice, streamSlc.get()) );

        /*
        isce::cuda::core::Stream streamSlc;
        isce::cuda::io::RasterDataStream datastreamSlc(&inputSlc, streamSlc);
        inputSlc.getBlock(d_slc.data().get(), 0, firstImageRow, inWidth, lastImageRow-firstImageRow, _inputBand);
        */

        thrust::device_vector<thrust::complex<float>> d_chip(nOutPixels * chipSize * chipSize); // make contiguous 2D
        thrust::device_vector<thrust::complex<float>> d_imgOut(nOutPixels); // tie in with RasterDataStream
        gpuPoly2d d_rgCarrier(_rgCarrier);
        gpuPoly2d d_azCarrier(_azCarrier);
        gpuLUT1d<double> d_dopplerLUT(_dopplerLUT);

        // move phase removal computation into device
        // determine block layout
        dim3 block(THRD_PER_BLOCK);
        dim3 grid((nInPixels+(THRD_PER_BLOCK-1))/THRD_PER_BLOCK);
        removeCarrier<<<grid, block>>>(d_slc.data().get(), firstImageRow, outWidth, outLength, _rgCarrier, _azCarrier);
        // TODO synchronize before resampling

        // Perform interpolation
        // determine block layout
        grid = (nOutPixels+(THRD_PER_BLOCK-1)) / THRD_PER_BLOCK;
        int rowOffset = rowStart-firstImageRow;
        std::cout << "Interpolating tile " << tileCount << std::endl;
        transformTile<<<grid, block>>>(d_slc.data().get(),
                d_chip.data().get(),
                d_imgOut.data().get(),
                d_rgOffsets.data().get(), d_azOffsets.data().get(),
                d_rgCarrier, d_azCarrier, d_dopplerLUT,
                interp, flatten,
                outWidth, outLength,
                inWidth, inLength,
                this->startingRange(), this->rangePixelSpacing(),
                this->prf(), this->wavelength(),
                this->refStartingRange(), this->refRangePixelSpacing(),
                this->refWavelength(), chipSize,
                rowOffset,// needed to keep az in bounds in subtiles
                rowStart); // needed to match az components on CPU

    }

    // Print out timing information and reset
    auto timerEnd = std::chrono::steady_clock::now();
    const double elapsed = 1.0e-3 * std::chrono::duration_cast<std::chrono::milliseconds>(
        timerEnd - timerStart).count();
    std::cout << "Elapsed processing time: " << elapsed << " sec" << "\n";
}

// end of file
