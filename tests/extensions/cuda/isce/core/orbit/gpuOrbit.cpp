// -*- C++ -*-
// -*- coding: utf-8 -*-
//
// Author: Liang Yu
// Copyright 2018
//

#include <cmath>
#include <iostream>
#include <vector>
#include "isce/core/Constants.h"
#include "isce/core/Orbit.h"
#include "isce/core/cuda/gpuOrbit.h"
#include "gtest/gtest.h"

using isce::core::orbitInterpMethod;
using isce::core::HERMITE_METHOD;
using isce::core::LEGENDRE_METHOD;
using isce::core::SCH_METHOD;
using isce::core::Orbit;
using isce::core::cuda::gpuOrbit;
using std::endl;
using std::vector;
using isce::core::cartesian_t;

struct gpuOrbitTest : public ::testing::Test {
    virtual void SetUp() {
        fails = 0;
    }
    virtual void TearDown() {
        if (fails > 0) {
            std::cerr << "gpuOrbit::TearDown sees failures" << std::endl;
        }
    }
    unsigned fails;
};

#define compareTriplet(a,b,c)\
    EXPECT_NEAR(a[0], b[0], c); \
    EXPECT_NEAR(a[1], b[1], c); \
    EXPECT_NEAR(a[2], b[2], c);

void makeLinearSV(double dt, cartesian_t &opos, cartesian_t &ovel, cartesian_t &pos,
                  cartesian_t &vel) {
    pos = {opos[0] + (dt * ovel[0]), opos[1] + (dt * ovel[1]), opos[2] + (dt * ovel[2])};
    vel = ovel;
}

TEST_F(gpuOrbitTest, LinearSCH) {
    /*
     * Test linear orbit.
     */

    // create mirror orbit objects
    Orbit orb_cpu(1,11);
    double t = 1000.;
    cartesian_t opos = {0., 0., 0.};
    cartesian_t ovel = {4000., -1000., 4500.};
    cartesian_t pos, vel;
    cartesian_t hpos, hvel;

    // Create straight-line orbit with 11 state vectors, each 10 s apart
    for (int i=0; i<11; i++) {
        makeLinearSV(i*10., opos, ovel, pos, vel); 
        orb_cpu.setStateVector(i, t+(i*10.), pos, vel);
    }

    // deep copy create same orbit on GPU
    gpuOrbit orb_gpu(orb_cpu);

    // Interpolation test times
    double test_t[] = {23.3, 36.7, 54.5, 89.3};
    cartesian_t ref_pos, ref_vel;

    for (int i=0; i<4; i++) {
        makeLinearSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        orb_cpu.interpolate(t+test_t[i], pos, vel, SCH_METHOD);
        orb_gpu.interpolateSCHOrbit_h(t+test_t[i], hpos, hvel);
        compareTriplet(pos, hpos, 1.0e-5);
        compareTriplet(vel, hvel, 1.0e-6);
        compareTriplet(ref_pos, hpos, 1.0e-5);
        compareTriplet(ref_vel, hvel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

TEST_F(gpuOrbitTest,LinearHermite){
    /*
     * Test linear orbit.
     */

    // create mirror orbit objects
    Orbit orb_cpu(1,11);
    double t = 1000.;
    cartesian_t opos = {0., 0., 0.};
    cartesian_t ovel = {4000., -1000., 4500.};
    cartesian_t pos, vel;
    cartesian_t hpos, hvel;

    // Create straight-line orbit with 11 state vectors, each 10 s apart
    for (int i=0; i<11; i++) {
        makeLinearSV(i*10., opos, ovel, pos, vel);
        orb_cpu.setStateVector(i, t+(i*10.), pos, vel);
    }

    // deep copy create same orbit on GPU
    gpuOrbit orb_gpu(orb_cpu);

    // Interpolation test times
    double test_t[] = {23.3, 36.7, 54.5, 89.3};
    cartesian_t ref_pos, ref_vel;

    for (int i=0; i<4; i++) {
        makeLinearSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        orb_cpu.interpolate(t+test_t[i], pos, vel, HERMITE_METHOD);
        orb_gpu.interpolateWGS84Orbit_h(t+test_t[i], hpos, hvel);
        compareTriplet(pos, hpos, 1.0e-5);
        compareTriplet(vel, hvel, 1.0e-6);
        compareTriplet(ref_pos, hpos, 1.0e-5);
        compareTriplet(ref_vel, hvel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

TEST_F(gpuOrbitTest,LinearLegendre){
    /*
     * Test linear orbit.
     */

    // create mirror orbit objects
    Orbit orb_cpu(1,11);
    double t = 1000.;
    cartesian_t opos = {0., 0., 0.};
    cartesian_t ovel = {4000., -1000., 4500.};
    cartesian_t pos, vel;
    cartesian_t hpos, hvel;

    // Create straight-line orbit with 11 state vectors, each 10 s apart
    for (int i=0; i<11; i++) {
        makeLinearSV(i*10., opos, ovel, pos, vel);
        orb_cpu.setStateVector(i, t+(i*10.), pos, vel);
    }

    // deep copy create same orbit on GPU
    gpuOrbit orb_gpu(orb_cpu);

    // Interpolation test times
    double test_t[] = {23.3, 36.7, 54.5, 89.3};
    cartesian_t ref_pos, ref_vel;

    for (int i=0; i<4; i++) {
        makeLinearSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        orb_cpu.interpolate(t+test_t[i], pos, vel, LEGENDRE_METHOD);
        orb_gpu.interpolateLegendreOrbit_h(t+test_t[i], hpos, hvel);
        compareTriplet(pos, hpos, 1.0e-5);
        compareTriplet(vel, hvel, 1.0e-6);
        compareTriplet(ref_pos, hpos, 1.0e-5);
        compareTriplet(ref_vel, hvel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

void makeCircularSV(double dt, cartesian_t &opos, cartesian_t &ovel, cartesian_t &pos,
                    cartesian_t &vel) {
    double omega1 = (2. * M_PI) / 7000.;
    double omega2 = (2. * M_PI) / 4000.;
    double theta1 = (2. * M_PI) / 8.;
    double theta2 = (2. * M_PI) / 12.;
    double radius = 8000000.;
    double ang1 = theta1 + (dt * omega1);
    double ang2 = theta2 + (dt * omega2);
    pos = {opos[0] + (radius * cos(ang1)),
           opos[1] + (radius * (sin(ang1) + cos(ang2))),
           opos[2] + (radius * sin(ang2))};
    vel = {radius * -omega1 * sin(ang1),
           radius * ((omega1 * cos(ang1)) - (omega2 * sin(ang2))),
           radius * omega2 * cos(ang2)};
}

TEST_F(gpuOrbitTest,CircleSCH) {
    /*
     * Test circular orbit.
     */

    Orbit orb_cpu(1,11);
    double t = 1000.;
    cartesian_t opos = {7000000., -4500000., 7800000.};
    cartesian_t ovel, pos, vel, hpos, hvel;

    // Create circular orbit with 11 state vectors, each 5 s apart
    for (int i=0; i<11; i++) {
        makeCircularSV(i*5., opos, ovel, pos, vel);
        orb_cpu.setStateVector(i, t+(i*5.), pos, vel);
    }

    // deep copy create same orbit on GPU
    gpuOrbit orb_gpu(orb_cpu);

    // Interpolation test times
    double test_t[] = {11.65, 18.35, 27.25, 44.65};
    cartesian_t ref_pos, ref_vel;

    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<4; i++) {
        makeCircularSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        orb_cpu.interpolate(t+test_t[i], pos, vel, SCH_METHOD);
        orb_gpu.interpolateSCHOrbit_h(t+test_t[i], hpos, hvel);
        compareTriplet(pos, hpos, 1.0e-5);
        compareTriplet(vel, hvel, 1.0e-6);
        compareTriplet(ref_pos, hpos, 1.0e-5);
        compareTriplet(ref_vel, hvel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

TEST_F(gpuOrbitTest,CircleHermite) {
    /*
     * Test circular orbit.
     */

    Orbit orb_cpu(1,11);
    double t = 1000.;
    cartesian_t opos = {7000000., -4500000., 7800000.};
    cartesian_t ovel, pos, vel, hpos, hvel;

    // Create circular orbit with 11 state vectors, each 5 s apart
    for (int i=0; i<11; i++) {
        makeCircularSV(i*5., opos, ovel, pos, vel);
        orb_cpu.setStateVector(i, t+(i*5.), pos, vel);
    }

    // deep copy create same orbit on GPU
    gpuOrbit orb_gpu(orb_cpu);

    // Interpolation test times
    double test_t[] = {11.65, 18.35, 27.25, 44.65};
    cartesian_t ref_pos, ref_vel;

    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<4; i++) {
        makeCircularSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        orb_cpu.interpolate(t+test_t[i], pos, vel, HERMITE_METHOD);
        orb_gpu.interpolateWGS84Orbit_h(t+test_t[i], hpos, hvel);
        compareTriplet(pos, hpos, 1.0e-5);
        compareTriplet(vel, hvel, 1.0e-6);
        compareTriplet(ref_pos, hpos, 1.0e-5);
        compareTriplet(ref_vel, hvel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

TEST_F(gpuOrbitTest,CircleLegendre) {
    /*
     * Test circular orbit.
     */

    Orbit orb_cpu(1,11);
    double t = 1000.;
    cartesian_t opos = {7000000., -4500000., 7800000.};
    cartesian_t ovel, pos, vel, hpos, hvel;

    // Create circular orbit with 11 state vectors, each 5 s apart
    for (int i=0; i<11; i++) {
        makeCircularSV(i*5., opos, ovel, pos, vel);
        orb_cpu.setStateVector(i, t+(i*5.), pos, vel);
    }

    // deep copy create same orbit on GPU
    gpuOrbit orb_gpu(orb_cpu);

    // Interpolation test times
    double test_t[] = {11.65, 18.35, 27.25, 44.65};
    cartesian_t ref_pos, ref_vel;

    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<4; i++) {
        makeCircularSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        orb_cpu.interpolate(t+test_t[i], pos, vel, LEGENDRE_METHOD);
        orb_gpu.interpolateLegendreOrbit_h(t+test_t[i], hpos, hvel);
        compareTriplet(pos, hpos, 1.0e-5);
        compareTriplet(vel, hvel, 1.0e-6);
        compareTriplet(ref_pos, hpos, 1.0e-5);
        compareTriplet(ref_vel, hvel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

void makePolynomialSV(double dt, vector<double> &xpoly, vector<double> &ypoly,
                                 vector<double> &zpoly, cartesian_t &pos,
                                 cartesian_t &vel) {

    pos[0] = 0.0;
    double fact = 1.0;
    for (int i=0; i < static_cast<int>(xpoly.size()); i++, fact*=dt)
    {
        pos[0] += fact * xpoly[i];
    }

    vel[0] = 0.0;
    fact = 1.0;
    for(int i=1; i < static_cast<int>(xpoly.size()); i++, fact*=dt)
    {
        vel[0] += i * xpoly[i] * fact;
    }


    pos[1] = 0.0;
    fact = 1.0;
    for (int i=0; i < static_cast<int>(ypoly.size()); i++, fact*=dt)
    {
        pos[1] += fact * ypoly[i];
    }

    vel[1] = 0.0;
    fact = 1.0;
    for(int i=1; i < static_cast<int>(ypoly.size()); i++, fact*=dt)
    {
        vel[1] += i * ypoly[i] * fact;
    }


    pos[2] = 0.0;
    fact = 1.0;
    for (int i=0; i < static_cast<int>(zpoly.size()); i++, fact*=dt)
    {
        pos[2] += fact * zpoly[i];
    }

    vel[2] = 0.0;
    fact = 1.0;
    for(int i=1; i < static_cast<int>(zpoly.size()); i++, fact*=dt)
    {
        vel[2] += i * zpoly[i] * fact;
    }
}

TEST_F(gpuOrbitTest,PolynomialSCH) {
    /*
     * Test polynomial orbit.
     */

    Orbit orb_cpu(1,11);
    double t = 1000.;
    cartesian_t pos, vel, hpos, hvel;

    vector<double> xpoly = {-7000000., 5435., -45.0, 7.3};
    vector<double> ypoly = {5400000., -4257., 23.0, 3.9, 0.01};
    vector<double> zpoly = {0.0, 7000., 11.0};

    // Create straight-line orbit with 11 state vectors, each 10 s apart
    for (int i=0; i<11; i++) {
        makePolynomialSV(i*10., xpoly, ypoly, zpoly, pos, vel);
        orb_cpu.setStateVector(i, t+(i*10.), pos, vel);
    }

    // deep copy create same orbit on GPU
    gpuOrbit orb_gpu(orb_cpu);

    // Interpolation test times
    double test_t[] = {23.3, 36.7, 54.5, 89.3};
    cartesian_t ref_pos, ref_vel;

    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<4; i++) {
        makePolynomialSV(test_t[i], xpoly, ypoly, zpoly, ref_pos, ref_vel);
        orb_cpu.interpolate(t+test_t[i], pos, vel, SCH_METHOD);
        orb_gpu.interpolateSCHOrbit_h(t+test_t[i], hpos, hvel);
        compareTriplet(ref_pos, pos, 1.0e-5);
        compareTriplet(ref_vel, vel, 1.0e-6);
        compareTriplet(ref_pos, hpos, 1.0e-5);
        compareTriplet(ref_vel, hvel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

TEST_F(gpuOrbitTest,PolynomialHermite) {
    /*
     * Test polynomial orbit.
     */

    Orbit orb_cpu(1,11);
    double t = 1000.;
    cartesian_t pos, vel, hpos, hvel;

    vector<double> xpoly = {-7000000., 5435., -45.0, 7.3};
    vector<double> ypoly = {5400000., -4257., 23.0, 3.9, 0.01};
    vector<double> zpoly = {0.0, 7000., 11.0};

    // Create straight-line orbit with 11 state vectors, each 10 s apart
    for (int i=0; i<11; i++) {
        makePolynomialSV(i*10., xpoly, ypoly, zpoly, pos, vel);
        orb_cpu.setStateVector(i, t+(i*10.), pos, vel);
    }

    // deep copy create same orbit on GPU
    gpuOrbit orb_gpu(orb_cpu);

    // Interpolation test times
    double test_t[] = {23.3, 36.7, 54.5, 89.3};
    cartesian_t ref_pos, ref_vel;

    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<4; i++) {
        makePolynomialSV(test_t[i], xpoly, ypoly, zpoly, ref_pos, ref_vel);
        orb_cpu.interpolate(t+test_t[i], pos, vel, HERMITE_METHOD);
        orb_gpu.interpolateWGS84Orbit_h(t+test_t[i], hpos, hvel);
        compareTriplet(ref_pos, pos, 1.0e-5);
        compareTriplet(ref_vel, vel, 1.0e-6);
        compareTriplet(ref_pos, hpos, 1.0e-5);
        compareTriplet(ref_vel, hvel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

TEST_F(gpuOrbitTest,LegendreSCH) {
    /*
     * Test polynomial orbit.
     */

    Orbit orb_cpu(1,11);
    double t = 1000.;
    cartesian_t pos, vel, hpos, hvel;

    vector<double> xpoly = {-7000000., 5435., -45.0, 7.3};
    vector<double> ypoly = {5400000., -4257., 23.0, 3.9, 0.01};
    vector<double> zpoly = {0.0, 7000., 11.0};

    // Create straight-line orbit with 11 state vectors, each 10 s apart
    for (int i=0; i<11; i++) {
        makePolynomialSV(i*10., xpoly, ypoly, zpoly, pos, vel);
        orb_cpu.setStateVector(i, t+(i*10.), pos, vel);
    }

    // deep copy create same orbit on GPU
    gpuOrbit orb_gpu(orb_cpu);

    // Interpolation test times
    double test_t[] = {23.3, 36.7, 54.5, 89.3};
    cartesian_t ref_pos, ref_vel;

    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<4; i++) {
        makePolynomialSV(test_t[i], xpoly, ypoly, zpoly, ref_pos, ref_vel);
        orb_cpu.interpolate(t+test_t[i], pos, vel, LEGENDRE_METHOD);
        orb_gpu.interpolateLegendreOrbit_h(t+test_t[i], hpos, hvel);
        compareTriplet(ref_pos, pos, 1.0e-5);
        compareTriplet(ref_vel, vel, 1.0e-6);
        compareTriplet(ref_pos, hpos, 1.0e-5);
        compareTriplet(ref_vel, hvel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
