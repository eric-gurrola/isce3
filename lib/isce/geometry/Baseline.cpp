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
using isce::io::Raster;
using isce::core::Poly2d;
using isce::core::LinAlg;


// Run Baseline - main entrypoint
void isce::geometry::Baseline::
computeBaseline(isce::io::Raster & topoRaster,
                const std::string & outdir,
                double azshift, double rgshift) {

  // Create reusable pyre::journal channels
  pyre::journal::warning_t warning("isce.geometry.Baseline");
  pyre::journal::info_t info("isce.geometry.Baseline");

  // Cache the size of the DEM images
  const size_t demWidth = topoRaster.width();
  const size_t demLength = topoRaster.length();

  // Initialize projection for topo results
  _projTopo = isce::core::createProj(topoRaster.getEPSG());


  // Valarrays to hold line of results
  std::valarray<double> x(demWidth), y(demWidth), hgt(demWidth), inc(demWidth);
  std::valarray<float> bperp(demWidth), kz(demWidth);

  // Create output rasters
  Raster bperpRaster = Raster(outdir + "/bperp.bin", demWidth, demLength, 1,
                              GDT_Float32, "ENVI");
  Raster kzRaster = Raster(outdir + "/kz.bin", demWidth, demLength, 1,
                           GDT_Float32, "ENVI");

  // Cache sensing start (pasted from geo2rdr)
  double t0 = _modeMaster.startAzTime().secondsSinceEpoch(_refEpochMaster);
  // Adjust for const azimuth shift
  t0 -= (azshift - 0.5 * (_modeMaster.numberAzimuthLooks() - 1)) / _modeMaster.prf();

  // Cache starting range
  double r0 = _modeMaster.startingRange();
  // Adjust for constant range shift
  r0 -= (rgshift - 0.5 * (_modeMaster.numberRangeLooks() - 1)) * _modeMaster.rangePixelSpacing();

  // Compute azimuth time extents
  double dtaz = _modeMaster.numberAzimuthLooks() / _modeMaster.prf();
  const double tend = t0 + ((_modeMaster.length() - 1) * dtaz);
  const double tmid = 0.5 * (t0 + tend);

  // Compute range extents
  const double dmrg = _modeMaster.numberRangeLooks() * _modeMaster.rangePixelSpacing();
  const double rngend = r0 + ((_modeMaster.width() - 1) * dmrg);

  // Print out extents
  _printExtents(info, t0, tend, dtaz, r0, rngend, dmrg, demWidth, demLength);

  // Interpolate orbit to middle of the scene as a test
  _checkOrbitInterpolation(tmid);

    // Loop over DEM lines
    int converged = 0;
    for (size_t line = 0; line < demLength; ++line) {    // ml - REMOVE 100 when DONE debugging

      // Periodic diagnostic printing
      if ((line % 1000) == 0) {     //ml - change 100 to 1000

            info
                << "Processing line: " << line << " " << pyre::journal::newline
                << "Dopplers near mid far (master): "
                << _dopplerMaster.eval(0, 0) << " "
                << _dopplerMaster.eval(0, (_modeMaster.width() / 2) - 1) << " "
                << _dopplerMaster.eval(0, _modeMaster.width() - 1) << " "
                << pyre::journal::newline
                << "Dopplers near mid far (slave): "
                << _dopplerSlave.eval(0, 0) << " "
                << _dopplerSlave.eval(0, (_modeSlave.width() / 2) - 1) << " "
                << _dopplerSlave.eval(0, _modeSlave.width() - 1) << " "
                << pyre::journal::endl;
        }


      // Read line of data
      topoRaster.getLine(x, line, 1);
      topoRaster.getLine(y, line, 2);
      topoRaster.getLine(hgt, line, 3);
      topoRaster.getLine(inc, line, 6);

      // Loop over DEM pixels
      #pragma omp parallel for reduction(+:converged)
      for (size_t pixel = 0; pixel < demWidth; ++pixel) {

        // Convert topo XYZ to LLH
        isce::core::cartesian_t xyz{x[pixel], y[pixel], hgt[pixel]};
        isce::core::cartesian_t llh;
        _projTopo->inverse(xyz, llh);

        // Perform geo->rdr iterations
        double aztime, slantRange, kzSample, bperpSample;
        cartesian_t satposMaster, satposSlave;
        int geostat = isce::geometry::baseline(llh, _ellipsoidMaster, _ellipsoidSlave,
                                               _orbitMaster, _orbitSlave,
                                               _dopplerMaster, _dopplerSlave,
                                               _modeMaster, _modeSlave,
                                               aztime, slantRange, _threshold, _numiter, 1.0e-8,
                                               bperpSample, kzSample
                                               );

        //>>>> TEMPORARY HACK
        double wvl = _modeMaster.wavelength();
        kzSample = 4*M_PI / wvl * bperpSample / (slantRange*sin(inc[pixel]*M_PI/180.));
        //<<<<<

        if ((line % 1000) == 0) {
          if ((pixel % 3000) == 0) {     //ml - change 100 to 1000
            //std::cout << basTot << std::endl;
            info
              << pyre::journal::at(__HERE__)
              << "Norm of satposMaster-satposSlave: "
              << bperpSample
              << pyre::journal::newline
              << "Perpendicular baseline: "
              << kzSample
              << pyre::journal::newline
              << "wavelength: "
              << wvl
              << pyre::journal::newline
              << "slantrange: "
              << slantRange
              << pyre::journal::endl;
          }
        }



            // Check of solution is out of bounds
            bool isOutside = false;
            if ((aztime < t0) || (aztime > tend))
                isOutside = true;
            if ((slantRange < r0) || (slantRange > rngend))
                isOutside = true;

            // Save result if valid
            if (!isOutside) {
              bperp[pixel] = bperpSample; //((slantRange - r0) / dmrg) - float(pixel);
              kz[pixel] = kzSample; //((aztime - t0) / dtaz) - float(line);
              converged += geostat;
              //std::cout << "converged" << std::endl;
            } else {
                bperp[pixel] = NULL_VALUE;
                kz[pixel] = NULL_VALUE;
            }
        }

        // Write data
        bperpRaster.setLine(bperp, line);
        kzRaster.setLine(kz, line);
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
         << "Radar image length: " << _modeMaster.length() << pyre::journal::newline
         << "Radar image width: " << _modeMaster.width() << pyre::journal::newline
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
