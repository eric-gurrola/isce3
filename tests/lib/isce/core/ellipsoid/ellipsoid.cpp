//
// Author: Joshua Cohen
// Copyright 2017
//

#include <cmath>
#include <iostream>
#include <vector>
// needed for searching lines of stdout
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>
#include <iterator>
// end includes for searching lines of stdout
#include <portinfo>
#include <pyre/journal.h>
#include <gtest/gtest.h>
#include <isce/core.h>

using isce::core::Ellipsoid;
using std::cout;
using std::endl;
using std::vector;

//Some commonly used values
Ellipsoid wgs84(6378137.0, 0.0066943799901);
const double a = wgs84.a();
const double b = a * std::sqrt(1.0 - wgs84.e2());

// begin function to split content
std::vector<std::string> splitter(std::string in_pattern, std::string& content) {
    std::vector<std::string> split_content;

    std::regex pattern(in_pattern);
    copy(
        std::sregex_token_iterator(content.begin(), content.end(), pattern, -1),
        std::sregex_token_iterator(),
        std::back_inserter(split_content)
    );
    return split_content;
}

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

// end function to split content


struct EllipsoidTest : public ::testing::Test {
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
                << "Ellipsoid::TearDown sees " << fails << " failures"
                << pyre::journal::endl;
        }
    }
    unsigned fails;
};



#define ellipsoidTest(name,p,q,r,x,y,z)       \
    TEST_F(EllipsoidTest, name) {       \
        isce::core::cartesian_t ref_llh{p,q,r};    \
        isce::core::cartesian_t ref_xyz = {x,y,z};    \
        isce::core::cartesian_t xyz, llh;  \
        llh = ref_llh;                  \
        wgs84.lonLatToXyz(llh, xyz);    \
        EXPECT_NEAR(xyz[0], ref_xyz[0], 1.0e-6);\
        EXPECT_NEAR(xyz[1], ref_xyz[1], 1.0e-6);\
        EXPECT_NEAR(xyz[2], ref_xyz[2], 1.0e-6);\
        xyz = ref_xyz;                  \
        wgs84.xyzToLonLat(xyz, llh);    \
        EXPECT_NEAR(llh[0], ref_llh[0], 1.0e-9);\
        EXPECT_NEAR(llh[1], ref_llh[1], 1.0e-9);\
        EXPECT_NEAR(llh[2], ref_llh[2], 1.0e-6);\
        fails += ::testing::Test::HasFailure();\
    }


ellipsoidTest(Origin, {0.,0.,0.}, {a,0.,0.});

ellipsoidTest(Equator90E, {0.5*M_PI, 0., 0.}, {0.,a,0.});

ellipsoidTest(Equator90W,{-0.5*M_PI,0.,0.}, {0.,-a,0.});

ellipsoidTest(EquatorDateline, {M_PI,0.,0.}, {-a,0.,0.});

ellipsoidTest(NorthPole, {0.,0.5*M_PI,0.}, {0.,0.,b});

ellipsoidTest(SouthPole, {0.,-0.5*M_PI,0.}, {0.,0.,-b});

ellipsoidTest(Point1, {1.134431523585921e+00,-1.180097204507889e+00,7.552767636707697e+03},
        {1030784.925758840050548,2210337.910070449113846,-5881839.839890958741307});

ellipsoidTest(Point2, {-1.988929481271171e+00,-3.218156967477281e-01,4.803829875484664e+02},
        {-2457926.302319798618555,-5531693.075449729338288,-2004656.608288598246872});

ellipsoidTest(Point3, { 3.494775870065641e-01,1.321028021250511e+00, 6.684702668405185e+03},
        {1487474.649522442836314,542090.182021118933335, 6164710.02066358923912});

ellipsoidTest(Point4, { 1.157071150199438e+00,1.539241336260909e+00,  2.075539115269004e+03},
        {81196.748833858233411,   184930.081202651723288, 6355641.007061666809022});

ellipsoidTest(Point5, { 2.903217190227029e+00,3.078348660646868e-02, 1.303664510818545e+03},
          {-6196130.955770593136549,  1505632.319945097202435,195036.854449656093493});

ellipsoidTest(Point6, { 1.404003364812063e+00,9.844570757478284e-01, 1.242074588639294e+03},
    {587386.746772550744936,  3488933.817566382698715, 5290575.784156281501055});

ellipsoidTest(Point7, {1.786087533202875e+00,-1.404475795144668e+00,  3.047509859826395e+03},
        {-226426.343401445570635,  1035421.647801387240179, -6271459.446578867733479});

ellipsoidTest(Point8, { -1.535570572315143e+00,-1.394372375292064e+00, 2.520818495701064e+01},
        {39553.214744714961853, -1122384.858932408038527, -6257455.705907705239952});

ellipsoidTest(Point9, { 2.002720719284312e+00,-6.059309705813630e-01, -7.671870434220574e+01},
        {-2197035.039946643635631,  4766296.481927301734686, -3612087.398071805480868});

ellipsoidTest(Point10, { -2.340221964131008e-01,1.162119493774084e+00,  6.948177664180818e+03},
         {2475217.167525716125965,  -590067.244431337225251, 5836531.74855871964246 });

ellipsoidTest(Point11, {6.067080997777370e-01,-9.030342054807169e-01, 4.244471400804430e+02},
        {3251592.655810729600489,  2256703.30570419318974 ,-4985277.930962197482586});

ellipsoidTest(Point12, { -2.118133740176279e+00,9.812354487540356e-01, 2.921301812478523e+03},
         {-1850635.103680874686688, -3036577.247930331621319,5280569.380736761726439});

ellipsoidTest(Point13, { -2.005023821660764e+00,1.535487121535718e+00, 2.182275729585851e+02},
         { -95048.576977927994449,  -204957.529435861855745, 6352981.530775795690715});

ellipsoidTest(Point14, {2.719747828172381e+00,-1.552548149921413e+00,  4.298201230045657e+03},
        {-106608.855637043248862,    47844.679874961388123, -6359984.3118050172925});

ellipsoidTest(Point15, { -1.498660315787147e+00,1.076512019764726e+00, 8.472554905622580e+02},
         {218676.696484291809611, -3026189.824885316658765, 5592409.664520519785583});

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

int main(int argc, char **argv) {

    // initialize the tests
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

//end of file
