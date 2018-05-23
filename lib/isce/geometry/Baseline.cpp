// -*- C++ -*-
// -*- coding: utf-8 -*-
//
// Author: Bryan V. Riel, Marco Lavalle, Joshua Cohen
// Copyright 2017-2018

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <future>
#include <valarray>
#include <algorithm>

// isce::core
#include <isce/core/Constants.h>
#include <isce/core/LinAlg.h>

// isce::geometry
#include "geometry.h"
#include "Baseline.h"

// pull in some isce::core namespaces
using isce::core::Raster;
using isce::core::Poly2d;
using isce::core::LinAlg;

// Run geo2rdr with no offsets
void isce::geometry::Baseline::
computeBaseline(isce::core::Raster & latRaster,
        isce::core::Raster & lonRaster,
        isce::core::Raster & hgtRaster,
        isce::core::Poly2d & dopplerMaster,
                isce::core::Poly2d & dopplerSlave,
        const std::string & outdir) {
  computeBaseline(latRaster, lonRaster, hgtRaster, dopplerMaster, dopplerSlave, outdir, 0.0, 0.0);
}

// Run geo2rdr - main entrypoint
void isce::geometry::Baseline::
computeBaseline(isce::core::Raster & latRaster,
        isce::core::Raster & lonRaster,
        isce::core::Raster & hgtRaster,
        isce::core::Poly2d & dopplerMaster,
                isce::core::Poly2d & dopplerSlave,
        const std::string & outdir,
        double azshift, double rgshift) {

    // Create reusable pyre::journal channels
    pyre::journal::warning_t warning("isce.geometry.Baseline");
    pyre::journal::info_t info("isce.geometry.Baseline");

    // Cache the size of the DEM images
    const size_t demWidth = hgtRaster.width();
    const size_t demLength = hgtRaster.length();
    const double rad = M_PI / 180.0;

    // Valarrays to hold line of results
    std::valarray<double> lat(demWidth), lon(demWidth), hgt(demWidth);
    std::valarray<float> rgoff(demWidth), azoff(demWidth);

    // Create output rasters
    Raster rgoffRaster = Raster(outdir + "/baseline.bin", demWidth, demLength, 1,
        GDT_Float32, "ISCE");
    Raster azoffRaster = Raster(outdir + "/kz.bin", demWidth, demLength, 1,
        GDT_Float32, "ISCE");

    // Cache sensing start
    double t0 = _metaMaster.sensingStart.secondsSinceEpoch(_refEpochMaster);
    // Adjust for const azimuth shift
    t0 -= (azshift - 0.5 * (_metaMaster.numberAzimuthLooks - 1)) / _metaMaster.prf;

    // Cache starting range
    double r0 = _metaMaster.rangeFirstSample;
    // Adjust for constant range shift
    r0 -= (rgshift - 0.5 * (_metaMaster.numberRangeLooks - 1)) * _metaMaster.slantRangePixelSpacing;

    // Compute azimuth time extents
    double dtaz = _metaMaster.numberAzimuthLooks / _metaMaster.prf;
    const double tend = t0 + ((_metaMaster.length - 1) * dtaz);
    const double tmid = 0.5 * (t0 + tend);

    // Compute range extents
    const double dmrg = _metaMaster.numberRangeLooks * _metaMaster.slantRangePixelSpacing;
    const double rngend = r0 + ((_metaMaster.width - 1) * dmrg);

    // Print out extents
    _printExtents(info, t0, tend, dtaz, r0, rngend, dmrg, demWidth, demLength);

    // Interpolate orbit to middle of the scene as a test
    _checkOrbitInterpolation(tmid);

    // Loop over DEM lines
    int converged = 0;
    for (size_t line = 0; line < demLength/100; ++line) { //ml - remove 100

      // Periodic diagnostic printing
      if ((line % 100) == 0) {     //ml - change 100 to 1000
            info
                << "Processing line: " << line << " " << pyre::journal::newline
                << "Dopplers near mid far (master): "
                << dopplerMaster.eval(0, 0) << " "
                << dopplerMaster.eval(0, (_metaMaster.width / 2) - 1) << " "
                << dopplerMaster.eval(0, _metaMaster.width - 1) << " "
                << pyre::journal::newline
                << "Dopplers near mid far (slave): "
                << dopplerSlave.eval(0, 0) << " "
                << dopplerSlave.eval(0, (_metaSlave.width / 2) - 1) << " "
                << dopplerSlave.eval(0, _metaSlave.width - 1) << " "
                << pyre::journal::endl;
        }

        // Read line of data
        latRaster.getLine(lat, line);
        lonRaster.getLine(lon, line);
        hgtRaster.getLine(hgt, line);

        // Loop over DEM pixels
        #pragma omp parallel for reduction(+:converged)
        for (size_t pixel = 0; pixel < demWidth; ++pixel) {

            // Perform geo->rdr iterations
            cartesian_t llh = {lat[pixel]*rad, lon[pixel]*rad, hgt[pixel]};
            double aztime, slantRange, basTot;
            cartesian_t satposMaster, satposSlave;
            int geostat = isce::geometry::baseline(
                                                   llh, _ellipsoidMaster, _ellipsoidSlave,
                                                   _orbitMaster, _orbitSlave,
                                                   dopplerMaster, dopplerSlave,
                                                   _metaMaster, _metaSlave,
                                                   aztime, slantRange, _threshold, _numiter, 1.0e-8, basTot
                                                   );

             if ((pixel % 3000) == 0) {     //ml - change 100 to 1000
               //std::cout << basTot << std::endl;
               info
                 << pyre::journal::at(__HERE__)
                 << "Norm of satposMaster-satposSlave: "
                 << basTot
                 << pyre::journal::endl;
             }



            // Check of solution is out of bounds
            bool isOutside = false;
            if ((aztime < t0) || (aztime > tend))
                isOutside = true;
            if ((slantRange < r0) || (slantRange > rngend))
                isOutside = true;

            // Save result if valid
            if (!isOutside) {
              rgoff[pixel] = basTot; //((slantRange - r0) / dmrg) - float(pixel);
                azoff[pixel] = ((aztime - t0) / dtaz) - float(line);
                converged += geostat;
            } else {
                rgoff[pixel] = NULL_VALUE;
                azoff[pixel] = NULL_VALUE;
            }
        }

        // Write data
        rgoffRaster.setLine(rgoff, line);
        azoffRaster.setLine(azoff, line);
    }

    // Print out convergence statistics
    info << "Total convergence: " << converged << " out of "
         << (demWidth * demLength) << pyre::journal::endl;

}

// Print extents and image sizes
void isce::geometry::Baseline::
_printExtents(pyre::journal::info_t & info, double t0, double tend, double dtaz,
              double r0, double rngend, double dmrg, size_t demWidth, size_t demLength) {
    info << pyre::journal::newline
         << "Starting acquisition time: " << t0 << pyre::journal::newline
         << "Stop acquisition time: " << tend << pyre::journal::newline
         << "Azimuth line spacing in seconds: " << dtaz << pyre::journal::newline
         << "Near range (m): " << r0 << pyre::journal::newline
         << "Far range (m): " << rngend << pyre::journal::newline
         << "Radar image length: " << _metaMaster.length << pyre::journal::newline
         << "Radar image width: " << _metaMaster.width << pyre::journal::newline
         << "Geocoded lines: " << demLength << pyre::journal::newline
         << "Geocoded samples: " << demWidth << pyre::journal::newline;
}

// Check we can interpolate orbit to middle of DEM
void isce::geometry::Baseline::
_checkOrbitInterpolation(double aztime) {
    cartesian_t satxyz, satvel;
    int stat = _orbitMaster.interpolate(aztime, satxyz, satvel, _orbitMethod);
    if (stat != 0) {
        pyre::journal::error_t error("isce.core.Baseline");
        error
            << pyre::journal::at(__HERE__)
            << "Error in Topo::topo - Error getting state vector for bounds computation."
            << pyre::journal::newline
            << " - requested time: " << aztime << pyre::journal::newline
            << " - bounds: " << _orbitMaster.UTCtime[0] << " -> " << _orbitMaster.UTCtime[_orbitMaster.nVectors-1]
            << pyre::journal::endl;
    }
}

// end of file
