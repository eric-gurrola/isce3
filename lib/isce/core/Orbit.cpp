//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Joshua Cohen, Bryan V. Riel
// Copyright 2017-2018
//

#include "Orbit.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "Constants.h"
#include "Ellipsoid.h"
#include "TimeDelta.h"

//Reformat using MIN_DATE_TIME
void isce::core::Orbit::
reformatOrbit() {
    // Use min date time as epoch
    reformatOrbit(MIN_DATE_TIME);
}

/** @param[in] epoch DateTime corresponding to reference epocj
 *
 * Reference epoch is used to translate DateTime tags to double
 * precision seconds for all numerical computation*/
void isce::core::Orbit::
reformatOrbit(const DateTime & epoch) {

    // Get the number of state vectors
    nVectors = stateVectors.size();

    // Clear vectors
    position.clear();
    velocity.clear();
    UTCtime.clear();
    // Re-size
    position.resize(3*nVectors);
    velocity.resize(3*nVectors);
    UTCtime.resize(nVectors);
    epochs.resize(nVectors);

    // Loop over state vectors and fill in vectors
    for (int i = 0; i < nVectors; ++i) {
        // Get the statevector
        StateVector sv = stateVectors[i];
        // Save epoch
        epochs[i] = sv.datetime;
        // Place in correct spot in vectors
        for (size_t j = 0; j < 3; ++j) {
            position[(3*i) + j] = sv.position[j];
            velocity[(3*i) + j] = sv.velocity[j];
        }
        // Get UTCtime (implicit conversion to double)
        UTCtime[i] = sv.datetime.secondsSinceEpoch(epoch);
    }

    // Store the reference epoch
    refEpoch = epoch;
}

// Update UTC times
void isce::core::Orbit::
updateUTCTimes(const DateTime & epoch) {
    // Loop over time epochs
    for (int i = 0; i < nVectors; ++i) {
        UTCtime[i] = epochs[i].secondsSinceEpoch(epoch);
    }
    // Store the new epoch
    refEpoch = epoch;
}

/**
 * @param[in] tintp Time since reference epoch in seconds
 * @param[out] state StateVector object
 * @param[in] intp_type Method to use for interpolation*/
int isce::core::Orbit::
interpolate(double tintp, StateVector & state, orbitInterpMethod intp_type) const {
    /*
    Convenience wrapper for interpolation to place results directly into a
    StateVector object.
    */
    // Call interpolation
    cartesian_t position, velocity;
    int orbitStat = interpolate(tintp, position, velocity, intp_type);
    // Set results
    state.position = position;
    state.velocity = velocity;
    return orbitStat;
}

/**
 * @param[in] tintp Time since reference epoch in seconds
 * @param[out] opos Interpolated position (m)
 * @param[out] ovel Interpolated velocity (m/s)
 * @param[in] intp_type Method to use for interpolation
 *
 * Returns non-zero status on error*/
int isce::core::Orbit::
interpolate(double tintp, cartesian_t & opos, cartesian_t & ovel,
            orbitInterpMethod intp_type) const {
    /*
     * Single entry-point wrapper for orbit interpolation.
     */

    // Each contained function has its own error checking for size, so no need to do it here
    if (intp_type == HERMITE_METHOD) return interpolateWGS84Orbit(tintp, opos, ovel);
    else if (intp_type == SCH_METHOD) return interpolateSCHOrbit(tintp, opos, ovel);
    else if (intp_type == LEGENDRE_METHOD) return interpolateLegendreOrbit(tintp, opos, ovel);
    else {
        std::string errstr = "Unrecognized interpolation type in Orbit::interpolate.\n";
        errstr += "Expected one of:\n";
        errstr += "  HERMITE_METHOD (== "+std::to_string(HERMITE_METHOD)+")\n";
        errstr += "  SCH_METHOD (== "+std::to_string(SCH_METHOD)+")\n";
        errstr += "  LEGENDRE_METHOD (== "+std::to_string(LEGENDRE_METHOD)+")\n";
        errstr += "Encountered interpolation type "+std::to_string(intp_type);
        throw std::invalid_argument(errstr);
    }
}
/**
 * @param[in] tintp Time since reference epoch in seconds
 * @param[out] opos Interpolated position (m)
 * @param[out] ovel Interpolated position (m/s)
 *
 * Returns non-zero status on error. This method is similar to \cite sandwell2008accuracy*/
int isce::core::Orbit::
interpolateWGS84Orbit(double tintp, cartesian_t & opos, cartesian_t & ovel) const {
    /*
     * Interpolate WGS-84 orbit
     */
    if (nVectors < 4) {
        std::string errstr = "Orbit::interpolateWGS84Orbit requires at least 4 state vectors to ";
        errstr += "interpolate, Orbit only contains "+std::to_string(nVectors);
        throw std::length_error(errstr);
    }
    // Totally possible that a time is passed to interpolate that's out-of-epoch, but not
    // exception-worthy (so it just returns a 1 status)
    if ((tintp < UTCtime[0]) || (tintp > UTCtime[nVectors-1])) {
        //std::cout << "Error in Orbit::interpolateWGS84Orbit - Interpolation time requested (" << tintp <<
        //        ") is outside the epoch range of the stored vectors" << std::endl;
        // Don't stop the whole program, just flag this particular result
        return 1;
    }

    int idx = -1;
    for (int i=0; i<nVectors; i++) {
        if (UTCtime[i] >= tintp) {
            idx = i;
            break;
        }
    }
    idx -= 2;
    idx = std::min(std::max(idx, 0), nVectors-4);

    std::vector<cartesian_t> pos(4), vel(4);
    std::vector<double> t(4);
    for (int i=0; i<4; i++) getStateVector(idx+i, t[i], pos[i], vel[i]);

    orbitHermite(pos, vel, t, tintp, opos, ovel);

    return 0;
}

void isce::core::
orbitHermite(const std::vector<cartesian_t> &x, const std::vector<cartesian_t> &v,
             const std::vector<double> &t, double time, cartesian_t &xx,
             cartesian_t &vv) {
    /*
     * Method used by interpolateWGS84Orbit but is not tied to an Orbit
     */

    // No error checking needed, x/v/t were created (not passed) and xx/vv were size-checked before
    // passing through

    std::array<double, 4> f0, f1, h, hdot, g0, g1;
    double sum;
    for (int i=0; i<4; i++) {
        f1[i] = time - t[i];
        sum = 0.;
        for (int j=0; j<4; j++) {
            if (i != j) sum += 1. / (t[i] - t[j]);
        }
        f0[i] = 1. - (2. * (time - t[i]) * sum);
    }

    double product;
    for (int i=0; i<4; i++) {
        product = 1.;
        for (int j=0; j<4; j++) {
            if (i != j) product *= (time - t[j]) / (t[i] - t[j]);
        }
        h[i] = product;
        sum = 0.;
        for (int j=0; j<4; j++) {
            product = 1.;
            for (int k=0; k<4; k++) {
                if ((i != k) && (j != k)) product *= (time - t[k]) / (t[i] - t[k]);
            }
            if (i != j) sum += (1. / (t[i] - t[j])) * product;
        }
        hdot[i] = sum;
    }

    for (int i=0; i<4; i++) {
        g1[i] = h[i] + (2. * (time - t[i]) * hdot[i]);
        sum = 0.;
        for (int j=0; j<4; j++) {
            if (i != j) sum += 1. / (t[i] - t[j]);
        }
        g0[i] = 2. * ((f0[i] * hdot[i]) - (h[i] * sum));
    }

    for (int j=0; j<3; j++) {
        sum = 0.;
        for (int i=0; i<4; i++) {
            sum += ((x[i][j] * f0[i]) + (v[i][j] * f1[i])) * h[i] * h[i];
        }
        xx[j] = sum;
        sum = 0.;
        for (int i=0; i<4; i++) {
            sum += ((x[i][j] * g0[i]) + (v[i][j] * g1[i])) * h[i];
        }
        vv[j] = sum;
    }
}


/**
 * @param[in] tintp Time since reference epoch in seconds
 * @param[out] opos Interpolated position (m)
 * @param[out] ovel Interpolated position (m/s)
 *
 * Returns non-zero status on error. This method is very similar to \cite getorbdelft*/
int isce::core::Orbit::
interpolateLegendreOrbit(double tintp, cartesian_t & opos, cartesian_t & ovel) const {
    /*
     * Interpolate Legendre orbit
     */
    if (nVectors < 9) {
        std::string errstr = "Orbit::interpolateLegendreOrbit requires at least 9 state vectors to ";
        errstr += "interpolate, Orbit only contains "+std::to_string(nVectors);
        throw std::length_error(errstr);
    }
    // Totally possible that a time is passed to interpolate that's out-of-epoch, but not
    // exception-worthy (so it just returns a 1 status)
    if ((tintp < UTCtime[0]) || (tintp > UTCtime[nVectors-1])) {
        //std::cout << "Error in Orbit::interpolateLegendreOrbit - Interpolation time requested (" <<
        //        tintp << ") is outside the epoch range of the stored vectors" << std::endl;
        // Don't stop the whole program, just flag this particular result
        return 1;
    }

    int idx = -1;
    for (int i=0; i<nVectors; i++) {
        if (UTCtime[i] >= tintp) {
            idx = i;
            break;
        }
    }
    idx -= 5;
    idx = std::min(std::max(idx, 0), nVectors-9);

    std::vector<cartesian_t> pos(9), vel(9);
    std::vector<double> t(9);
    for (int i=0; i<9; i++) getStateVector(idx+i, t[i], pos[i], vel[i]);

    double trel = (8. * (tintp - t[0])) / (t[8] - t[0]);
    double teller = 1.;
    for (int i=0; i<9; i++) teller *= trel - i;

    opos = {0.0, 0.0, 0.0};
    ovel = {0.0, 0.0, 0.0};
    if (teller == 0.) {
        for (int i=0; i<3; i++) {
            opos[i] = pos[int(trel)][i];
            ovel[i] = vel[int(trel)][i];
        }
    } else {
        std::vector<double> noemer = {40320.0, -5040.0, 1440.0, -720.0, 576.0, -720.0, 1440.0, -5040.0,
                                 40320.0};
        double coeff;
        for (int i=0; i<9; i++) {
            coeff = (teller / noemer[i]) / (trel - i);
            for (int j=0; j<3; j++) {
                opos[j] += coeff * pos[i][j];
                ovel[j] += coeff * vel[i][j];
            }
        }
    }
    return 0;
}

/**
 * @param[in] tintp Time since reference epoch in seconds
 * @param[out] opos Interpolated position (m)
 * @param[out] ovel Interpolated position (m/s)
 *
 * Returns non-zero status on error*/
int isce::core::Orbit::
interpolateSCHOrbit(double tintp, cartesian_t & opos, cartesian_t & ovel) const {
    /*
     * Interpolate SCH orbit
     */
    if (nVectors < 2) {
        std::string errstr = "Orbit::interpolateSCHOrbit requires at least 2 state vectors to ";
        errstr += "interpolate, Orbit only contains "+std::to_string(nVectors);
        throw std::length_error(errstr);
    }
    // Totally possible that a time is passed to interpolate that's out-of-epoch, but not
    // exception-worthy (so it just returns a 1 status)
    if ((tintp < UTCtime[0]) || (tintp > UTCtime[nVectors-1])) {
        //std::cout << "Error in Orbit::interpolateSCHOrbit - Interpolation time requested (" << tintp <<
        //        ") is outside the epoch range of the stored vectors" << std::endl;
        // Don't stop the whole program, just flag this particular result
        return 1;
    }

    std::vector<cartesian_t> pos(2), vel(2);
    std::vector<double> t(2);
    opos = {0.0, 0.0, 0.0};
    ovel = {0.0, 0.0, 0.0};
    double frac;
    for (int i=0; i<nVectors; i++) {
        frac = 1.;
        getStateVector(i, t[0], pos[0], vel[0]);
        for (int j=0; j<nVectors; j++) {
            if (i == j) continue;
            getStateVector(j, t[1], pos[1], vel[1]);
            frac *= (t[1] - tintp) / (t[1] - t[0]);
        }
        for (int j=0; j<3; j++) {
            opos[j] += frac * pos[0][j];
            ovel[j] += frac * vel[0][j];
        }
    }
    return 0;
}

/**
 * @param[in] tintp Time since reference epoch in seconds
 * @param[out] acc Acceleration in m/s^2
 *
 * An interval of +/- 0.01 seconds is used for numerical computations*/
int isce::core::Orbit::
computeAcceleration(double tintp, cartesian_t &acc) const {
    /*
     * Interpolate acceleration.
     */
    cartesian_t dummy, vbef;
    double temp = tintp - .01;
    int stat = interpolateWGS84Orbit(temp, dummy, vbef);
    if (stat == 1) return stat;

    cartesian_t vaft;
    temp = tintp + .01;
    stat = interpolateWGS84Orbit(temp, dummy, vaft);
    if (stat == 1) return stat;

    for (int i=0; i<3; i++) acc[i] = (vaft[i] - vbef[i]) / .02;
    return 0;
}

/** Computes heading at a given azimuth time using a single state vector
 * @param[in] aztime Time since reference epoch in seconds*/
double isce::core::Orbit::
getENUHeading(double aztime) const {
    const isce::core::Ellipsoid refElp(EarthSemiMajorAxis, EarthEccentricitySquared);

    // Interpolate orbit to azimuth time
    cartesian_t pos, vel;
    interpolateWGS84Orbit(aztime, pos, vel);
    // Convert platform position to LLH
    const Vec3 llh = refElp.xyzToLonLat(pos);
    // Get ENU transformation matrix
    const Mat3 xyz2enu = Mat3::xyzToEnu(llh[1], llh[0]);
    // Compute velocity in ENU
    const Vec3 enuvel = xyz2enu.dot(vel);
    // Get heading from velocity
    return std::atan2(enuvel[0], enuvel[1]);
}

void isce::core::Orbit::
printOrbit() const {
    /*
     * Debug print the stored orbit.
     */

    std::cout << "Orbit - nVectors: " << nVectors << std::endl;
    for (int i=0; i<nVectors; i++) {
        std::cout << "  UTC = " << UTCtime[i] << std::endl;
        std::cout << "  Position = [ " << position[3*i] << " , " << position[3*i+1] << " , " <<
                position[3*i+2] << " ]" << std::endl;
        std::cout << "  Velocity = [ " << velocity[3*i] << " , " << velocity[3*i+1] << " , " <<
                velocity[3*i+2] << " ]" << std::endl;
    }
}

void isce::core::Orbit::
loadFromHDR(const char *filename, bool update_epoch) {
    /*
     *  Load Orbit from a saved HDR file using fstreams. This assumes that the Orbit was dumped to
     *  an HDR file using this interface (or a compatible one given the reading scheme below), and
     *  will most likely fail on any other arbitrary file.
     */

    std::ifstream fs(filename);
    if (!fs.is_open()) {
        std::string errstr = "Unable to open orbit HDR file '"+std::string(filename)+"'";
        fs.close();
        throw std::runtime_error(errstr);
    }

    nVectors = 0;
    std::string line;
    while (std::getline(fs, line)) nVectors++;

    UTCtime.clear();
    UTCtime.resize(nVectors);
    epochs.clear();
    epochs.resize(nVectors);
    position.clear();
    position.resize(3*nVectors);
    velocity.clear();
    velocity.resize(3*nVectors);

    fs.clear();
    fs.seekg(0);

    cartesian_t pos, vel;
    std::string ts;
    int count = 0;
    // Take advantage of the power of fstreams
    while (fs >> ts >> pos[0] >> pos[1] >> pos[2] >> vel[0] >> vel[1] >> vel[2]) {
        auto t = DateTime(ts);
        if (update_epoch && (count == 0)) {
            refEpoch = t;
        }
        double sec = t.secondsSinceEpoch(refEpoch);
        setStateVector(count, sec, pos, vel);
        count++;
    }
    fs.close();

    if (fs.bad()) {
        nVectors = 0;
        UTCtime.resize(0);
        position.resize(0);
        velocity.resize(0);
        std::cout << "Error reading orbit HDR file '" << filename << "'" << std::endl;
    } else {
        std::cout << "Read in " << nVectors << " state vectors from " << filename << std::endl;
    }
}

void isce::core::Orbit::
dumpToHDR(const char* filename) const {
    /*
     *  Save Orbit to a given HDR file using fstreams. This saving scheme is compatible with the
     *  above reading scheme.
     */

    std::ofstream fs(filename);
    if (!fs.is_open()) {
        std::string errstr = "Unable to open orbit HDR file '"+std::string(filename)+"'";
        fs.close();
        throw std::runtime_error(errstr);
    }

    std::cout << "Writing " << nVectors << " vectors to '" << filename << "'" << std::endl;
    // In keeping with the original HDR file formatting for this object, force the + sign to display
    // for positive values
    fs << std::showpos;
    fs.precision(16);
    for (int i=0; i<nVectors; i++) {
        auto t = refEpoch + TimeDelta(UTCtime[i]);
        fs << t.isoformat() << " " << position[3*i] << " " << position[3*i+1] << " " << position[3*i+2]
            << " " << velocity[3*i] << " " << velocity[3*i+1] << " " << velocity[3*i+2] << std::endl;
    }
    fs.close();
}

// end of file
