// -*- C++ -*-
// -*- coding: utf-8 -*-
//
// Author: Bryan Riel
// Copyright 2017-2018
//

#ifndef ISCE_CORE_DEMInterpolator_H
#define ISCE_CORE_DEMInterpolator_H

// pyre
#include <portinfo>
#include <pyre/journal.h>

// isce::core
#include <isce/core/Constants.h>
#include <isce/core/Interpolator.h>
#include <isce/core/Projections.h>

// isce::io
#include <isce/io/Raster.h>

// Declaration
namespace isce {
    namespace geometry {
        class DEMInterpolator;
    }
}

// DEMInterpolator declaration
class isce::geometry::DEMInterpolator {

    public:
        // Constructors
        DEMInterpolator() : _haveRaster(false), _refHeight(0.0) {}
        DEMInterpolator(float height) : _haveRaster(false), _refHeight(height) {}

        // Read in subset of data from a DEM with a supported projection
        void loadDEM(isce::io::Raster &demRaster,
                     double minLon, double maxLon,
                     double minLat, double maxLat,
                     isce::core::dataInterpMethod,
                     int epsgcode=4326);
        // Print stats
        void declare() const;
        // Compute max and mean DEM height
        void computeHeightStats(float &maxValue, float &meanValue,
                    pyre::journal::info_t &info) const;
        // Interpolate at a given longitude and latitude
        double interpolateLonLat(double lon, double lat) const;
        // Interpolate at native XY coordinates of DEM
        double interpolateXY(double x, double y) const;
        // Get transform properties
        double xStart() const { return _xstart; }
        double yStart() const { return _ystart; }
        double deltaX() const { return _deltax; }
        double deltaY() const { return _deltay; }
        // Middle X and Y coordinates
        double midX() const { return _xstart + 0.5*_dem.width()*_deltax; }
        double midY() const { return _ystart + 0.5*_dem.length()*_deltay; }
        // Middle lat/lon/h
        isce::core::cartesian_t midLonLat(double height) const;

    private:
        // Flag indicating whether we have access to a DEM raster
        bool _haveRaster;
        // Constant value if no raster is provided
        float _refHeight;
        // Pointer to a ProjectionBase
        int _epsgcode;
        isce::core::ProjectionBase * _proj;
        // 2D array for storing DEM subset
        isce::core::Matrix<float> _dem;
        // Starting x/y for DEM subset and spacing
        double _xstart, _ystart, _deltax, _deltay;
        // Interpolation method
        isce::core::dataInterpMethod _interpMethod;
};

#endif

// end of file
