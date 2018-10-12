//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Marco Lavalle, Bryan Riel
// Copyright 2018
//

#include <iostream>
#include <complex>
#include <string>
#include <sstream>
#include <fstream>
#include <gtest/gtest.h>

// isce::core
#include "isce/core/Constants.h"
#include "isce/core/Serialization.h"

// isce::io
#include "isce/io/Raster.h"

// isce::product
#include "isce/product/ImageMode.h"

// isce::geometry
#include "isce/geometry/Serialization.h"
#include "isce/geometry/Baseline.h"

// Declaration for utility function to read metadata stream from VRT
std::stringstream streamFromVRT(const char * filename, int bandNum=1);

TEST(BaselineTest, RunBaseline) {

    // Instantiate isce::core objects
    isce::core::Poly2d dopplerMaster;
    isce::core::Orbit orbitMaster;
    isce::core::Ellipsoid ellipsoidMaster;
    isce::product::ImageMode modeMaster;

    // Instantiate isce::core objects
    isce::core::Poly2d dopplerSlave;
    isce::core::Orbit orbitSlave;
    isce::core::Ellipsoid ellipsoidSlave;
    isce::product::ImageMode modeSlave;

    // Load metadata
    std::stringstream metastreamMaster = streamFromVRT("/home/mlavalle/dat/uavsar/lope2/lopenp_14043_16015_001_160308/lope.vrt");
    {
    cereal::XMLInputArchive archive(metastreamMaster);
    archive(cereal::make_nvp("Orbit", orbitMaster),
            cereal::make_nvp("SkewDoppler", dopplerMaster),
            cereal::make_nvp("Ellipsoid", ellipsoidMaster),
            cereal::make_nvp("Radar", modeMaster));
    }

    std::stringstream metastreamSlave = streamFromVRT("/home/mlavalle/dat/uavsar/lope2/fortym_14045_16008_004_160225/forty.vrt");
    {
    cereal::XMLInputArchive archive(metastreamSlave);
    archive(cereal::make_nvp("Orbit", orbitSlave),
            cereal::make_nvp("SkewDoppler", dopplerSlave),
            cereal::make_nvp("Ellipsoid", ellipsoidSlave),
            cereal::make_nvp("Radar", modeSlave));
    }


    // Create geo2rdr isntance
    isce::geometry::Baseline bas(ellipsoidMaster, orbitMaster, modeMaster,
                                 ellipsoidSlave, orbitSlave, modeSlave);

    // Load topo processing parameters to finish configuration
    std::ifstream xmlfid("../../data/topo.xml", std::ios::in);
    {
    cereal::XMLInputArchive archive(xmlfid);
    archive(cereal::make_nvp("Baseline", bas));
    }

    // Open lat raster
    isce::io::Raster topoRaster("../../data/topo.vrt");

    // Run geo2rdr
    bas.computeBaseline(topoRaster, dopplerMaster, dopplerSlave, ".");

}


// Read metadata from a VRT file and return a stringstream object
std::stringstream streamFromVRT(const char * filename, int bandNum) {
    // Register GDAL drivers
    GDALAllRegister();
    // Open the VRT dataset
    GDALDataset * dataset = (GDALDataset *) GDALOpen(filename, GA_ReadOnly);
    if (dataset == NULL) {
        std::cout << "Cannot open dataset " << filename << std::endl;
        exit(1);
    }
    // Read the metadata
    char **metadata_str = dataset->GetRasterBand(bandNum)->GetMetadata("xml:isce");
    // The cereal-relevant XML is the first element in the list
    std::string meta{metadata_str[0]};
    // Close the VRT dataset
    GDALClose(dataset);
    // Convert to stream
    std::stringstream metastream;
    metastream << meta;
    // All done
    return metastream;
}

int main(int argc, char * argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// end of file
