//
// Author: Joshua Cohen
// Copyright 2017
//

#include <cmath>
#include <iostream>
#include <vector>
#include <pyre/journal.h>
#include <gtest/gtest.h>
#include <isce/core.h>


struct OrbitTest : public ::testing::Test {
    virtual void SetUp() {
        fails = 0;
    }
    virtual void TearDown() {
        if (fails > 0) {
            // create testerror channel
            pyre::journal::firewall_t channel("tests.lib.core.fails");
            // complain
            channel
                << pyre::journal::at(__HERE__)
                << "Orbit::TearDown sees " << fails << " failures"
                << pyre::journal::endl;
        }
    }
    unsigned fails;
};


#define compareTriplet(a,b,c)\
    EXPECT_NEAR(a[0], b[0], c); \
    EXPECT_NEAR(a[1], b[1], c); \
    EXPECT_NEAR(a[2], b[2], c);


void
makeLinearSV(double dt, std::vector<double> &opos, std::vector<double> &ovel,
             std::vector<double> &pos, std::vector<double> &vel) {
    pos = {opos[0] + (dt * ovel[0]), opos[1] + (dt * ovel[1]), opos[2] + (dt * ovel[2])};
    vel = ovel;
}

TEST_F(OrbitTest,LinearSCH){
    /*
     * Test linear orbit.
     */

    isce::core::Orbit orb(1,11);
    double t = 1000.;
    std::vector<double> opos = {0., 0., 0.};
    std::vector<double> ovel = {4000., -1000., 4500.};
    std::vector<double> pos(3), vel(3);

    // Create straight-line orbit with 11 state vectors, each 10 s apart
    for (int i=0; i<11; i++) {
        makeLinearSV(i*10., opos, ovel, pos, vel);
        orb.setStateVector(i, t+(i*10.), pos, vel);
    }

    // Interpolation test times
    double test_t[] = {23.3, 36.7, 54.5, 89.3};
    std::vector<double> ref_pos(3), ref_vel(3);


    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<4; i++) {
        makeLinearSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        orb.interpolate(t+test_t[i], pos, vel, isce::core::SCH_METHOD);
        compareTriplet(pos, ref_pos, 1.0e-5);
        compareTriplet(vel, ref_vel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

TEST_F(OrbitTest,LinearHermite){
    /*
     * Test linear orbit.
     */

    isce::core::Orbit orb(1,11);
    double t = 1000.;
    std::vector<double> opos = {0., 0., 0.};
    std::vector<double> ovel = {4000., -1000., 4500.};
    std::vector<double> pos(3), vel(3);

    // Create straight-line orbit with 11 state vectors, each 10 s apart
    for (int i=0; i<11; i++) {
        makeLinearSV(i*10., opos, ovel, pos, vel);
        orb.setStateVector(i, t+(i*10.), pos, vel);
    }

    // Interpolation test times
    double test_t[] = {23.3, 36.7, 54.5, 89.3};
    std::vector<double> ref_pos(3), ref_vel(3);

    for (int i=0; i<4; i++) {
        makeLinearSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        orb.interpolate(t+test_t[i], pos, vel, isce::core::HERMITE_METHOD);
        compareTriplet(pos, ref_pos, 1.0e-5);
        compareTriplet(vel, ref_vel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

TEST_F(OrbitTest,LinearLegendre){
    /*
     * Test linear orbit.
     */

    isce::core::Orbit orb(1,11);
    double t = 1000.;
    std::vector<double> opos = {0., 0., 0.};
    std::vector<double> ovel = {4000., -1000., 4500.};
    std::vector<double> pos(3), vel(3);

    // Create straight-line orbit with 11 state vectors, each 10 s apart
    for (int i=0; i<11; i++) {
        makeLinearSV(i*10., opos, ovel, pos, vel);
        orb.setStateVector(i, t+(i*10.), pos, vel);
    }

    // Interpolation test times
    double test_t[] = {23.3, 36.7, 54.5, 89.3};
    std::vector<double> ref_pos(3), ref_vel(3);

    for (int i=0; i<4; i++) {
        makeLinearSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        orb.interpolate(t+test_t[i], pos, vel, isce::core::LEGENDRE_METHOD);
        compareTriplet(pos, ref_pos, 1.0e-5);
        compareTriplet(vel, ref_vel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}


void
makeCircularSV(double dt, std::vector<double> &opos, std::vector<double> &ovel,
               std::vector<double> &pos, std::vector<double> &vel) {
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

TEST_F(OrbitTest,CircleSCH) {
    /*
     * Test circular orbit.
     */

    isce::core::Orbit orb(1,11);
    double t = 1000.;
    std::vector<double> opos = {7000000., -4500000., 7800000.};
    std::vector<double> ovel(3,0.), pos(3,0.), vel(3,0.);

    // Create circular orbit with 11 state vectors, each 5 s apart
    for (int i=0; i<11; i++) {
        makeCircularSV(i*5., opos, ovel, pos, vel);
        orb.setStateVector(i, t+(i*5.), pos, vel);
    }

    // Interpolation test times
    double test_t[] = {11.65, 18.35, 27.25, 44.65};
    std::vector<double> ref_pos(3), ref_vel(3);

    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<4; i++) {
        makeCircularSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        orb.interpolate(t+test_t[i], pos, vel, isce::core::SCH_METHOD);
        compareTriplet(ref_pos, pos, 1.0e-5);
        compareTriplet(ref_vel, vel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

TEST_F(OrbitTest,CircleHermite) {
    /*
     * Test circular orbit.
     */

    isce::core::Orbit orb(1,11);
    double t = 1000.;
    std::vector<double> opos = {7000000., -4500000., 7800000.};
    std::vector<double> ovel(3,0.), pos(3,0.), vel(3,0.);

    // Create circular orbit with 11 state vectors, each 5 s apart
    for (int i=0; i<11; i++) {
        makeCircularSV(i*5., opos, ovel, pos, vel);
        orb.setStateVector(i, t+(i*5.), pos, vel);
    }

    // Interpolation test times
    double test_t[] = {11.65, 18.35, 27.25, 44.65};
    std::vector<double> ref_pos(3), ref_vel(3);

    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<4; i++) {
        makeCircularSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        orb.interpolate(t+test_t[i], pos, vel, isce::core::HERMITE_METHOD);
        compareTriplet(ref_pos, pos, 1.0e-5);
        compareTriplet(ref_vel, vel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}


TEST_F(OrbitTest,CircleLegendre) {
    /*
     * Test circular orbit.
     */

    isce::core::Orbit orb(1,11);
    double t = 1000.;
    std::vector<double> opos = {7000000., -4500000., 7800000.};
    std::vector<double> ovel(3,0.), pos(3,0.), vel(3,0.);

    // Create circular orbit with 11 state vectors, each 5 s apart
    for (int i=0; i<11; i++) {
        makeCircularSV(i*5., opos, ovel, pos, vel);
        orb.setStateVector(i, t+(i*5.), pos, vel);
    }

    // Interpolation test times
    double test_t[] = {11.65, 18.35, 27.25, 44.65};
    std::vector<double> ref_pos(3), ref_vel(3);

    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<4; i++) {
        makeCircularSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        orb.interpolate(t+test_t[i], pos, vel, isce::core::LEGENDRE_METHOD);
        compareTriplet(ref_pos, pos, 1.0e-5);
        compareTriplet(ref_vel, vel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}


void makePolynomialSV(double dt, std::vector<double> &xpoly, std::vector<double> &ypoly,
                                 std::vector<double> &zpoly, std::vector<double> &pos,
                                 std::vector<double> &vel) {

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

TEST_F(OrbitTest,PolynomialSCH) {
    /*
     * Test linear orbit.
     */

    isce::core::Orbit orb(1,11);
    double t = 1000.;
    std::vector<double> pos(3), vel(3);

    std::vector<double> xpoly = {-7000000., 5435., -45.0, 7.3};
    std::vector<double> ypoly = {5400000., -4257., 23.0, 3.9, 0.01};
    std::vector<double> zpoly = {0.0, 7000., 11.0};

    // Create straight-line orbit with 11 state vectors, each 10 s apart
    for (int i=0; i<11; i++) {
        makePolynomialSV(i*10., xpoly, ypoly, zpoly, pos, vel);
        orb.setStateVector(i, t+(i*10.), pos, vel);
    }

    // Interpolation test times
    double test_t[] = {23.3, 36.7, 54.5, 89.3};
    std::vector<double> ref_pos(3), ref_vel(3);


    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<4; i++) {
        makePolynomialSV(test_t[i], xpoly, ypoly, zpoly, ref_pos, ref_vel);
        orb.interpolate(t+test_t[i], pos, vel, isce::core::SCH_METHOD);
        compareTriplet(ref_pos, pos, 1.0e-5);
        compareTriplet(ref_vel, vel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

TEST_F(OrbitTest,PolynomialHermite) {
    /*
     * Test linear orbit.
     */

    isce::core::Orbit orb(1,11);
    double t = 1000.;
    std::vector<double> pos(3), vel(3);

    std::vector<double> xpoly = {-7000000., 5435., -45.0, 7.3};
    std::vector<double> ypoly = {5400000., -4257., 23.0, 3.9, 0.01};
    std::vector<double> zpoly = {0.0, 7000., 11.0};

    // Create straight-line orbit with 11 state vectors, each 10 s apart
    for (int i=0; i<11; i++) {
        makePolynomialSV(i*10., xpoly, ypoly, zpoly, pos, vel);
        orb.setStateVector(i, t+(i*10.), pos, vel);
    }

    // Interpolation test times
    double test_t[] = {23.3, 36.7, 54.5, 89.3};
    std::vector<double> ref_pos(3), ref_vel(3);


    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<4; i++) {
        makePolynomialSV(test_t[i], xpoly, ypoly, zpoly, ref_pos, ref_vel);
        orb.interpolate(t+test_t[i], pos, vel, isce::core::HERMITE_METHOD);
        compareTriplet(ref_pos, pos, 1.0e-5);
        compareTriplet(ref_vel, vel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

TEST_F(OrbitTest,PolynomialLegendre) {
    /*
     * Test linear orbit.
     */

    isce::core::Orbit orb(1,11);
    double t = 1000.;
    std::vector<double> pos(3), vel(3);

    std::vector<double> xpoly = {-7000000., 5435., -45.0, 7.3};
    std::vector<double> ypoly = {5400000., -4257., 23.0, 3.9, 0.01};
    std::vector<double> zpoly = {0.0, 7000., 11.0};

    // Create straight-line orbit with 11 state vectors, each 10 s apart
    for (int i=0; i<11; i++) {
        makePolynomialSV(i*10., xpoly, ypoly, zpoly, pos, vel);
        orb.setStateVector(i, t+(i*10.), pos, vel);
    }

    // Interpolation test times
    double test_t[] = {23.3, 36.7, 54.5, 89.3};
    std::vector<double> ref_pos(3), ref_vel(3);


    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<4; i++) {
        makePolynomialSV(test_t[i], xpoly, ypoly, zpoly, ref_pos, ref_vel);
        orb.interpolate(t+test_t[i], pos, vel, isce::core::LEGENDRE_METHOD);
        compareTriplet(ref_pos, pos, 1.0e-5);
        compareTriplet(ref_vel, vel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}


int main(int argc, char **argv) {
    /*
     * Orbit unit-testing script.
     */

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();

}
