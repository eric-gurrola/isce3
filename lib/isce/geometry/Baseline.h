//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Bryan Riel, Marco Lavalle, Joshua Cohen
// Copyright 2017-2018

#ifndef ISCE_CORE_BASELINE_H
#define ISCE_CORE_BASELINE_H

// pyre
#include <pyre/journal.h>

// isce::core
#include <isce/core/Metadata.h>
#include <isce/core/Orbit.h>
#include <isce/core/Poly2d.h>
#include <isce/core/LUT1d.h>
#include <isce/core/Ellipsoid.h>
#include <isce/core/Peg.h>
#include <isce/core/Pegtrans.h>
#include <isce/core/Projections.h>

// isce::io
#include <isce/io/Raster.h>


// isce::product
#include <isce/product/Product.h>
#include <isce/product/RadarGridParameters.h>


// Declaration
namespace isce {
    namespace geometry {
        class Baseline;
    }
}

// Baseline declaration
class isce::geometry::Baseline {

    public:
        // /** Constructor from product */
        // Baseline(isce::product::Product &, isce::product::Product &);

        // /** Construct from core object */
        // Baseline(isce::core::Ellipsoid,
        //                 isce::core::Orbit,
        //                 isce::product::ImageMode,
        //                 isce::core::Ellipsoid,
        //                 isce::core::Orbit,
        //                 isce::product::ImageMode);

        // // Set options
        // inline void threshold(double);
        // inline void numiter(int);
        // inline void orbitMethod(isce::core::orbitInterpMethod);


        // /** Run geo2rdr with constant offsets and internally created offset rasters */
        // void computeBaseline(isce::io::Raster & topoRaster,
        //                      const std::string & outdir,
        //                      double azshift=0.0, double rgshift=0.0);


        // // Value for null pixels
        // const double NULL_VALUE = -1.0e6;

        /** Constructor from product */
        inline Baseline(const isce::product::Product &,
                        const isce::product::Product &,
                        char frequency = 'A',
                        bool nativeDoppler = false,
                        size_t numberAzimuthLooks = 1,
                        size_t numberRangeLooks = 1);

        /** Constructor from core objects */
        inline Baseline(const isce::core::Ellipsoid &,
                        const isce::core::Orbit &,
                        const isce::core::LUT2d<double> &,
                        const isce::core::Metadata &,
                        const isce::core::Ellipsoid &,
                        const isce::core::Orbit &,
                        const isce::core::LUT2d<double> &,
                        const isce::core::Metadata &,
                        size_t numberAzimuthLooks = 1,
                        size_t numberRangeLooks = 1);

        /** Set convergence threshold*/
        inline void threshold(double);
        /** Set number of Newton-Raphson iterations*/
        inline void numiter(int);
        /** Set orbit interpolation method */
        inline void orbitMethod(isce::core::orbitInterpMethod);

        /** Run geo2rdr with offsets and externally created offset rasters */
        // void geo2rdr(isce::io::Raster & topoRaster,
        //              isce::io::Raster & rgoffRaster,
        //              isce::io::Raster & azoffRaster,
        //              double azshift=0.0, double rgshift=0.0);

        /** Run geo2rdr with constant offsets and internally created offset rasters */
        void computeBaseline(isce::io::Raster & topoRaster,
                             const std::string & outdir,
                             double azshift=0.0, double rgshift=0.0);

        /** NoData Value*/
        const double NULL_VALUE = -1.0e6;

        // Getters for isce objects
        /** Get Orbit object used for processing */
        inline const isce::core::Orbit & orbitMaster() const { return _orbitMaster; }
        /** Get Ellipsoid object used for processing */
        inline const isce::core::Ellipsoid & ellipsoidMaster() const { return _ellipsoidMaster; }
        /** Get Doppler model used for processing */
        inline const isce::core::LUT2d<double> & dopplerMaster() const { return _dopplerMaster; }

        /** Get read-only reference to RadarGridParameters */
        inline const isce::product::RadarGridParameters & radarGridParametersMaster() const {
            return _radarGridMaster;
        }


        /** Get Orbit object used for processing */
        inline const isce::core::Orbit & orbitSlave() const { return _orbitSlave; }
        /** Get Ellipsoid object used for processing */
        inline const isce::core::Ellipsoid & ellipsoidSlave() const { return _ellipsoidSlave; }
        /** Get Doppler model used for processing */
        inline const isce::core::LUT2d<double> & dopplerSlave() const { return _dopplerSlave; }

        /** Get read-only reference to RadarGridParameters */
        inline const isce::product::RadarGridParameters & radarGridParametersSlave() const {
            return _radarGridSlave;
        }


        // Get geo2rdr processing options
        /** Return the azimuth time convergence threshold used for processing */
        inline double threshold() const { return _threshold; }
        /** Return number of Newton-Raphson iterations used for processing */
        inline int numiter() const { return _numiter; }
        /** Return the orbit interpolation method used for processing */
        inline isce::core::orbitInterpMethod orbitMethod() const { return _orbitMethod; }


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
        isce::core::LUT2d<double> _dopplerMaster;

        isce::core::Ellipsoid _ellipsoidSlave;
        isce::core::Orbit _orbitSlave;
        isce::core::LUT2d<double> _dopplerSlave;

        // RadarGridParameters
        isce::product::RadarGridParameters _radarGridMaster;
        isce::product::RadarGridParameters _radarGridSlave;

        // Projection related data
        isce::core::ProjectionBase * _projTopo;

        // Processing parameters
        int _numiter;
        double _threshold;
        size_t _linesPerBlock = 1000;
        isce::core::orbitInterpMethod _orbitMethod;

};

// Get inline implementations for Baseline
#define ISCE_GEOMETRY_BASELINE_ICC
#include "Baseline.icc"
#undef ISCE_GEOMETRY_BASELINE_ICC

#endif

// end of file
