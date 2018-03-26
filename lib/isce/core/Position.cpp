//
// Author: Joshua Cohen
// Copyright 2017
//

#include <cmath>
#include <vector>
#include "Constants.h"
#include "LinAlg.h"
#include "Position.h"


void
isce::core::Position::
lookVec(double look, double az, std::vector<double> &v) const {
    /*
     * Computes the look vector given the look angle, azimuth angle, and position vector.
     */

    checkVecLen(v,3);

    std::vector<double> n(3);
    isce::core::LinAlg::unitVec(j, n);

    for (int i=0; i<3; i++) n[i] = -n[i];
    std::vector<double> temp(3);
    isce::core::LinAlg::cross(n, jdot, temp);

    std::vector<double> c(3);
    isce::core::LinAlg::unitVec(temp, c);
    isce::core::LinAlg::cross(c, n, temp);

    std::vector<double> t(3);
    isce::core::LinAlg::unitVec(temp, t);
    isce::core::LinAlg::linComb(cos(az), t, sin(az), c, temp);

    std::vector<double> w(3);
    isce::core::LinAlg::linComb(cos(look), n, sin(look), temp, w);
    isce::core::LinAlg::unitVec(w, v);
}
