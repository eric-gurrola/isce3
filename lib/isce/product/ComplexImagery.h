// -*- C++ -*-
// -*- coding: utf-8 -*-
//
// Source Author: Bryan Riel
// Copyright 2017-2018

#ifndef ISCE_PRODUCT_COMPLEXIMAGERY_H
#define ISCE_PRODUCT_COMPLEXIMAGERY_H

// isce::product
#include <isce/product/ImageMode.h>

// Declaration
namespace isce {
    namespace product {
        class ComplexImagery;
    }
}

// ComplexImagery class declaration
class isce::product::ComplexImagery {

    public:
        /** Default constructor. */
        inline ComplexImagery() {}

        /** Get a copy of the auxiliary mode. */
        inline ImageMode auxMode() const  { return _auxMode; }

        /** Set auxiliary mode. */
        inline void auxMode(const ImageMode & mode) { _auxMode = mode; }

        /** Get a copy of the primary mode. */
        inline ImageMode primaryMode() const { return _primaryMode; }
        /** Set primary mode. */
        inline void primaryMode(const ImageMode & mode) { _primaryMode = mode; }

        /** Crop the ComplexImagery. */
        inline void crop(size_t, size_t, size_t, size_t);

   private:
        // ImageMode data
        ImageMode _auxMode;
        ImageMode _primaryMode;
};

// Get inline implementations for ImageMode
#define ISCE_PRODUCT_COMPLEXIMAGERY_ICC
#include "ComplexImagery.icc"
#undef ISCE_PRODUCT_COMPLEXIMAGERY_ICC

#endif

// end of file
