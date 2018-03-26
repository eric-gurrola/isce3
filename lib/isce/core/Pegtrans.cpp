//
// Author: Joshua Cohen
// Copyright 2017
//

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>
#include "Constants.h"
#include "Ellipsoid.h"
#include "LinAlg.h"
#include "Peg.h"
#include "Pegtrans.h"

using std::vector;


void
isce::core::Pegtrans::
radarToXYZ(const isce::core::Ellipsoid &elp, const isce::core::Peg &peg) {
    /*
     * Computes the transformation matrix and translation vector needed to convert
     * between radar (s,c,h) coordinates and WGS-84 (x,y,z) coordinates
    */

    mat[0][0] = cos(peg.lat) * cos(peg.lon);
    mat[0][1] = -(sin(peg.hdg) * sin(peg.lon)) - (sin(peg.lat) * cos(peg.lon) * cos(peg.hdg));
    mat[0][2] = (sin(peg.lon) * cos(peg.hdg)) - (sin(peg.lat) * cos(peg.lon) * sin(peg.hdg));
    mat[1][0] = cos(peg.lat) * sin(peg.lon);
    mat[1][1] = (cos(peg.lon) * sin(peg.hdg)) - (sin(peg.lat) * sin(peg.lon) * cos(peg.hdg));
    mat[1][2] = -(cos(peg.lon) * cos(peg.hdg)) - (sin(peg.lat) * sin(peg.lon) * sin(peg.hdg));
    mat[2][0] = sin(peg.lat);
    mat[2][1] = cos(peg.lat) * cos(peg.hdg);
    mat[2][2] = cos(peg.lat) * sin(peg.hdg);

    isce::core::LinAlg::tranMat(mat, matinv);

    radcur = elp.rDir(peg.hdg, peg.lat);

    std::vector<double> llh = {peg.lat, peg.lon, 0.};
    std::vector<double> p(3);
    elp.latLonToXyz(llh, p);
    std::vector<double> up = {cos(peg.lat) * cos(peg.lon), cos(peg.lat) * sin(peg.lon), sin(peg.lat)
                             };
    isce::core::LinAlg::linComb(1., p, -radcur, up, ov);
}

void
isce::core::Pegtrans::
convertSCHtoXYZ(std::vector<double> &schv, std::vector<double> &xyzv,
                isce::core::orbitConvMethod ctype) const {
    /*
     * Applies the affine matrix provided to convert from the radar sch coordinates to WGS-84 xyz
     * coordinates or vice-versa
    */

    // Error checking
    checkVecLen(schv,3);
    checkVecLen(xyzv,3);

    std::vector<double> schvt(3), llh(3);
    isce::core::Ellipsoid sph(radcur,0.);

    if (ctype == SCH_2_XYZ) {
        llh = {schv[1]/radcur, schv[0]/radcur, schv[2]};
        sph.latLonToXyz(llh, schvt);
        isce::core::LinAlg::matVec(mat, schvt, xyzv);
        isce::core::LinAlg::linComb(1., xyzv, 1., ov, xyzv);
    } else if (ctype == XYZ_2_SCH) {
        isce::core::LinAlg::linComb(1., xyzv, -1., ov, schvt);
        isce::core::LinAlg::matVec(matinv, schvt, schv);
        sph.xyzToLatLon(schv, llh);
        schv = {radcur*llh[1], radcur*llh[0], llh[2]};
    } else {
        std::string errstr = "Unrecognized conversion type in Pegtrans::convertSCHtoXYZ.\n";
        errstr += "Expected one of:\n";
        errstr += "  SCH_2_XYZ (== "+std::to_string(SCH_2_XYZ)+")\n";
        errstr += "  XYZ_2_SCH (== "+std::to_string(XYZ_2_SCH)+")\n";
        errstr += "Encountered conversion type "+std::to_string(ctype);
        throw std::invalid_argument(errstr);
    }
}

void
isce::core::Pegtrans::
convertSCHdotToXYZdot(const std::vector<double> &sch, const std::vector<double> &xyz,
                      std::vector<double> &schdot, std::vector<double> &xyzdot,
                      isce::core::orbitConvMethod ctype) const {
    /*
     * Applies the affine matrix provided to convert from the radar sch velociy to WGS-84 xyz
     * velocity or vice-versa
    */

    checkVecLen(sch,3);
    checkVecLen(xyz,3);
    checkVecLen(schdot,3);
    checkVecLen(xyzdot,3);

    std::vector<std::vector<double>> schxyzmat(3, std::vector<double>(3));
    std::vector<std::vector<double>> xyzschmat(3, std::vector<double>(3));
    SCHbasis(sch, xyzschmat, schxyzmat);

    if (ctype == SCH_2_XYZ) isce::core::LinAlg::matVec(schxyzmat, schdot, xyzdot);
    else if (ctype == XYZ_2_SCH) isce::core::LinAlg::matVec(xyzschmat, xyzdot, schdot);
    else {
        std::string errstr = "Unrecognized conversion type in Pegtrans::convertSCHdotToXYZdot.\n";
        errstr += "Expected one of:\n";
        errstr += "  SCH_2_XYZ (== "+std::to_string(SCH_2_XYZ)+")\n";
        errstr += "  XYZ_2_SCH (== "+std::to_string(XYZ_2_SCH)+")\n";
        errstr += "Encountered conversion type "+std::to_string(ctype);
        throw std::invalid_argument(errstr);
    }
}

void
isce::core::Pegtrans::
SCHbasis(const std::vector<double> &sch, std::vector<std::vector<double>> &xyzschmat,
         std::vector<std::vector<double>> &schxyzmat) const {
    /*
     * Computes the transformation matrix from xyz to a local sch frame
     */

    checkVecLen(sch,3);
    check2dVecLen(xyzschmat,3,3);
    check2dVecLen(schxyzmat,3,3);

    std::vector<std::vector<double>> matschxyzp = {
        {-sin(sch[0]/radcur), -(sin(sch[1]/radcur) * cos(sch[0]/radcur)),
          cos(sch[0]/radcur) * cos(sch[1]/radcur)},
        { cos(sch[0]/radcur), -(sin(sch[1]/radcur) * sin(sch[0]/radcur)),
          sin(sch[0]/radcur) * cos(sch[1]/radcur)},
        { 0., cos(sch[1]/radcur), sin(sch[1]/radcur)}};
    isce::core::LinAlg::matMat(mat, matschxyzp, schxyzmat);
    isce::core::LinAlg::tranMat(schxyzmat, xyzschmat);
}
