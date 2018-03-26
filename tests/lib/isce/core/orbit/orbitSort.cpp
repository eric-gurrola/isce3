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


#define compareTriplet(a,b)\
    EXPECT_EQ(a[0], b[0]); \
    EXPECT_EQ(a[1], b[1]); \
    EXPECT_EQ(a[2], b[2]);


void makeLinearSV(double dt, std::vector<double> &opos, std::vector<double> &ovel,
                  std::vector<double> &pos, std::vector<double> &vel) {
    pos = {opos[0] + (dt * ovel[0]), opos[1] + (dt * ovel[1]), opos[2] + (dt * ovel[2])};
    vel = ovel;
}

TEST_F(OrbitTest,Reverse) {
    /*
     * Test linear orbit.
     */

    isce::core::Orbit orb(1,11);
    double t = 1000.;
    double t1;
    std::vector<double> opos = {0., 0., 0.};
    std::vector<double> ovel = {4000., -1000., 4500.};
    std::vector<double> pos(3), vel(3);

    // Create straight-line orbit with 11 state vectors, each 10 s apart
    for (int i=0; i<11; i++) {
        makeLinearSV(i*10., opos, ovel, pos, vel);
        orb.setStateVector(i, t+(i*10.), pos, vel);
    }


    isce::core::Orbit newOrb(1,0);

    for(int i=10; i>=0; i--)
    {
        orb.getStateVector(i, t, pos, vel);
        newOrb.addStateVector(t,pos,vel);
    }

    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<10; i++) {
        orb.getStateVector(i, t, pos, vel);
        newOrb.getStateVector(i, t1, opos, ovel);

        EXPECT_EQ(t1,t);
        compareTriplet(opos, pos);
        compareTriplet(ovel, vel);
    }

    fails += ::testing::Test::HasFailure();

}

TEST_F(OrbitTest,OutOfOrder) {
    /*
     * Test linear orbit.
     */

    isce::core::Orbit orb(1,11);
    double t = 1000.;
    double t1;
    std::vector<double> opos = {0., 0., 0.};
    std::vector<double> ovel = {4000., -1000., 4500.};
    std::vector<double> pos(3), vel(3);

    // Create straight-line orbit with 11 state vectors, each 10 s apart
    for (int i=0; i<11; i++) {
        makeLinearSV(i*10., opos, ovel, pos, vel);
        orb.setStateVector(i, t+(i*10.), pos, vel);
    }


    isce::core::Orbit newOrb(1,0);

    for(int i=10; i>=0; i-=2)
    {
        orb.getStateVector(i, t, pos, vel);
        newOrb.addStateVector(t,pos,vel);
    }


    for(int i=1; i<10; i+=2)
    {
        orb.getStateVector(i, t, pos, vel);
        newOrb.addStateVector(t, pos, vel);
    }


    // Test each interpolation time against SCH, Hermite, and Legendre interpolation methods
    for (int i=0; i<10; i++) {
        orb.getStateVector(i, t, pos, vel);
        newOrb.getStateVector(i, t1, opos, ovel);

        EXPECT_EQ(t1, t);
        compareTriplet(pos, opos);
        compareTriplet(vel, ovel);
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
