// -*- C++ -*-
// -*- coding: utf-8 -*-
//
// Author: Bryan Riel
// Copyright 2017-2018
//

#include "DEMInterpolator.h"

// Load DEM subset into memory
void isce::geometry::DEMInterpolator::
loadDEM(isce::core::Raster & demRaster,
        double minX, double maxX, double minY, double maxY,
        isce::core::dataInterpMethod interpMethod) {

    // Initialize journal
    pyre::journal::warning_t warning("isce.core.Geometry");

    // Get original GeoTransform using raster
    double geoTransform[6];
    demRaster.getGeoTransform(geoTransform);
    const double firstY = geoTransform[3];
    const double firstX = geoTransform[0];
    const double deltaY = geoTransform[5];
    const double deltaX = geoTransform[1];
    const double lastY = firstY + (demRaster.length() - 2) * deltaY;
    const double lastX = firstX + (demRaster.width() - 2) * deltaX;

    // Validate requested geographic bounds with input DEM raster
    if (minX < firstX) {
        warning << "West limit may be insufficient for global height range"
                << pyre::journal::endl;
        minX = firstX;
    }
    if (maxX > lastX) {
        warning << "East limit may be insufficient for global height range"
                << pyre::journal::endl;
        maxX = lastX;
    }
    if (minY < lastY) {
        warning << "South limit may be insufficient for global height range"
                << pyre::journal::endl;
        minY = lastY;
    }
    if (maxY > firstY) {
        warning << "North limit may be insufficient for global height range"
                << pyre::journal::endl;
        maxY = firstY;
    }

    // Compute pixel coordinates for geographic bounds
    int xstart = int((minX - firstX) / deltaX);
    int xend = int((maxX - firstX) / deltaX + 0.5);
    int ystart = int((maxY - firstY) / deltaY);
    int yend = int((minY - firstY) / deltaY - 0.5);
    
    // Store actual starting lat/lon for raster subset
    _xstart = firstX + xstart * deltaX;
    _ystart = firstY + ystart * deltaY;
    _deltax = deltaX;
    _deltay = deltaY;

    // Create memory to feed into spline interpolator
    _width = xend - xstart;
    _length = yend - ystart;

    // Read in the DEM
    _dem.resize(_length * _width);
    demRaster.getBlock(&_dem[0], xstart, ystart, _width, _length);

    // Indicate we have loaded a valid raster
    _haveRaster = true;
    // Store interpolation method
    _interpMethod = interpMethod;

    // If nearest neighbor interpolation requested, we skip spline initialization
    if (_interpMethod == isce::core::NEAREST_METHOD) {
        return;
    }

    // Fill in XY coordinates
    std::valarray<double> Y(_length), X(_width);
    for (int i = 0; i < _length; ++i)
        Y[i] = i;
    for (int j = 0; j < _width; ++j)
        X[j] = j;

    // Choose correct spline degree based on interpolation method
    int splineDeg;
    if (interpMethod == isce::core::BILINEAR_METHOD) {
        splineDeg = 1;
    } else if (_interpMethod == isce::core::BICUBIC_METHOD) {
        splineDeg = 3;
    } else if (_interpMethod == isce::core::BIQUINTIC_METHOD) { 
        splineDeg = 5;
    } else {
        splineDeg = 3;      // Fall back to bicubic
    }

    // Initialize spline
    _spline.initialize(Y, X, _dem, splineDeg, splineDeg, 0.0);
}

// Load DEM subset into memory
void isce::geometry::DEMInterpolator::
declare() const {
    pyre::journal::info_t info("isce.core.DEMInterpolator");
    info << pyre::journal::newline
         << "Actual DEM bounds used:" << pyre::journal::newline
         << "Top Left: " << _xstart << " " << _ystart << pyre::journal::newline
         << "Bottom Right: " << _xstart + _deltax * (_width - 1) << " "
         << _ystart + _deltay * (_length - 1) << " " << pyre::journal::newline
         << "Spacing: " << _deltax << " " << _deltay << pyre::journal::newline
         << "Dimensions: " << _width << " " << _length << pyre::journal::newline;
}

// Compute maximum DEM height
void isce::geometry::DEMInterpolator::
computeHeightStats(float & maxValue, float & meanValue, pyre::journal::info_t & info) const {
    // Announce myself
    info << "Computing DEM statistics" << pyre::journal::newline << pyre::journal::newline;
    // If we don't have a DEM, just use reference height
    if (!_haveRaster) {
        maxValue = _refHeight;
        meanValue = _refHeight;
    } else {
        maxValue = -10000.0;
        float sum = 0.0;
        for (int i = 0; i < _length; ++i) {
            for (int j = 0; j < _width; ++j) {
                float value = _dem[i*_width + j];
                if (value > maxValue)
                    maxValue = value;
                sum += value;
            }
        }
        meanValue = sum / (_length * _width);
    }
    // Announce results
    info << "Max DEM height: " << maxValue << pyre::journal::newline
         << "Average DEM height: " << meanValue << pyre::journal::newline;
}

// Interpolate DEM
double isce::geometry::DEMInterpolator::
interpolate(double x, double y) {

    // If we don't have a DEM, just return reference height
    if (!_haveRaster) {
        return _refHeight;
    } 

    // Compute the row and column for requested lat and lon
    const double row = (y - _ystart) / _deltay;
    const double col = (x - _xstart) / _deltax;

    // Check validity of interpolation coordinates
    const int irow = int(std::floor(row));
    const int icol = int(std::floor(col));
    // If outside bounds, return reference height
    if (irow < 2 || irow >= (_length - 1))
        return _refHeight;
    if (icol < 2 || icol >= (_width - 1))
        return _refHeight;

    // If nearest neighbor interpolation, get the value directly
    double value;
    if (_interpMethod == isce::core::NEAREST_METHOD) {
        const int index = int(std::round(row)) * _width + int(std::round(col));
        value = _dem[index];
    // Otherwise, evaluate spline
    } else {
        value = _spline.eval(row, col);
    }

    // Done
    return value;
}

// end of file
