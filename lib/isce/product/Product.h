// -*- C++ -*-
// -*- coding: utf-8 -*-
//
// Source Author: Bryan Riel
// Copyright 2017-2018

#ifndef ISCE_PRODUCT_PRODUCT_H
#define ISCE_PRODUCT_PRODUCT_H

// std
#include <string>

// isce::product
#include <isce/product/ComplexImagery.h>
#include <isce/product/Metadata.h>
#include <isce/product/Serialization.h>
#include <isce/io/Raster.h>

// Declarations
namespace isce {
    namespace product {
        class Product;
    }
}

// Product class declaration
class isce::product::Product {

    public:
        /** Constructor from IH5File object. */
        inline Product(isce::io::IH5File &);

        /** Constructor from VRT file. */
        inline Product(std::string vrtFile, int numBand=1);

        /** Constructor with ComplexImagery and Metadata objects. */
        inline Product(const ComplexImagery &, const Metadata &);

        /** Get a const reference to the complex imagery. */
        inline const ComplexImagery & complexImagery() const { return _complexImagery; }
  //inline  ComplexImagery & complexImagery() { return _complexImagery; }

        /** Get a const reference to the metadata. */
        inline const Metadata & metadata() const { return _metadata; }

        /** Get the filename of the HDF5 file. */
        inline std::string filename() const { return _filename; }

        /** Crop the product. */
        inline void crop(size_t, size_t, size_t, size_t);
        inline void crop(const std::vector<size_t> & w) { crop(w[0], w[1], w[2], w[3]); };

    private:
        ComplexImagery _complexImagery;
        Metadata _metadata;
        std::string _filename;
};

// Get inline implementations for ImageMode
#define ISCE_PRODUCT_PRODUCT_ICC
#include "Product.icc"
#undef ISCE_PRODUCT_PRODUCT_ICC

#endif

// end of file
