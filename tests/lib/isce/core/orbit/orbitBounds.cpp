//
// Author: Joshua Cohen
// Copyright 2017
//

#include <cmath>
#include <iostream>
#include <vector>
#include <pyre/journal.h>
#include "gtest/gtest.h"
#include "isce/core.h"


struct OrbitTest : public ::testing::Test {
    virtual void SetUp() {
        fails = 0;
    }
    virtual void TearDown() {
        if (fails > 0) {
            // create testerror channel
            pyre::journal::firewall_t channel("tests.lib.core.fails");
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


void makeLinearSV(double dt, std::vector<double> &opos, std::vector<double> &ovel,
                  std::vector<double> &pos, std::vector<double> &vel) {
    pos = {opos[0] + (dt * ovel[0]), opos[1] + (dt * ovel[1]), opos[2] + (dt * ovel[2])};
    vel = ovel;
}

TEST_F(OrbitTest, OutOfBoundsSCH) {
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
    double test_t[] = {-23.0, -1.0, 101.0, 112.0};
    std::vector<double> ref_pos(3), ref_vel(3);


    for (int i=0; i<4; i++) {
        int stat = orb.interpolate(t+test_t[i], pos, vel, isce::core::SCH_METHOD);
        EXPECT_EQ(stat,1);
    }

    fails += ::testing::Test::HasFailure();
}

TEST_F(OrbitTest, OutOfBoundsHermite) {
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
    double test_t[] = {-23.0, -1.0, 101.0, 112.0};
    std::vector<double> ref_pos(3), ref_vel(3);


    for (int i=0; i<4; i++) {
        int stat = orb.interpolate(t+test_t[i], pos, vel, isce::core::HERMITE_METHOD);
        EXPECT_EQ(stat,1);
    }

    fails += ::testing::Test::HasFailure();
}

TEST_F(OrbitTest, OutOfBoundsLegendre) {
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
    double test_t[] = {-23.0, -1.0, 101.0, 112.0};
    std::vector<double> ref_pos(3), ref_vel(3);


    for (int i=0; i<4; i++) {
        int stat = orb.interpolate(t+test_t[i], pos, vel, isce::core::LEGENDRE_METHOD);
        EXPECT_EQ(stat,1);
    }

    fails += ::testing::Test::HasFailure();
}



TEST_F(OrbitTest, EdgesSCH){
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
    double test_t[] = {0.0, 100.0};
    std::vector<double> ref_pos(3), ref_vel(3);

    for (int i=0; i<2; i++) {
        makeLinearSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        int stat = orb.interpolate(t+test_t[i], pos, vel, isce::core::SCH_METHOD);
        EXPECT_EQ(stat, 0);
        compareTriplet(pos, ref_pos, 1.0e-5);
        compareTriplet(vel, ref_vel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

TEST_F(OrbitTest, EdgesHermite){
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
    double test_t[] = {0.0, 100.0};
    std::vector<double> ref_pos(3), ref_vel(3);

    for (int i=0; i<2; i++) {
        makeLinearSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        int stat = orb.interpolate(t+test_t[i], pos, vel, isce::core::HERMITE_METHOD);
        EXPECT_EQ(stat, 0);
        compareTriplet(pos, ref_pos, 1.0e-5);
        compareTriplet(vel, ref_vel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}


TEST_F(OrbitTest, EdgesLegendre){
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
    double test_t[] = {0.0, 100.0};
    std::vector<double> ref_pos(3), ref_vel(3);

    for (int i=0; i<2; i++) {
        makeLinearSV(test_t[i], opos, ovel, ref_pos, ref_vel);
        int stat = orb.interpolate(t+test_t[i], pos, vel, isce::core::LEGENDRE_METHOD);
        EXPECT_EQ(stat, 0);
        compareTriplet(pos, ref_pos, 1.0e-5);
        compareTriplet(vel, ref_vel, 1.0e-6);
    }

    fails += ::testing::Test::HasFailure();
}

int main(int argc, char **argv) {
    /*
     * Orbit unit-testing script.
     */

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();

    return 0;
}
