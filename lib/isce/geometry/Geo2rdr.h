//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Bryan Riel, Joshua Cohen
// Copyright 2017-2018

#ifndef ISCE_CORE_GEO2RDR_H
#define ISCE_CORE_GEO2RDR_H

// pyre
#include <portinfo>
#include <pyre/journal.h>

// isce::core
#include <isce/core/Metadata.h>
#include <isce/core/Orbit.h>
#include <isce/core/Poly2d.h>
#include <isce/core/Ellipsoid.h>
#include <isce/core/Peg.h>
#include <isce/core/Pegtrans.h>
#include <isce/core/Projections.h>

// isce::io
#include <isce/io/Raster.h>

// isce::product
#include <isce/product/Product.h>

// Declaration
namespace isce {
    namespace geometry {
        class Geo2rdr;
    }
}

// Geo2rdr declaration
/** Transformer from map coordinates to radar geometry coordinates.
 *
 * See <a href="overview_geometry.html#inversegeom">geometry overview</a> for description of the algorithm. */
class isce::geometry::Geo2rdr {

    public:
        /** Constructor from product */
        inline Geo2rdr(isce::product::Product &);
        /** Constructor from core objects*/
        inline Geo2rdr(const isce::core::Ellipsoid &,
                       const isce::core::Orbit &,
                       const isce::core::Poly2d &,
                       const isce::core::Metadata &);

        /** Set convergence threshold*/
        inline void threshold(double);
        /** Set number of Newton-Raphson iterations*/
        inline void numiter(int);
        /** Set orbit interpolation method */
        inline void orbitMethod(isce::core::orbitInterpMethod);

        /** Main entry point for the module */
        void geo2rdr(isce::io::Raster &,
                     const std::string &,
                     double, double);

        /** Alternate entry point with no offsets*/
        void geo2rdr(isce::io::Raster &,
                     const std::string &);

        /** NoData Value*/
        const double NULL_VALUE = -1.0e6;

        // Getters for isce objects
        /** Get Orbit object used for processing */
        inline const isce::core::Orbit & orbit() const { return _orbit; }
        /** Get Ellipsoid object used for processing */
        inline const isce::core::Ellipsoid & ellipsoid() const { return _ellipsoid; }
        /** Get Doppler model used for processing */
        inline const isce::core::Poly2d & doppler() const { return _doppler; }
        /** Get sensingStart used for processing */
        inline const isce::core::DateTime & sensingStart() const { return _sensingStart; }
        /** Get reference epoch of the orbit used for processing */
        inline const isce::core::DateTime & refEpoch() const { return _refEpoch; }
        /** Get imageMode object used for processing */
        inline const isce::product::ImageMode & mode() const { return _mode; }

        // Get geo2rdr processing options
        /** Return the azimuth time convergence threshold used for processing */
        inline double threshold() const { return _threshold; }
        /** Return number of Newton-Raphson iterations used for processing */
        inline int numiter() const { return _numiter; }
        /** Return the orbit interpolation method used for processing */
        inline isce::core::orbitInterpMethod orbitMethod() const { return _orbitMethod; }

    private:
        /** Print information for debugging */
        void _printExtents(pyre::journal::info_t &,
                           double, double, double,
                           double, double, double,
                           size_t, size_t);

        /** Quick check to ensure we can interpolate orbit to middle of DEM*/
        void _checkOrbitInterpolation(double);

    private:
        // isce::core objects
        isce::core::Ellipsoid _ellipsoid;
        isce::core::Orbit _orbit;
        isce::core::Poly2d _doppler;
        isce::core::DateTime _refEpoch;
        isce::core::DateTime _sensingStart;

        // isce::product objects
        isce::product::ImageMode _mode;

        // Projection related data
        isce::core::ProjectionBase * _projTopo;

        // Processing parameters
        int _numiter;
        double _threshold;
        size_t _linesPerBlock = 1000;
        isce::core::orbitInterpMethod _orbitMethod;
};

// Get inline implementations for Geo2rdr
#define ISCE_GEOMETRY_GEO2RDR_ICC
#include "Geo2rdr.icc"
#undef ISCE_GEOMETRY_GEO2RDR_ICC

#endif

// end of file
