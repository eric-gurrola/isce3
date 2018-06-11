//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Bryan V. Riel
// Copyright 2017-2018
//

#include <cmath>
#include <cstdio>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <valarray>
#include <vector>
#include "gtest/gtest.h"

// isce::core
#include "isce/core/Constants.h"
#include "isce/core/Interpolator.h"
#include "isce/core/RectBSpline.h"
using isce::core::Matrix;

void loadInterpData(Matrix<double> &);

// Function to return a Python style arange vector
std::vector<double> arange(double low, double high, double increment) {
    // Instantiate the vector
    std::vector<double> data;
    // Set the first value
    double current = low;
    // Loop over the increments and add to vector
    while (current < high) {
        data.push_back(current);
        current += increment;
    }
    // done
    return data;
}

// Function to compute median of a valarray
template <typename T>
T median(std::valarray<T> v) {
    T value;
    size_t n = v.size();
    size_t nmid = v.size() / 2;
    std::nth_element(std::begin(v), std::begin(v) + nmid, std::end(v));
    value = v[nmid];
    if (n % 2 == 1) {
        return value;
    }
    std::nth_element(std::begin(v), std::begin(v) + nmid - 1, std::end(v));
    value = 0.5 * (value + v[nmid - 1]);
    return value;
}

// Function to compute median absolute deviation of a valarray
template <typename T>
T medianAbsoluteDeviation(std::valarray<T> v) {
    // Compute median
    T medianValue = median(v);
    // Subtract median and take absolute value
    for (size_t i = 0; i < v.size(); ++i) {
        v[i] = std::abs(v[i] - medianValue);
    }
    // Compute median of normalized values
    T mad = median(v);
    return mad;
}

struct InterpolatorTest : public ::testing::Test {

    // The low resolution data
    isce::core::Matrix<double> M;
    // The low resolution indices
    std::valarray<double> xindex;
    std::valarray<double> yindex;
    // Spline interpolators
    isce::core::RectBSpline cubicSpline;
    isce::core::RectBSpline quinticSpline;
    // Truth data
    isce::core::Matrix<double> true_values;

    double start, delta;

    protected:
        // Constructor
        InterpolatorTest() {

            // Create indices
            auto xvec = arange(-6.01, 6.01, 0.25);
            auto yvec = arange(-6.01, 6.01, 0.25);
            size_t nx = xvec.size();
            size_t ny = yvec.size();

            // Copy to valarrays
            xindex.resize(nx);
            yindex.resize(ny);
            for (size_t i = 0; i < nx; ++i)
                xindex[i] = xvec[i];
            for (size_t i = 0; i < ny; ++i)
                yindex[i] = yvec[i];

            // Allocate the image data for interpolation
            M.resize(ny, nx);
            std::valarray<double> M_array(ny*nx);

            // Fill matrix values with function z = sin(x**2 + y**2)
            for (size_t i = 0; i < ny; ++i) {
                for (size_t j = 0; j < nx; ++j) {
                    M(i,j) = std::sin(yindex[i]*yindex[i] + xindex[j]*xindex[j]);
                    M_array[i*nx+j] = M(i,j);
                }
            }

            // Read the truth data
            loadInterpData(true_values);

            // Initialize spline interpolators
            cubicSpline.initialize(yindex, xindex, M_array, 3, 3, 0.0);
            quinticSpline.initialize(yindex, xindex, M_array, 5, 5, 0.0);

            // Starting coordinate and spacing of data
            start = -6.01;
            delta = 0.25;
        }
};

// Test bilinear interpolation
TEST_F(InterpolatorTest, Bilinear) {
    size_t N_pts = true_values.length();
    std::valarray<double> errors(N_pts);
    for (size_t i = 0; i < N_pts; ++i) {
        // Unpack location to interpolate
        const double x = (true_values(i,0) - start) / delta;
        const double y = (true_values(i,1) - start) / delta;
        const double zref = std::sin(true_values(i,0)*true_values(i,0) +
                                     true_values(i,1)*true_values(i,1));
        // Perform interpolation
        double z = isce::core::Interpolator::bilinear(x, y, M);
        // Accumulate error
        errors[i] = z - zref;
    }
    ASSERT_TRUE(medianAbsoluteDeviation(errors) < 0.11);
}

// Test bicubic interpolation
// Simply test final sum of square errors
TEST_F(InterpolatorTest, Bicubic) {
    size_t N_pts = true_values.length();
    std::valarray<double> errors(N_pts);
    double error = 0.0;
    for (size_t i = 0; i < N_pts; ++i) {
        // Unpack location to interpolate
        const double x = (true_values(i,0) - start) / delta;
        const double y = (true_values(i,1) - start) / delta;
        const double zref = std::sin(true_values(i,0)*true_values(i,0) +
                                     true_values(i,1)*true_values(i,1));
        // Perform interpolation
        double z = isce::core::Interpolator::bicubic(x, y, M);
        // Accumulate error
        errors[i] = z - zref;
    }
    ASSERT_TRUE(medianAbsoluteDeviation(errors) < 0.04);
}

// Test biquintic spline interpolation
TEST_F(InterpolatorTest, Biquintic) {
    size_t N_pts = true_values.length();
    std::valarray<double> errors(N_pts);
    for (size_t i = 0; i < N_pts; ++i) {
        // Unpack location to interpolate
        const double x = (true_values(i,0) - start) / delta;
        const double y = (true_values(i,1) - start) / delta;
        const double zref = std::sin(true_values(i,0)*true_values(i,0) + 
                                     true_values(i,1)*true_values(i,1));
        // Perform interpolation
        double z = isce::core::Interpolator::interp_2d_spline(6, M, x, y);
        // Accumulate error
        errors[i] = zref - z;
    }
    ASSERT_TRUE(medianAbsoluteDeviation(errors) < 0.02);
}

// Test fitpack spline interpolation
TEST_F(InterpolatorTest, SplineBicubic) {
    size_t N_pts = true_values.length();
    std::valarray<double> errors(N_pts);
    for (size_t i = 0; i < N_pts; ++i) {
        // Unpack location to interpolate
        const double x = true_values(i,0);
        const double y = true_values(i,1);
        const double zref = std::sin(x*x + y*y);
        // Perform interpolation
        double z = cubicSpline.eval(y, x);
        // Accumulate error
        errors[i] = z - zref;
    }
    ASSERT_TRUE(medianAbsoluteDeviation(errors) < 0.006);
}

// Test fitpack spline interpolation
TEST_F(InterpolatorTest, SplineBiquintic) {
    size_t N_pts = true_values.length();
    std::valarray<double> errors(N_pts);
    for (size_t i = 0; i < N_pts; ++i) {
        // Unpack location to interpolate
        const double x = true_values(i,0);
        const double y = true_values(i,1);
        const double zref = std::sin(x*x + y*y);
        // Perform interpolation
        double z = quinticSpline.eval(y, x);
        // Accumulate error
        errors[i] = z - zref;
    }
    ASSERT_TRUE(medianAbsoluteDeviation(errors) < 0.0007);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void loadInterpData(Matrix<double> & M) {
    /*
    Load ground truth interpolation data. The test data is the function:

    z = sin(x^2 + y^2)

    The columns of the data are:
    x_index    y_index    bilinear_interp    bicubic_interp    5thorder_spline    truth
    */

    // Open file for reading
    std::ifstream fid("data.txt");
    // Check if file open was successful
    if (fid.fail()) {
        std::cout << "Error: Failed to open data file for interpolator test." << std::endl;
    }

    std::vector<double> xvec, yvec;

    // Loop over interpolation data
    while (fid) {

        // Parse line
        std::string str;
        std::stringstream stream;
        double x, y;

        std::getline(fid, str);
        if (str.length() < 1)
            break;
        stream << str;
        stream >> x >> y;

        // Add data to orbit
        xvec.push_back(x);
        yvec.push_back(y);
    }

    // Close the file
    fid.close();

    // Fill the matrix
    const size_t N = xvec.size();
    M.resize(N, 2);
    for (size_t i = 0; i < N; ++i) {
        M(i,0) = xvec[i];
        M(i,1) = yvec[i];
    }
}

// end of file
