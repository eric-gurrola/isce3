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
#include <complex>
#include <vector>
#include "gtest/gtest.h"

// isce::core
#include "isce/core/Constants.h"
#include "isce/core/Interpolator.h"
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

struct InterpolatorTest : public ::testing::Test {

    // The low resolution indices
    std::vector<double> xindex;
    std::vector<double> yindex;

    // The low resolution data
    isce::core::Matrix<double> M;
    std::vector<double> M_vec;
    // Truth data
    isce::core::Matrix<double> true_values;

    // The low resolution complex data
    isce::core::Matrix<std::complex<double>> M_cpx;
    std::vector<std::complex<double>> M_vec_cpx;
    // Complex truth data
    isce::core::Matrix<std::complex<double>> true_values_cpx;

    double start, delta;

    protected:
        // Constructor
        InterpolatorTest() {

            // Create indices
            xindex = arange(-5.01, 5.01, 0.25);
            yindex = arange(-5.01, 5.01, 0.25);
            size_t nx = xindex.size();
            size_t ny = yindex.size();

            // Allocate the matrices
            M.resize(ny, nx);
            M_cpx.resize(ny, nx);
            // Also allocate the vectors
            M_vec.resize(ny * nx);
            M_vec_cpx.resize(ny * nx);

            // Fill matrix values with function z = sin(x**2 + y**2)
            for (size_t i = 0; i < ny; ++i) {
                for (size_t j = 0; j < nx; ++j) {
                    M(i,j) = std::sin(yindex[i]*yindex[i] + xindex[j]*xindex[j]);
                    M_vec[i*nx + j] = M(i,j);
                    M_cpx(i,j) = std::complex<double>(
                        M(i,j),
                        std::cos(yindex[i]*yindex[i] + xindex[j]*xindex[j])
                    );
                    M_vec_cpx[i*nx + j] = M_cpx(i,j);
                }
            }

            // Read the truth data
            loadInterpData(true_values);

            // Starting coordinate and spacing of data
            start = -5.01;
            delta = 0.25;
        }
};

// Test nearest neighbor interpolation
TEST_F(InterpolatorTest, Nearest) {
    size_t N_pts = true_values.length();
    double error = 0.0;
    // Create interpolator
    isce::core::NearestNeighborInterpolator<double> interp;
    // Loop over test points
    for (size_t i = 0; i < N_pts; ++i) {
        // Unpack location to interpolate
        const double x = (true_values(i,0) - start) / delta;
        const double y = (true_values(i,1) - start) / delta;
        // Perform interpolation
        const double z = interp.interpolate(x, y, M);
        // Manually get nearest neighbor
        const size_t col = (size_t) std::round(x);
        const size_t row = (size_t) std::round(y);
        const double zref = M(row, col);
        // Check
        ASSERT_NEAR(z, zref, 1.0e-8);
    }
}

// Test bilinear interpolation
TEST_F(InterpolatorTest, Bilinear) {
    size_t N_pts = true_values.length();
    double error = 0.0;
    // Create interpolator
    isce::core::BilinearInterpolator<double> interp;
    // Loop over test points
    for (size_t i = 0; i < N_pts; ++i) {
        // Unpack location to interpolate
        const double x = (true_values(i,0) - start) / delta;
        const double y = (true_values(i,1) - start) / delta;
        const double zref = true_values(i,2);
        // Perform interpolation
        double z = interp.interpolate(x, y, M);
        // Check
        ASSERT_NEAR(z, zref, 1.0e-8);
        // Accumulate error
        error += std::pow(z - true_values(i,5), 2);
    }
    ASSERT_TRUE((error / N_pts) < 0.07);
}

// Test bicubic interpolation
// Simply test final sum of square errors
TEST_F(InterpolatorTest, Bicubic) {
    size_t N_pts = true_values.length();
    double error = 0.0;
    // Create interpolator
    isce::core::BicubicInterpolator<double> interp;
    // Loop over test points
    for (size_t i = 0; i < N_pts; ++i) {
        // Unpack location to interpolate
        const double x = (true_values(i,0) - start) / delta;
        const double y = (true_values(i,1) - start) / delta;
        const double zref = true_values(i,5);
        // Perform interpolation
        double z = interp.interpolate(x, y, M);
        // Accumulate error
        error += std::pow(z - zref, 2);
    }
    ASSERT_TRUE((error / N_pts) < 0.058);
}

// Test biquintic spline interpolation
TEST_F(InterpolatorTest, Biquintic) {
    size_t N_pts = true_values.length();
    double error = 0.0;
    // Create interpolator
    isce::core::Spline2dInterpolator<double> interp(6);
    // Loop over test points
    for (size_t i = 0; i < N_pts; ++i) {
        // Unpack location to interpolate
        const double x = (true_values(i,0) - start) / delta;
        const double y = (true_values(i,1) - start) / delta;
        const double zref = true_values(i,5);
        // Perform interpolation
        double z = interp.interpolate(x, y, M);
        // Accumulate error
        error += std::pow(z - zref, 2);
    }
    ASSERT_TRUE((error / N_pts) < 0.058);
}

// Test biquintic spline interpolation
TEST_F(InterpolatorTest, BiquinticVector) {
    size_t N_pts = true_values.length();
    double error = 0.0;
    // Create interpolator
    isce::core::Spline2dInterpolator<double> interp(6);
    // Loop over test points
    for (size_t i = 0; i < N_pts; ++i) {
        // Unpack location to interpolate
        const double x = (true_values(i,0) - start) / delta;
        const double y = (true_values(i,1) - start) / delta;
        const double zref = true_values(i,5);
        // Perform interpolation
        double z = interp.interpolate(x, y, M_vec, M.width());
        // Accumulate error
        error += std::pow(z - zref, 2);
    }
    ASSERT_TRUE((error / N_pts) < 0.058);
}

// Test sinc interpolation
TEST_F(InterpolatorTest, Sinc2d) {
    double error = 0.0;
    size_t N_pts = 0;
    // Create interpolator (exercise flexible interpolator creation)
    isce::core::Interpolator<double> * interp = isce::core::createInterpolator<double>(
        isce::core::SINC_METHOD, 0, 8, 8192
    );
    // Loop over test points
    for (size_t i = 0; i < true_values.length(); ++i) {
        // Unpack location to interpolate
        const double x = (true_values(i,0) - start) / delta;
        const double y = (true_values(i,1) - start) / delta;
        // Skip for invalid sinc windows
        if ((x < 3) || (y < 3) || (x > M.width() -5 ) || (y > M.length() - 5))
            continue;
        const double zref = true_values(i,5);
        // Perform interpolation
        const double z = interp->interpolate(x, y, M);
        // Accumulate error
        error += std::pow(z - zref, 2);
        N_pts += 1;
    }
    ASSERT_TRUE((error / N_pts) < 0.0003);
    // Clean up
    delete interp;
}

// Test nearest neighbor interpolation
TEST_F(InterpolatorTest, NearestComplex) {
    size_t N_pts = true_values.length();
    double error = 0.0;
    // Create interpolator
    isce::core::NearestNeighborInterpolator<std::complex<double>> interp;
    // Loop over test points
    for (size_t i = 0; i < N_pts; ++i) {
        // Unpack location to interpolate
        const double x = (true_values(i,0) - start) / delta;
        const double y = (true_values(i,1) - start) / delta;
        // Perform interpolation
        const std::complex<double> z = interp.interpolate(x, y, M_cpx);
        // Manually get nearest neighbor
        const size_t col = (size_t) std::round(x);
        const size_t row = (size_t) std::round(y);
        const std::complex<double> zref = M_cpx(row, col);
        // Check components
        ASSERT_NEAR(z.real(), zref.real(), 1.0e-8);
        ASSERT_NEAR(z.imag(), zref.imag(), 1.0e-8);
    }
}

// Test bilinear interpolation
TEST_F(InterpolatorTest, BilinearComplex) {
    size_t N_pts = true_values.length();
    double real_error = 0.0;
    double imag_error = 0.0;
    // Create interpolator
    isce::core::BilinearInterpolator<std::complex<double>> interp;
    // Loop over test points
    for (size_t i = 0; i < N_pts; ++i) {
        // Unpack location to interpolate
        const double x = (true_values(i,0) - start) / delta;
        const double y = (true_values(i,1) - start) / delta;
        // Perform interpolation
        const std::complex<double> z = interp.interpolate(x, y, M_cpx);
        // Compute reference
        const std::complex<double> zref = std::complex<double>(
            std::sin(true_values(i,0)*true_values(i,0) + true_values(i,1)*true_values(i,1)),
            std::cos(true_values(i,0)*true_values(i,0) + true_values(i,1)*true_values(i,1))
        );
        // Accumulate error
        real_error += std::pow(z.real() - zref.real(), 2);
        imag_error += std::pow(z.imag() - zref.imag(), 2);
    }
    ASSERT_TRUE((real_error / N_pts) < 0.07);    
    ASSERT_TRUE((imag_error / N_pts) < 0.07);
}

// Test bicubic interpolation
// Simply test final sum of square errors
TEST_F(InterpolatorTest, BicubicComplex) {
    size_t N_pts = true_values.length();
    double real_error = 0.0;
    double imag_error = 0.0;
    // Create interpolator
    isce::core::BicubicInterpolator<std::complex<double>> interp;
    // Loop over test points
    for (size_t i = 0; i < N_pts; ++i) {
        // Unpack location to interpolate
        const double x = (true_values(i,0) - start) / delta;
        const double y = (true_values(i,1) - start) / delta;
        // Perform interpolation
        const std::complex<double> z = interp.interpolate(x, y, M_cpx);
        // Compute reference
        const std::complex<double> zref = std::complex<double>(
            std::sin(true_values(i,0)*true_values(i,0) + true_values(i,1)*true_values(i,1)),
            std::cos(true_values(i,0)*true_values(i,0) + true_values(i,1)*true_values(i,1))
        );
        // Accumulate error
        real_error += std::pow(z.real() - zref.real(), 2);
        imag_error += std::pow(z.imag() - zref.imag(), 2);
    }
    ASSERT_TRUE((real_error / N_pts) < 0.058);
    ASSERT_TRUE((imag_error / N_pts) < 0.058);
}

// Test biquintic spline interpolation
TEST_F(InterpolatorTest, BiquinticComplex) {
    size_t N_pts = true_values.length();
    double real_error = 0.0;
    double imag_error = 0.0;
    // Create interpolator
    isce::core::Spline2dInterpolator<std::complex<double>> interp(6);
    // Loop over test points
    for (size_t i = 0; i < N_pts; ++i) {
        // Unpack location to interpolate
        const double x = (true_values(i,0) - start) / delta;
        const double y = (true_values(i,1) - start) / delta;
        // Perform interpolation
        const std::complex<double> z = interp.interpolate(x, y, M_cpx);
        // Compute reference
        const std::complex<double> zref = std::complex<double>(
            std::sin(true_values(i,0)*true_values(i,0) + true_values(i,1)*true_values(i,1)),
            std::cos(true_values(i,0)*true_values(i,0) + true_values(i,1)*true_values(i,1))
        );
        // Accumulate error
        real_error += std::pow(z.real() - zref.real(), 2);
        imag_error += std::pow(z.imag() - zref.imag(), 2);
    }
    ASSERT_TRUE((real_error / N_pts) < 0.058);
    ASSERT_TRUE((imag_error / N_pts) < 0.058);
}


// Test sinc interpolation
TEST_F(InterpolatorTest, Sinc2dComplex) {
    double real_error = 0.0;
    double imag_error = 0.0;
    size_t N_pts = 0;
    // Create interpolator (exercise flexible interpolator creation)
    isce::core::Interpolator<std::complex<double>> * interp = 
        isce::core::createInterpolator<std::complex<double>>(
            isce::core::SINC_METHOD, 0, 8, 8192
        );
    // Loop over test points
    for (size_t i = 0; i < true_values.length(); ++i) {
        // Unpack location to interpolate
        const double x = (true_values(i,0) - start) / delta;
        const double y = (true_values(i,1) - start) / delta;
        // Skip for invalid sinc windows
        if ((x < 3) || (y < 3) || (x > M.width() -5 ) || (y > M.length() - 5))
            continue;
        // Perform interpolation
        const std::complex<double> z = interp->interpolate(x, y, M_cpx);
        // Compute reference
        const std::complex<double> zref = std::complex<double>(
            std::sin(true_values(i,0)*true_values(i,0) + true_values(i,1)*true_values(i,1)),
            std::cos(true_values(i,0)*true_values(i,0) + true_values(i,1)*true_values(i,1))
        );
        // Accumulate error
        real_error += std::pow(z.real() - zref.real(), 2);
        imag_error += std::pow(z.imag() - zref.imag(), 2);
        N_pts += 1;
    }
    ASSERT_TRUE((real_error / N_pts) < 0.0004);
    ASSERT_TRUE((imag_error / N_pts) < 0.0004);
    // Clean up
    delete interp;
}

TEST_F(InterpolatorTest, SimpleRampTest) {

    // This test creates a matrix of data whose values form a 
    // linear ramp in x direction (Values are x indices). 
    // For a given interpolation point x,y the interpolated value is compared aginst x.
    // bilinear, bicubic and biquintic methods are tested.

    size_t length = 100;
    size_t width = 100;

    isce::core::Matrix<double> M(length, width);

    for (size_t line = 20; line < 60; ++line) {
        for (size_t pixel = 20; pixel < 60; ++pixel) {
            M(line, pixel) = pixel;
        }
    }

    double interp_val = 0.0;
    double interp_val2 = 0.0;

    double err = 0.0; //interp_val - x;
    double err2 = 0.0;
      
    isce::core::dataInterpMethod method = isce::core::BILINEAR_METHOD;
    isce::core::Interpolator<double> * interp = nullptr;
    interp = isce::core::createInterpolator<double>(method);

    isce::core::Interpolator<double> * interp2 = nullptr;
    interp2 = isce::core::createInterpolator<double>(isce::core::BICUBIC_METHOD);

    isce::core::Interpolator<double> * interp3 = nullptr;
    interp3 = isce::core::createInterpolator<double>(isce::core::BIQUINTIC_METHOD);

    double y = 40.0;
    double x = 30.0;
    double p = 0.0;
    double maxErr = 0.0;
    double maxErr2 = 0.0;
    double maxErr3 = 0.0;
    while(x<35){

        x += 0.1;
        p = interp->interpolate(x, y, M);
        err = p - x;
        
        if (std::abs(err) > maxErr)
            maxErr = std::abs(err);

        p = interp2->interpolate(x, y, M);
        err = p - x;

        if (std::abs(err) > maxErr2)
            maxErr2 = std::abs(err);

        p = interp3->interpolate(x, y, M);
        err = p - x;

        if (std::abs(err) > maxErr3)
            maxErr3 = std::abs(err);
    }

    // Check the BILINEAR_METHOD
    ASSERT_LT(maxErr, 1.0e-8);

    // Check the BICUBIC_METHOD
    ASSERT_LT(maxErr2, 1.0e-8);

    // Check the BIQUINTIC_METHOD
    ASSERT_LT(maxErr3, 1.0e-8);

    delete interp;
    delete interp2;
    delete interp3;
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void loadInterpData(Matrix<double> & M) {
    /*
    Load ground truth interpolation data. The test data is the function:

    z = sqrt(x^2 + y^2)

    The columns of the data are:
    x_index    y_index    bilinear_interp    bicubic_interp    5thorder_spline    truth
    */

    // Open file for reading
    std::ifstream fid("data.txt");
    // Check if file open was successful
    if (fid.fail()) {
        std::cout << "Error: Failed to open data file for interpolator test." << std::endl;
    }

    std::vector<double> xvec, yvec, zlinear_vec, zcubic_vec, zquintic_vec, ztrue_vec;

    // Loop over interpolation data
    while (fid) {

        // Parse line
        std::string str;
        std::stringstream stream;
        double x, y, z_linear, z_cubic, z_quintic, z_true;

        std::getline(fid, str);
        if (str.length() < 1)
            break;
        stream << str;
        stream >> x >> y >> z_linear >> z_cubic >> z_quintic >> z_true;

        // Add data to orbit
        xvec.push_back(x);
        yvec.push_back(y);
        zlinear_vec.push_back(z_linear);
        zcubic_vec.push_back(z_cubic);
        zquintic_vec.push_back(z_quintic);
        ztrue_vec.push_back(z_true);
    }

    // Close the file
    fid.close();

    // Fill the matrix
    const size_t N = xvec.size();
    M.resize(N, 6);
    for (size_t i = 0; i < N; ++i) {
        M(i,0) = xvec[i];
        M(i,1) = yvec[i];
        M(i,2) = zlinear_vec[i];
        M(i,3) = zcubic_vec[i];
        M(i,4) = zquintic_vec[i];
        M(i,5) = ztrue_vec[i];
    }
}

// end of file
