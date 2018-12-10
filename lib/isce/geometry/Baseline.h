//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Bryan Riel, Marco Lavalle, Joshua Cohen
// Copyright 2017-2018

#ifndef ISCE_CORE_BASELINE_H
#define ISCE_CORE_BASELINE_H

// pyre
#include <portinfo>
#include <pyre/journal.h>

// isce::core
#include <isce/core/Orbit.h>
#include <isce/core/Poly2d.h>
#include <isce/core/Ellipsoid.h>
#include <isce/core/Peg.h>
#include <isce/core/Pegtrans.h>
#include <isce/product/ImageMode.h>

// isce::io
#include <isce/io/Raster.h>


// isce::product
#include <isce/product/Product.h>


// Declaration
namespace isce {
    namespace geometry {
        class Baseline;
    }
}

// Baseline declaration
class isce::geometry::Baseline {

    public:
        /** Constructor from product */
        Baseline(isce::product::Product &, isce::product::Product &);

        /** Construct from core object */
        Baseline(isce::core::Ellipsoid,
                        isce::core::Orbit,
                        isce::product::ImageMode,
                        isce::core::Ellipsoid,
                        isce::core::Orbit,
                        isce::product::ImageMode);

        // Set options
        inline void threshold(double);
        inline void numiter(int);
        inline void orbitMethod(isce::core::orbitInterpMethod);


        /** Run geo2rdr with constant offsets and internally created offset rasters */
        void computeBaseline(isce::io::Raster & topoRaster,
                             const std::string & outdir,
                             double azshift=0.0, double rgshift=0.0);


        // Value for null pixels
        const double NULL_VALUE = -1.0e6;

    private:
        // Print extents and image info
        void _printExtents(pyre::journal::info_t &,
                           double, double, double,
                           double, double, double,
                           size_t, size_t);

        // Check we can interpolate orbit to middle of DEM
        void _checkOrbitInterpolation(double);

    private:
        // isce::core objects
        isce::core::Ellipsoid _ellipsoidMaster;
        isce::core::Orbit _orbitMaster;
        isce::core::Poly2d _dopplerMaster;
        isce::core::DateTime _refEpochMaster;
        isce::core::DateTime _sensingStartMaster;

        isce::core::Ellipsoid _ellipsoidSlave;
        isce::core::Orbit _orbitSlave;
        isce::core::Poly2d _dopplerSlave;
        isce::core::DateTime _refEpochSlave;
        isce::core::DateTime _sensingStartSlave;

        // isce::product objects
        isce::product::ImageMode _modeMaster;
        isce::product::ImageMode _modeSlave;

        // Projection related data
        isce::core::ProjectionBase * _projTopo;

        // Processing parameters
        int _numiter;
        double _threshold;
        isce::core::orbitInterpMethod _orbitMethod;
};

// Get inline implementations for Baseline
#define ISCE_GEOMETRY_BASELINE_ICC
#include "Baseline.icc"
#undef ISCE_GEOMETRY_BASELINE_ICC

#endif

// end of file
