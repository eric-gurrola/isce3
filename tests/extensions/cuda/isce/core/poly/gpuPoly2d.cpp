//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Liang Yu
// Copyright 2018
//


#include <iostream>
#include <cmath>
#include <vector>
#include "isce/core/Constants.h"
#include "isce/core/Poly2d.h"
#include "isce/cuda/core/gpuPoly2d.h"
#include "gtest/gtest.h"
#include <stdio.h>

using isce::cuda::core::gpuPoly2d;
using isce::core::Poly2d;

struct gpuPoly2dTest : public ::testing::Test {
    virtual void SetUp() {
        fails = 0;
    }
    virtual void TearDown() {
        if (fails > 0) {
            std::cerr << "Poly2d::TearDown sees failures" << std::endl;
        }
    }
    unsigned fails;
};


TEST_F(gpuPoly2dTest, Constant) {

    const double refval = 10.0;

    // Interpolate N values in x and y
    for (size_t i = 1; i < 5; ++i)
    {
        //Mean and norm should not matter
        Poly2d poly(0, 0, i*1.0, 0, i*i*1.0, 1.0);
        poly.setCoeff(0, 0, refval);
        gpuPoly2d gpu_poly(poly);

        double value = gpu_poly.eval_h(0.0, i*1.0);
        EXPECT_DOUBLE_EQ(value, refval);
    }
    
    fails += ::testing::Test::HasFailure();
}


TEST_F(gpuPoly2dTest, MeanShift)
{
    //Use identity polynomial for testing
    Poly2d refpoly(2, 0, 0.0, 0.0, 1.0, 1.0);
    refpoly.setCoeff(0, 0, 0.0);
    refpoly.setCoeff(0, 1, 1.0);
    refpoly.setCoeff(0, 2, 0.0);
    gpuPoly2d ref_gpu_poly(refpoly);

    for(size_t i=0; i<5; i++)
    {
        Poly2d newpoly(refpoly);
        newpoly.rangeMean = 0.5 * i * i;
        gpuPoly2d new_gpu_poly(newpoly);

        double refval = ref_gpu_poly.eval_h(0.0, 2.0 * i);
        double newval = new_gpu_poly.eval_h(0.0, 2.0 * i + 0.5 * i * i);
        EXPECT_DOUBLE_EQ(newval, refval);
    }

    fails += ::testing::Test::HasFailure();
}


TEST_F(gpuPoly2dTest, NormShift)
{
    //Use square polynomial for testing
    Poly2d refpoly(2, 0, 0.0, 0.0, 1.0, 1.0);
    refpoly.setCoeff(0, 0, 0.0);
    refpoly.setCoeff(0, 1, 0.0);
    refpoly.setCoeff(0, 2, 1.0);
    gpuPoly2d gpu_refpoly(refpoly);

    for(size_t i=1; i<6; i++)
    {
        Poly2d newpoly(refpoly);
        newpoly.rangeNorm = i * i * 1.0;
        gpuPoly2d gpu_newpoly(newpoly);

        double refval = gpu_refpoly.eval_h(0.0, 2.5);
        double newval = gpu_newpoly.eval_h(0.0, 2.5 * i * i);

        EXPECT_DOUBLE_EQ(newval, refval);
    }

    fails += ::testing::Test::HasFailure();
}



int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

// end of file
