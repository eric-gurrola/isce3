//
// Author: Joshua Cohen
// Copyright 2017
//

#include <cmath>
#include <vector>
#include "Constants.h"
#include "Ellipsoid.h"
#include "LinAlg.h"


void
isce::core::Ellipsoid::latLonToXyz(const std::vector<double> &llh, std::vector<double> &xyz) const {
    /*
     * Given a lat, lon, and height, produces a geocentric vector.
     */

    // Error checking to make sure inputs have expected characteristics
    checkVecLen(llh,3);
    checkVecLen(xyz,3);

    // Radius of Earth in East direction
    auto re = rEast(llh[0]);
    // Parametric representation of a circle as a function of longitude
    xyz[0] = (re + llh[2]) * cos(llh[0]) * cos(llh[1]);
    xyz[1] = (re + llh[2]) * cos(llh[0]) * sin(llh[1]);
    // Parametric representation with the radius adjusted for eccentricity
    xyz[2] = ((re * (1. - e2)) + llh[2]) * sin(llh[0]);
}

void
isce::core::Ellipsoid::xyzToLatLon(const std::vector<double> &xyz, std::vector<double> &llh) const {
    /*
     * Given a geocentric XYZ, produces a lat, lon, and height above the reference ellipsoid.
     *      VERMEILLE IMPLEMENTATION
     */

    // Error checking to make sure inputs have expected characteristics
    checkVecLen(llh,3);
    checkVecLen(xyz,3);

    // Lateral distance normalized by the major axis
    double p = (pow(xyz[0], 2) + pow(xyz[1], 2)) / pow(a, 2);
    // Polar distance normalized by the minor axis
    double q = ((1. - e2) * pow(xyz[2], 2)) / pow(a, 2);
    double r = (p + q - pow(e2, 2)) / 6.;
    double s = (pow(e2, 2) * p * q) / (4. * pow(r, 3.));
    double t = pow(1. + s + sqrt(s * (2. + s)), (1./3.));
    double u = r * (1. + t + (1. / t));
    double rv = sqrt(pow(u, 2) + (pow(e2, 2) * q));
    double w = (e2 * (u + rv - q)) / (2. * rv);
    double k = sqrt(u + rv + pow(w, 2)) - w;
    // Radius adjusted for eccentricity
    double d = (k * sqrt(pow(xyz[0], 2) + pow(xyz[1], 2))) / (k + e2);
    // Latitude is a function of z and radius
    llh[0] = atan2(xyz[2], d);
    // Longitude is a function of x and y
    llh[1] = atan2(xyz[1], xyz[0]);
    // Height is a function of location and radius
    llh[2] = ((k + e2 - 1.) * sqrt(pow(d, 2) + pow(xyz[2], 2))) / k;
}

/*
void Ellipsoid::xyzToLatLon(std::vector<double> &xyz, std::vector<double> &llh) {
    //
    // Given a geocentric XYZ, produces a lat, lon, and height above the reference ellipsoid.
    //      SCOTT HENSLEY IMPLEMENTATION
    //

    // Error checking to make sure inputs have expected characteristics
    checkVecLen(llh,3);
    checkVecLen(xyz,3);

    double b = a * sqrt(1. - e2);
    double p = sqrt(pow(v[0], 2) + pow(v[1], 2));
    double tant = (v[2] / p) * sqrt(1. / (1. - e2));
    double theta = atan(tant);
    tant = (v[2] + (((1. / (1. - e2)) - 1.) * b * pow(sin(theta), 3))) /
           (p - (e2 * a * pow(cos(theta), 3)));
    llh[0] = atan(tant);
    llh[1] = atan2(v[1], v[0]);
    llh[2] = (p / cos(llh[0])) - rEast(llh[0]);
}
*/

void
isce::core::Ellipsoid::getAngs(const std::vector<double> &pos, const std::vector<double> &vel,
                        const std::vector<double> &vec, double &az, double &lk) const {
    /*
     * Computes the look vector given the look angle, azimuth angle, and position vector
     */

    // Error checking to make sure inputs have expected characteristics
    checkVecLen(pos,3);
    checkVecLen(vel,3);
    checkVecLen(vec,3);

    std::vector<double> temp(3);
    xyzToLatLon(pos, temp);

    std::vector<double> n = {-cos(temp[0]) * cos(temp[1]),
                        -cos(temp[0]) * sin(temp[1]),
                        -sin(temp[0])};
    lk = acos(isce::core::LinAlg::dot(n, vec) / isce::core::LinAlg::norm(vec));
    isce::core::LinAlg::cross(n, vel, temp);

    std::vector<double> c(3);
    isce::core::LinAlg::unitVec(temp, c);
    isce::core::LinAlg::cross(c, n, temp);

    std::vector<double> t(3);
    isce::core::LinAlg::unitVec(temp, t);
    az = atan2(isce::core::LinAlg::dot(c, vec), isce::core::LinAlg::dot(t, vec));
}

void
isce::core::Ellipsoid::getTCN_TCvec(const std::vector<double> &pos, const std::vector<double> &vel,
                             const std::vector<double> &vec, std::vector<double> &TCVec) const {
    /*
     * Computes the projection of an xyz vector on the TC plane in xyz
     */

    // Error checking to make sure inputs have expected characteristics
    checkVecLen(pos,3);
    checkVecLen(vel,3);
    checkVecLen(vec,3);
    checkVecLen(TCVec,3);

    std::vector<double> temp(3);
    xyzToLatLon(pos, temp);

    std::vector<double> n = {-cos(temp[0]) * cos(temp[1]),
                        -cos(temp[0]) * sin(temp[1]),
                        -sin(temp[0])};
    isce::core::LinAlg::cross(n, vel, temp);

    std::vector<double> c(3);
    isce::core::LinAlg::unitVec(temp, c);
    isce::core::LinAlg::cross(c, n, temp);

    std::vector<double> t(3);
    isce::core::LinAlg::unitVec(temp, t);
    isce::core::LinAlg::linComb(isce::core::LinAlg::dot(t, vec), t, isce::core::LinAlg::dot(c, vec),
                                c, TCVec);
}

void
isce::core::Ellipsoid::TCNbasis(const std::vector<double> &pos, const std::vector<double> &vel,
                         std::vector<double> &t, std::vector<double> &c,
                         std::vector<double> &n) const {
    /*
     *
     */

    // Error checking to make sure inputs have expected characteristics
    checkVecLen(pos,3);
    checkVecLen(vel,3);
    checkVecLen(t,3);
    checkVecLen(c,3);
    checkVecLen(n,3);

    std::vector<double> llh(3);
    xyzToLatLon(pos, llh);

    n = {-cos(llh[0]) * cos(llh[1]),
         -cos(llh[0]) * sin(llh[1]),
         -sin(llh[0])};

    std::vector<double> temp(3);
    isce::core::LinAlg::cross(n, vel, temp);
    isce::core::LinAlg::unitVec(temp, c);
    isce::core::LinAlg::cross(c, n, temp);
    isce::core::LinAlg::unitVec(temp, t);
}
