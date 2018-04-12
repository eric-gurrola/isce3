//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Bryan V. Riel
// Copyright 2018
//

#include <cstdio>
#include <cmath>
#include <vector>
#include <string>
#include <regex>
#include <algorithm>
#include <iterator>
#include <gtest/gtest.h>
#include <portinfo>
#include <pyre/journal.h>
#include <isce/core.h>

std::vector<std::string> string_splitter(const std::string split_token, const std::string& content) {
    // function to split string content on a given token
    std::vector<std::string> split_content;
    for( auto x : split_content ){ std::cout << x << std::endl; }
    std::regex pattern(split_token);
    copy(
        std::sregex_token_iterator(content.begin(), content.end(), pattern, -1),
        std::sregex_token_iterator(),

 std::back_inserter(split_content)
    );
    return split_content;
}

std::vector<std::string> string_to_lines(const std::string& content) {
    // function to split content on newlines
    std::vector<std::string> split_content;

    const std::string pattern = R"(\n)";
    split_content = string_splitter(pattern, content);
    return split_content;
}


struct EulerTest : public ::testing::Test {
    virtual void SetUp() {
        fails = 0;
    }
    virtual void TearDown() {
        if (fails > 0) {
            // create testerror channel
            pyre::journal::error_t channel("tests.lib.core.fails");
            // complain
            channel
                << pyre::journal::at(__HERE__)
                << "euler::TearDown sees " << fails << " failures"
                << pyre::journal::endl;
        }
    }
    unsigned fails;


    typedef isce::core::EulerAngles EulerAngles;

    double yaw, pitch, roll, tol;
    std::vector<std::vector<double>> R_ypr_ref, R_rpy_ref;
    EulerAngles attitude;

    protected:

        EulerTest() {

            // Set the attitude angles
            yaw = 0.1;
            pitch = 0.05;
            roll = -0.1;

            // Instantate attitude object
            attitude = EulerAngles(yaw, pitch, roll);

            // Define the reference rotation matrix (YPR)
            R_ypr_ref = {
                {0.993760669166, -0.104299329454, 0.039514330251},
                {0.099708650872, 0.989535160981, 0.104299329454},
                {-0.049979169271, -0.099708650872, 0.993760669166}
            };

            // Define the reference rotation matrix (RPY)
            R_rpy_ref = {
                {0.993760669166, -0.099708650872, 0.049979169271},
                {0.094370001341, 0.990531416861, 0.099708650872},
                {-0.059447752410, -0.094370001341, 0.993760669166}
            };

            // Set tolerance
            tol = 1.0e-9;
        }

        ~EulerTest() {
            R_ypr_ref.clear();
            R_rpy_ref.clear();
        }
};

TEST_F(EulerTest, CheckYPR) {
    std::vector<std::vector<double>> R_ypr = attitude.rotmat("ypr");
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            ASSERT_NEAR(R_ypr_ref[i][j], R_ypr[i][j], tol);
        }
    }
    fails += ::testing::Test::HasFailure();
}

TEST_F(EulerTest, CheckRPY) {
    std::vector<std::vector<double>> R_rpy = attitude.rotmat("rpy");
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            ASSERT_NEAR(R_rpy_ref[i][j], R_rpy[i][j], tol);
        }
    }
    fails += ::testing::Test::HasFailure();
}


void report_gtest_errors( const std::string gtest_std_out ) {
    // split gtest_std_out on new lines
    const std::vector<std::string> lines = string_to_lines(gtest_std_out);
    for ( auto x : lines ){ std::cout << x << std::endl; }
    // find the lines containing error messages
    for ( unsigned int i = 0; i < lines.size(); i++ ) {
        if ( lines[i].find("error")    != std::string::npos ||
             lines[i].find("firewall") != std::string::npos   ){
            std::cout << lines[i] << std::endl;
            std::cout << lines[i+1] << std::endl;
            std::cout << lines[i+2] << std::endl;
        }
    }
}

int main(int argc, char * argv[]) {

    // innitialize the tests
    testing::InitGoogleTest(&argc, argv);

    // capture the std output
    testing::internal::CaptureStdout();

    // run the tests
    int run_out = RUN_ALL_TESTS();

    // if there are any test failures report them
    if( run_out != 0 ) {
        // get the capturedStdOut
        const std::string stdoutput = testing::internal::GetCapturedStdout();
        // send to error reporter for handling
        report_gtest_errors(stdoutput);
    }

    // return status of test runs
    return run_out;
}

// end of file
