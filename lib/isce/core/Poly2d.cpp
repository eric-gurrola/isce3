//
// Author: Joshua Cohen
// Copyright 2017
//

#include <iostream>
#include <pyre/journal.h>

#include "Constants.h"
#include "Poly2d.h"

double
isce::core::Poly2d::
eval(double azi, double rng) const {

    double xval = (rng - rangeMean) / rangeNorm;
    double yval = (azi - azimuthMean) / azimuthNorm;

    double scalex;
    double scaley = 1.;
    double val = 0.;
    for (int i=0; i<=azimuthOrder; i++,scaley*=yval) {
        scalex = 1.;
        for (int j=0; j<=rangeOrder; j++,scalex*=xval) {
            val += scalex * scaley * coeffs[IDX1D(i,j,rangeOrder+1)];
        }
    }
    return val;
}

void
isce::core::Poly2d::
printPoly() const {
    // make a channel
    pyre::journal::debug_t channel("isce.core.debug");
    // print information
    channel
        << pyre::journal::at(__HERE__)
        << "Polynomial order (azimuth, range): "
        << azimuthOrder
        << ", "
        << rangeOrder
        << pyre::journal::newline;

    for (int i=0; i<=azimuthOrder; i++) {
        for (int j=0; j<=rangeOrder; j++) {
            channel << getCoeff(i,j) << " ";
        }
        channel << pyre::journal::newline;
    }
    channel << pyre::journal::endl;
}
