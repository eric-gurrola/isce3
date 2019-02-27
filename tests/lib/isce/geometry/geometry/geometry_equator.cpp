//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Piyush Agram
// Copyright 2019
//

#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <fstream>
#include <gtest/gtest.h>

// isce::core
#include "isce/core/Constants.h"
#include "isce/core/DateTime.h"
#include "isce/core/Ellipsoid.h"
#include "isce/core/Orbit.h"
#include "isce/core/LUT1d.h"
#include "isce/core/LinAlg.h"

// isce::geometry
#include "isce/geometry/geometry.h"


struct GeometryTest : public ::testing::Test {

    // isce::core objects
    isce::core::Ellipsoid ellipsoid;
    isce::core::Orbit orbit;
    double hsat;

    // Constructor
    GeometryTest(){}

    void Setup(double lon0, double omega, int Nvec) 
    {
        //WGS84 ellipsoid 
        ellipsoid = isce::core::Ellipsoid(6378137.,.0066943799901);

        //Satellite height
        hsat = 700000.0;

        //Setup orbit
        isce::core::DateTime t0("2017-02-12T01:12:30.0");
        for(int ii=0; ii < Nvec; ii++)
        {
            double deltat = ii * 10.0;
            double lon = lon0 + omega * deltat;
            isce::core::cartesian_t pos, vel;

            pos[0] = (ellipsoid.a() + hsat) * std::cos(lon);
            pos[1] = (ellipsoid.a() + hsat) * std::sin(lon);
            pos[2] = 0.0;

            vel[0] = -omega * pos[1];
            vel[1] = omega * pos[0];
            vel[2] = 0.0;

            isce::core::DateTime epoch = t0 + deltat;

            isce::core::StateVector sv;
            sv.date(epoch.isoformat());
            sv.position(pos);
            sv.velocity(vel);

            orbit.stateVectors.push_back(sv);
        }

        orbit.reformatOrbit(t0);
    }

    //Solve for Geocentric latitude given a slant range
    double solve(double R)
    {
        double temp = 1.0 + hsat/ellipsoid.a();
        double temp1 = R/ellipsoid.a();
        double A = ellipsoid.e2();
        double B = - 2 * temp;
        double C = temp*temp + 1.0 - ellipsoid.e2()- temp1*temp1;

        //Solve quadratic equation
        double D = std::sqrt(B * B - 4 * A * C);

        double x1 = (D-B)/(2*A);
        double x2 = -(D+B)/(2*A);

        double x = ( std::abs(x1) > std::abs(x2)) ? x2 : x1;
        return x;

    }
};

TEST_F(GeometryTest, RdrToGeoEquator) {
    
    // Loop over test data
    const double degrees = 180.0 / M_PI;

    //Moving at 0.1 degrees / sec
    const double lon0 = 0.0;
    const double omega = 0.1/degrees;
    const int Nvec = 10;
    Setup(lon0, omega, Nvec);

    //Constant height DEM
    isce::geometry::DEMInterpolator dem(0.);

    //Test over 20 points
    for (size_t ii = 0; ii < 20; ++ii) 
    {
        //Azimuth time
        double tinp = 5.0 + ii * 2.0;
       
        
        //Slant range
        double rng = 800000. + 10.0 * ii;

        //Theoretical solutions
        double expectedLon = lon0 + omega * tinp;

        //Expected solution
        double geocentricLat = std::acos(solve(rng));

        //Convert geocentric coords to xyz
        isce::core::cartesian_t xyz = { ellipsoid.a() * std::cos(geocentricLat) * std::cos(expectedLon),
                                        ellipsoid.a() * std::cos(geocentricLat) * std::sin(expectedLon), ellipsoid.b() * std::sin(geocentricLat)};
        isce::core::cartesian_t expLLH;
        ellipsoid.xyzToLonLat(xyz, expLLH);

        // Initialize guess
        isce::core::cartesian_t targetLLH = {0.0, 0.0, 0.0};

        // Run rdr2geo with left looking side
        int stat = isce::geometry::rdr2geo(tinp, rng, 0.0,
            orbit, ellipsoid, dem, targetLLH, 0.24, 1,
            1.0e-8, 25, 15, isce::core::HERMITE_METHOD);

        // Check
        ASSERT_EQ(stat, 1);
        ASSERT_NEAR(targetLLH[0], expLLH[0], 1.0e-8);
        ASSERT_NEAR(targetLLH[1], expLLH[1], 1.0e-8);
        ASSERT_NEAR(targetLLH[2], 0.0, 1.0e-8);

        // Run again with right looking side
        targetLLH = {0., 0., 0.};
        stat = isce::geometry::rdr2geo(tinp, rng, 0.0,
            orbit, ellipsoid, dem, targetLLH, 0.24, -1,
            1.0e-8, 25, 15, isce::core::HERMITE_METHOD);

        // Check
        ASSERT_EQ(stat, 1);
        ASSERT_NEAR(targetLLH[0], expLLH[0], 1.0e-8);
        ASSERT_NEAR(targetLLH[1], -expLLH[1], 1.0e-8);
        ASSERT_NEAR(targetLLH[2], 0.0, 1.0e-8);
    }
   
}

TEST_F(GeometryTest, GeoToRdrEquator) {
    
    // Loop over test data
    const double degrees = 180.0 / M_PI;

    //Moving at 0.1 degrees / sec
    const double lon0 = 0.0;
    const double omega = 0.1/degrees;
    const int Nvec = 10;
    Setup(lon0, omega, Nvec);

    //Constant zero Doppler
    isce::core::LUT2d<double> zeroDoppler;

    // Dummy wavelength
    const double wavelength = 0.24;

    //Test over 20 points
    for (size_t ii = 0; ii < 20; ++ii) 
    {
        //Azimuth time
        double tinp = 5.0 + ii * 2.0;
       
        //Start with geocentric lat
        double geocentricLat = (2.0 + ii * 0.1)/degrees ;

        //Theoretical solutions
        double expectedLon = lon0 + omega * tinp;

        //Convert geocentric coords to xyz
        isce::core::cartesian_t targ_xyz = { ellipsoid.a() * std::cos(geocentricLat) * std::cos(expectedLon),
                                             ellipsoid.a() * std::cos(geocentricLat) * std::sin(expectedLon), 
                                             ellipsoid.b() * std::sin(geocentricLat)};
        isce::core::cartesian_t targ_LLH;
        ellipsoid.xyzToLonLat(targ_xyz, targ_LLH);

        //Expected satellite position
        isce::core::cartesian_t sat_xyz = { (ellipsoid.a() + hsat) * std::cos(expectedLon),
                                            (ellipsoid.a() + hsat) * std::sin(expectedLon),
                                            0.};

        isce::core::cartesian_t los;
        isce::core::LinAlg::linComb(1.0, sat_xyz, -1.0, targ_xyz, los);

        //Expected slant range
        double expRange = isce::core::LinAlg::norm(los);

        // Run geo2rdr
        double aztime, slantRange;

        // Run rdr2geo with left looking side
        int stat = isce::geometry::geo2rdr(targ_LLH, ellipsoid, orbit,
            zeroDoppler, aztime, slantRange, wavelength, 1.0e-9, 50, 10.0);

        // Check
        ASSERT_EQ(stat, 1);
        ASSERT_NEAR(aztime, tinp, 1.0e-6);
        ASSERT_NEAR(slantRange, expRange, 1.0e-8);
    }
   
}

int main(int argc, char * argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// end of file
