//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Bryan V. Riel
// Copyright 2018
//

#include <iostream>
#include <fstream>
#include <gtest/gtest.h>
#include <portinfo>
#include <pyre/journal.h>
#include <isce/core.h>


struct SerializeTest : public ::testing::Test {
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
                << "Serialize::TearDown sees " << fails << " failures"
                << pyre::journal::endl;
        }
    }
    unsigned fails;
};

TEST(EllipsoidTest, CheckArchive) {
    // Make an ellipsoid
    isce::core::Ellipsoid ellipsoid;

    // create test error channel
    pyre::journal::error_t channel("tests.lib.core.fails");

    // Open XML file
    std::ifstream xmlfid("archive.xml", std::ios::in);
    // Check if file was open successfully
    if (xmlfid.fail()) {
        // complain
        channel
            << "Error: failed to open archive.xml file."
            << pyre::journal::endl;
    }

    // Create cereal archive and load
    {
    cereal::XMLInputArchive archive(xmlfid);
    archive(cereal::make_nvp("Ellipsoid", ellipsoid));
    }

    // Check values
    ASSERT_NEAR(ellipsoid.a, 6378137.0, 1.0e-9);
    ASSERT_NEAR(ellipsoid.e2, 0.0066943799, 1.0e-9);

}

int main(int argc, char * argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// end of file
