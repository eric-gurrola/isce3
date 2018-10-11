//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Joshua Cohen, Bryan V. Riel
// Copyright 2017-2018
//

#include <algorithm>
#include <iostream>
#include <chrono>
#include <cmath>

// pyre
#include <portinfo>
#include <pyre/journal.h>

// isce::core
#include "isce/core/Constants.h"

// isce::image
#include "ResampSlc.h"

using isce::io::Raster;

// Main product-based resamp entry point
void isce::image::ResampSlc::
resamp(const std::string & outputFilename,
       const std::string & polarization,
       const std::string & rgOffsetFilename,
       const std::string & azOffsetFilename,
       bool flatten, bool isComplex, int rowBuffer) {

    // Form the GDAL-compatible path for the HDF5 dataset
    const std::string dataPath = _mode.dataPath(polarization);
    const std::string h5path = "HDF5:\"" + _filename + "\":/" + dataPath;

    // Call alternative resmap entry point
    resamp(h5path, outputFilename, rgOffsetFilename, azOffsetFilename, 1,
           flatten, isComplex, rowBuffer);
}

// Alternative generic resamp entry point
void isce::image::ResampSlc::
resamp(const std::string & inputFilename,          // filename of input SLC
       const std::string & outputFilename,         // filename of output resampled SLC
       const std::string & rgOffsetFilename,       // filename of range offsets
       const std::string & azOffsetFilename,       // filename of azimuth offsets
       int inputBand, bool flatten, bool isComplex, int rowBuffer) {

    // Initialize journal channel for info
    pyre::journal::info_t infoChannel("isce.core.ResampSlc");

    // Check if data are not complex
    if (!isComplex) {
        pyre::journal::error_t errorChannel("isce.core.ResampSlc");
        errorChannel
            << pyre::journal::at(__HERE__)
            << "Real data interpolation not implemented yet."
            << pyre::journal::newline
            << pyre::journal::endl;
        return;
    }
        
    // Make input rasters
    Raster inputSlc(inputFilename, GA_ReadOnly);
    Raster rgOffsetRaster(rgOffsetFilename, GA_ReadOnly);
    Raster azOffsetRaster(azOffsetFilename, GA_ReadOnly);
    // Set the band number for input SLC
    _inputBand = inputBand;
    // Cache width of SLC image
    const int inLength = inputSlc.length();
    const int inWidth = inputSlc.width();
    // Cache output length and width from offset images
    const int outLength = rgOffsetRaster.length();
    const int outWidth = rgOffsetRaster.width();

    // Make output raster
    Raster outputSlc(outputFilename, outWidth, outLength, 1, GDT_CFloat32, "ISCE");

    // Announce myself to the world
    declare(inLength, inWidth, outLength, outWidth);

    // Initialize resampling methods
    _prepareInterpMethods(isce::core::SINC_METHOD);
   
    // Determine number of tiles needed to process image
    const int nTiles = _computeNumberOfTiles(outLength, _linesPerTile);
    infoChannel << 
        "Resampling using " << nTiles << " tiles of " << _linesPerTile 
        << " lines per tile"
        << pyre::journal::newline << pyre::journal::endl;
    
    // Start timer
    auto timerStart = std::chrono::steady_clock::now();

    // For each full tile of _linesPerTile lines...
    for (int tileCount = 0; tileCount < nTiles; tileCount++) {

        // Make a tile for representing input SLC data
        Tile_t tile;
        tile.width(inWidth);
        // Set its line index bounds (line number in output image)
        tile.rowStart(tileCount * _linesPerTile);
        if (tileCount == (nTiles - 1)) {
            tile.rowEnd(outLength);
        } else {
            tile.rowEnd(tile.rowStart() + _linesPerTile);
        }

        // Initialize offsets tiles
        isce::image::Tile<float> azOffTile, rgOffTile;
        _initializeOffsetTiles(tile, azOffsetRaster, rgOffsetRaster,
                               azOffTile, rgOffTile, outWidth);

        // Get corresponding image indices
        infoChannel << "Reading in image data for tile " << tileCount << pyre::journal::newline;
        _initializeTile(tile, inputSlc, azOffTile, outLength, rowBuffer); 
        // Send some diagnostics to the journal
        tile.declare(infoChannel);
    
        // Perform interpolation
        infoChannel << "Interpolating tile " << tileCount << pyre::journal::endl;
        _transformTile(tile, outputSlc, rgOffTile, azOffTile, inLength, flatten);
    }

    // Print out timing information and reset
    auto timerEnd = std::chrono::steady_clock::now();
    const double elapsed = 1.0e-3 * std::chrono::duration_cast<std::chrono::milliseconds>(
        timerEnd - timerStart).count();
    infoChannel << "Elapsed processing time: " << elapsed << " sec"
                << pyre::journal::endl;
}

// Initialize and read azimuth and range offsets
void isce::image::ResampSlc::
_initializeOffsetTiles(Tile_t & tile,
                       Raster & azOffsetRaster,
                       Raster & rgOffsetRaster,
                       isce::image::Tile<float> & azOffTile,
                       isce::image::Tile<float> & rgOffTile,
                       int outWidth) {

    // Copy size properties and initialize azimuth offset tiles
    azOffTile.width(outWidth);
    azOffTile.rowStart(tile.rowStart());
    azOffTile.rowEnd(tile.rowEnd());
    azOffTile.firstImageRow(tile.rowStart());
    azOffTile.lastImageRow(tile.rowEnd());
    azOffTile.allocate();

    // Do the same for range offset tiles
    rgOffTile.width(outWidth);
    rgOffTile.rowStart(tile.rowStart());
    rgOffTile.rowEnd(tile.rowEnd());
    rgOffTile.firstImageRow(tile.rowStart());
    rgOffTile.lastImageRow(tile.rowEnd());
    rgOffTile.allocate();

    // Read in block of range and azimuth offsets 
    azOffsetRaster.getBlock(&azOffTile[0], 0, azOffTile.rowStart(),
                            azOffTile.width(), azOffTile.length());
    rgOffsetRaster.getBlock(&rgOffTile[0], 0, rgOffTile.rowStart(),
                            rgOffTile.width(), rgOffTile.length());
}

// Initialize tile bounds
void isce::image::ResampSlc::
_initializeTile(Tile_t & tile, Raster & inputSlc, const isce::image::Tile<float> & azOffTile,
                int outLength, int rowBuffer) {

    // Cache geometry values
    const int inLength = inputSlc.length();
    const int inWidth = inputSlc.width();
    const int outWidth = azOffTile.width();

    // Compute minimum row index needed from input image
    tile.firstImageRow(outLength - 1);
    bool haveOffsets = false;
    for (int i = 0; i < std::min(rowBuffer, azOffTile.length()); ++i) {
        for (int j = 0; j < outWidth; ++j) {
            // Get azimuth offset for pixel
            const double azOff = azOffTile(i,j);
            // Skip null values
            if (azOff < -5.0e5) {
                continue;
            } else {
                haveOffsets = true;
            }
            // Calculate corresponding minimum line index of input image
            const int imageLine = static_cast<int>(i + azOff + azOffTile.rowStart() - SINC_HALF);
            // Update minimum row index
            tile.firstImageRow(std::min(tile.firstImageRow(), imageLine));
        }
    }
    // Final update
    if (haveOffsets) {
        tile.firstImageRow(std::max(tile.firstImageRow(), 0));
    } else {
        tile.firstImageRow(0);
    }

    // Compute maximum row index needed from input image
    tile.lastImageRow(0);
    haveOffsets = false;
    for (int i = std::max(azOffTile.length() - rowBuffer, 0); i < azOffTile.length(); ++i) {
        for (int j = 0; j < outWidth; ++j) {
            // Get azimuth offset for pixel
            const double azOff = azOffTile(i,j);
            // Skip null values 
            if (azOff < -5.0e5) {
                continue;
            } else {
                haveOffsets = true;
            }
            // Calculate corresponding minimum line index of input image
            const int imageLine = static_cast<int>(i + azOff + azOffTile.rowStart() + SINC_HALF);
            // Update maximum row index
            tile.lastImageRow(std::max(tile.lastImageRow(), imageLine));
        }
    }
    // Final udpate
    if (haveOffsets) {
        tile.lastImageRow(std::min(tile.lastImageRow() + 1, inLength));
    } else {
        tile.lastImageRow(inLength);
    }
    
    // Tile will allocate memory for itself
    tile.allocate();

    // Read in tile.length() lines of data from the input image to the image block
    inputSlc.getBlock(&tile[0], 0, tile.firstImageRow(), tile.width(),
                      tile.length(), _inputBand);

    // Remove carrier from input data
    for (int i = 0; i < tile.length(); i++) {
        for (int j = 0; j < inWidth; j++) {
            // Evaluate the pixel's carrier phase
            const double phase = modulo_f(
                  _rgCarrier.eval(tile.firstImageRow() + i, j) 
                + _azCarrier.eval(tile.firstImageRow() + i, j), 2.0*M_PI);
            // Remove the carrier
            std::complex<float> cpxPhase(std::cos(phase), -std::sin(phase));
            tile(i,j) *= cpxPhase;
        }
    }
}

// Interpolate tile to perform transformation
void isce::image::ResampSlc::
_transformTile(Tile_t & tile,
               Raster & outputSlc,
               const isce::image::Tile<float> & rgOffTile,
               const isce::image::Tile<float> & azOffTile,
               int inLength, bool flatten) {

    // Cache geometry values
    const int inWidth = tile.width();
    const int outWidth = azOffTile.width();
    const int outLength = azOffTile.length();

    // Allocate valarray for working sinc chip
    isce::core::Matrix<std::complex<float>> chip(SINC_ONE, SINC_ONE);

    // Allocate valarray for output image block
    std::valarray<std::complex<float>> imgOut(outLength * outWidth);
    // Initialize to zeros
    imgOut = std::complex<float>(0.0, 0.0);
    
    // Loop over lines to perform interpolation
    int tileLine = 0;
    for (int i = tile.rowStart(); i < tile.rowEnd(); ++i) {

        // Loop over width
        #pragma omp parallel for firstprivate(chip)
        for (int j = 0; j < outWidth; ++j) {

            // Unpack offsets
            const float azOff = azOffTile(tileLine, j);
            const float rgOff = rgOffTile(tileLine, j);

            // Break into fractional and integer parts
            const int intAz = static_cast<int>(i + azOff);
            const int intRg = static_cast<int>(j + rgOff);
            const double fracAz = i + azOff - intAz;
            const double fracRg = j + rgOff - intRg;
           
            // Check bounds
            if ((intAz < SINC_HALF) || (intAz >= (inLength - SINC_HALF)))
                continue;
            if ((intRg < SINC_HALF) || (intRg >= (inWidth - SINC_HALF)))
                continue;

            // Evaluate Doppler polynomial
            const double dop = _dopplerPoly.eval(0, j) * 2*M_PI / _mode.prf();

            // Doppler to be added back. Simultaneously evaluate carrier that needs to
            // be added back after interpolation
            double phase = (dop * fracAz) 
                + _rgCarrier.eval(i + azOff, j + rgOff) 
                + _azCarrier.eval(i + azOff, j + rgOff);

            // Flatten the carrier phase if requested
            if (flatten && _haveRefMode) {
                phase += ((4. * (M_PI / _mode.wavelength())) * 
                    ((_mode.startingRange() - _refMode.startingRange()) 
                    + (j * (_mode.rangePixelSpacing() - _refMode.rangePixelSpacing())) 
                    + (rgOff * _mode.rangePixelSpacing()))) + ((4.0 * M_PI 
                    * (_refMode.startingRange() + (j * _refMode.rangePixelSpacing()))) 
                    * ((1.0 / _refMode.wavelength()) - (1.0 / _mode.wavelength())));
            }
            // Modulate by 2*PI
            phase = modulo_f(phase, 2.0*M_PI);
            
            // Read data chip without the carrier phases
            for (int ii = 0; ii < SINC_ONE; ++ii) {
                // Row to read from
                const int chipRow = intAz - tile.firstImageRow() + ii - SINC_HALF;
                // Carrier phase
                const double phase = dop * (ii - 4.0);
                const std::complex<float> cval(std::cos(phase), -std::sin(phase));
                // Set the data values after removing doppler in azimuth
                for (int jj = 0; jj < SINC_ONE; ++jj) {
                    // Column to read from
                    const int chipCol = intRg + jj - SINC_HALF;
                    chip(ii,jj) = tile(chipRow,chipCol) * cval;
                }
            }

            // Interpolate fractional component
            const std::complex<float> cval = _interpolateComplex(
                chip, (SINC_HALF + 1), (SINC_HALF + 1), fracAz, fracRg, SINC_ONE, SINC_ONE
            );

            // Add doppler to interpolated value and save
            imgOut[tileLine*outWidth + j] = cval * std::complex<float>(
                std::cos(phase), std::sin(phase)
            );

        } // end for over width

        // Update input line counter
        ++tileLine;

    } // end for over length

    // Write block of data
    outputSlc.setBlock(imgOut, 0, tile.rowStart(), outWidth, outLength);
}

// end of file