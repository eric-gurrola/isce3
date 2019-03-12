//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Bryan V. Riel
// Copyright 2017-2018

/** \file Serialization.h
 *
 * Serialization functions for isce::core objects. */

#ifndef ISCE_CORE_SERIALIZATION_H
#define ISCE_CORE_SERIALIZATION_H

#include <iostream>
#include <memory>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/xml.hpp>

// pyre
#include <pyre/journal.h>

// isce::core
#include <isce/core/DateTime.h>
#include <isce/core/EulerAngles.h>
#include <isce/core/Ellipsoid.h>
#include <isce/core/Metadata.h>
#include <isce/core/Orbit.h>
#include <isce/core/Poly2d.h>
#include <isce/core/LUT1d.h>
#include <isce/core/LUT2d.h>
#include <isce/core/StateVector.h>

// isce::io
#include <isce/io/IH5.h>
#include <isce/io/Serialization.h>

//! The isce namespace
namespace isce {
    //! The isce::core namespace
    namespace core {

        // Archiving any isce::core object by pointer
        template <typename T>
        inline void load_archive(std::string metadata, char * objectTag, T * object) {
            std::stringstream metastream;
            metastream << metadata;
            cereal::XMLInputArchive archive(metastream);
            archive(cereal::make_nvp(objectTag, (*object)));
        }

        // Archiving any isce::core object by reference
        template <typename T>
        inline void load_archive_reference(std::string metadata, char * objectTag, T & object) {
            std::stringstream metastream;
            metastream << metadata;
            cereal::XMLInputArchive archive(metastream);
            archive(cereal::make_nvp(objectTag, object));
        }

        // ------------------------------------------------------------------------
        // Serialization for Ellipsoid
        // ------------------------------------------------------------------------

        template<class Archive>
        inline void save(Archive & archive, const Ellipsoid & ellps) {
            archive(cereal::make_nvp("a", ellps.a()),
                    cereal::make_nvp("e2", ellps.e2()));
        }

        template<class Archive>
        inline void load(Archive & archive, Ellipsoid & ellps) {
            double a, e2;
            archive(cereal::make_nvp("a", a),
                    cereal::make_nvp("e2", e2));
            ellps.a(a);
            ellps.e2(e2);
        }

        /** Load Ellipsoid parameters from HDF5.
         *
         * @param[in] group         HDF5 group object.
         * @param[in] ellps         Ellipsoid object to be configured. */
        inline void loadFromH5(isce::io::IGroup & group, Ellipsoid & ellps) {
            // Read data
            std::vector<double> ellpsData;
            isce::io::loadFromH5(group, "ellipsoid", ellpsData);
            // Set ellipsoid properties
            ellps.a(ellpsData[0]);
            ellps.e2(ellpsData[1]);
        }

        // ------------------------------------------------------------------------
        // Serialization for Orbit
        // ------------------------------------------------------------------------

        template <class Archive>
        inline void save(Archive & archive, const Orbit & orbit) {
            archive(cereal::make_nvp("StateVectors", orbit.stateVectors));
        }

        template <class Archive>
        inline void load(Archive & archive, Orbit & orbit) {
            // Load data
            archive(cereal::make_nvp("StateVectors", orbit.stateVectors));
            // Reformat state vectors to 1D arrays
            orbit.reformatOrbit();
        }

        /** \brief Load orbit data from HDF5 product.
         *
         * @param[in] group         HDF5 group object.
         * @param[in] orbit         Orbit object to be configured. */
        inline void loadFromH5(isce::io::IGroup & group, Orbit & orbit) {
            // Reset orbit data
            orbit.position.clear();
            orbit.velocity.clear();
            orbit.UTCtime.clear();
            orbit.epochs.clear();

            // Load position
            isce::io::loadFromH5(group, "position", orbit.position);

            // Load velocity
            isce::io::loadFromH5(group, "velocity", orbit.velocity);

            // Load time
            isce::io::loadFromH5(group, "time", orbit.UTCtime);

            // Get the reference epoch
            orbit.refEpoch = isce::io::getRefEpoch(group, "time");

            // Convert UTC seconds since epoch to timestamp
            orbit.nVectors = orbit.UTCtime.size();
            orbit.epochs.resize(orbit.nVectors);
            for (size_t i = 0; i < orbit.nVectors; ++i) {
                DateTime date = orbit.refEpoch;
                date += orbit.UTCtime[i];
                orbit.epochs[i] = date;
            }

        }

        // ------------------------------------------------------------------------
        // Serialization for EulerAngles
        // ------------------------------------------------------------------------

        /** \brief Load Euler angle data from HDF5 product.
         *
         * @param[in] group         HDF5 group object.
         * @param[in] euler         Orbit object to be configured. */
        inline void loadFromH5(isce::io::IGroup & group, EulerAngles & euler) {

            // Create temporary data
            std::vector<double> time, angles, yaw, pitch, roll;
            isce::core::DateTime refEpoch;

            // Load angles
            isce::io::loadFromH5(group, "eulerAngles", angles);

            // Load time
            isce::io::loadFromH5(group, "time", time);

            // Get the reference epoch
            refEpoch = isce::io::getRefEpoch(group, "time");

            // Unpack the angles
            const double rad = M_PI / 180.0;
            yaw.resize(time.size());
            pitch.resize(time.size());
            roll.resize(time.size());
            for (size_t i = 0; i < time.size(); ++i) {
                yaw[i] = rad * angles[i*3 + 0];
                pitch[i] = rad * angles[i*3 + 1];
                roll[i] = rad * angles[i*3 + 2];
            }

            // Save to EulerAngles object
            euler.data(time, yaw, pitch, roll);
            euler.refEpoch(refEpoch);
        }

        // ------------------------------------------------------------------------
        // Serialization for Metadata
        // ------------------------------------------------------------------------

        template <class Archive>
        inline void save(Archive & archive, const Metadata & meta) {
            archive(cereal::make_nvp("width", meta.width),
                    cereal::make_nvp("length", meta.length),
                    cereal::make_nvp("numberRangeLooks", meta.numberRangeLooks),
                    cereal::make_nvp("numberAzimuthLooks", meta.numberAzimuthLooks),
                    cereal::make_nvp("slantRangePixelSpacing", meta.slantRangePixelSpacing),
                    cereal::make_nvp("rangeFirstSample", meta.rangeFirstSample),
                    cereal::make_nvp("lookSide", meta.lookSide),
                    cereal::make_nvp("prf", meta.prf),
                    cereal::make_nvp("radarWavelength", meta.radarWavelength),
                    cereal::make_nvp("pegHeading", meta.pegHeading),
                    cereal::make_nvp("pegLatitude", meta.pegLatitude),
                    cereal::make_nvp("pegLongitude", meta.pegLongitude),
                    cereal::make_nvp("chirpSlope", meta.chirpSlope),
                    cereal::make_nvp("antennaLength", meta.antennaLength),
                    cereal::make_nvp("sensingStart", meta.sensingStart.isoformat()));
        }

        template <class Archive>
        inline void load(Archive & archive, Metadata & meta) {
            std::string sensingStart;
            archive(cereal::make_nvp("width", meta.width),
                    cereal::make_nvp("length", meta.length),
                    cereal::make_nvp("numberRangeLooks", meta.numberRangeLooks),
                    cereal::make_nvp("numberAzimuthLooks", meta.numberAzimuthLooks),
                    cereal::make_nvp("slantRangePixelSpacing", meta.slantRangePixelSpacing),
                    cereal::make_nvp("rangeFirstSample", meta.rangeFirstSample),
                    cereal::make_nvp("lookSide", meta.lookSide),
                    cereal::make_nvp("prf", meta.prf),
                    cereal::make_nvp("radarWavelength", meta.radarWavelength),
                    cereal::make_nvp("pegHeading", meta.pegHeading),
                    cereal::make_nvp("pegLatitude", meta.pegLatitude),
                    cereal::make_nvp("pegLongitude", meta.pegLongitude),
                    cereal::make_nvp("chirpSlope", meta.chirpSlope),
                    cereal::make_nvp("antennaLength", meta.antennaLength),
                    cereal::make_nvp("sensingStart", sensingStart));
            meta.sensingStart = sensingStart;
        }

        // ------------------------------------------------------------------------
        // Serialization for Poly2d
        // ------------------------------------------------------------------------

        // Definition for Poly2d
        template <class Archive>
        inline void serialize(Archive & archive, Poly2d & poly) {
            archive(cereal::make_nvp("rangeOrder", poly.rangeOrder),
                    cereal::make_nvp("azimuthOrder", poly.azimuthOrder),
                    cereal::make_nvp("rangeMean", poly.rangeMean),
                    cereal::make_nvp("azimuthMean", poly.azimuthMean),
                    cereal::make_nvp("rangeNorm", poly.rangeNorm),
                    cereal::make_nvp("azimuthNorm", poly.azimuthNorm),
                    cereal::make_nvp("coeffs", poly.coeffs));
        }

        /** \brief Load polynomial coefficients from HDF5 product.
         *
         * @param[in] group         HDF5 group object.
         * @param[in] poly          Poly2d to be configured.
         * @param[in] name          Dataset name within group. */
        inline void loadFromH5(isce::io::IGroup & group, Poly2d & poly, std::string name) {

            // Configure the polynomial coefficients
            isce::io::loadFromH5(group, name, poly.coeffs);

            // Set other polynomial properties
            poly.rangeOrder = poly.coeffs.size() - 1;
            poly.azimuthOrder = 0;
            poly.rangeMean = 0.0;
            poly.azimuthMean = 0.0;
            poly.rangeNorm = 1.0;
            poly.azimuthNorm = 1.0;
        }

        // ------------------------------------------------------------------------
        // Serialization for LUT2d (specifically for calibration grids)
        // ------------------------------------------------------------------------

         /** \brief Load LUT2d data from HDF5 product.
         *
         * @param[in] group         HDF5 group object.
         * @param[in] dsetName      Dataset name within group
         * @param[in] lut           LUT2d to be configured. */
        template <typename T>
        inline void loadCalGrid(isce::io::IGroup & group, const std::string & dsetName,
                                isce::core::LUT2d<T> & lut) {

            // Load coordinates
            std::valarray<double> slantRange, zeroDopplerTime;
            isce::io::loadFromH5(group, "slantRange", slantRange);
            isce::io::loadFromH5(group, "zeroDopplerTime", zeroDopplerTime);

            // Get reference epoch
            isce::core::DateTime refEpoch = isce::io::getRefEpoch(group, "zeroDopplerTime");

            // Load LUT2d data in matrix
            isce::core::Matrix<T> matrix(zeroDopplerTime.size(), slantRange.size());
            isce::io::loadFromH5(group, dsetName, matrix);

            // Set in lut
            lut.setFromData(slantRange, zeroDopplerTime, matrix);
        }

        // ------------------------------------------------------------------------
        // Serialization for LUT1d
        // ------------------------------------------------------------------------

        // Serialization save method
        template <class Archive, typename T>
        inline void save(Archive & archive, LUT1d<T> const & lut) {
            // Copy LUT data from valarrays to vectors
            std::vector<double> coords(lut.size());
            std::vector<T> values(lut.size());
            auto v_coords = lut.coords();
            auto v_values = lut.values();
            coords.assign(std::begin(v_coords), std::end(v_coords));
            values.assign(std::begin(v_values), std::end(v_values));
            // Archive
            archive(cereal::make_nvp("Coords", coords),
                    cereal::make_nvp("Values", values));
        }

        // Serialization load method
        template<class Archive, typename T>
        inline void load(Archive & archive, LUT1d<T> & lut) {
            // Create vector for loading results
            std::vector<double> coords;
            std::vector<T> values;
            // Load the archive
            archive(cereal::make_nvp("Coords", coords),
                    cereal::make_nvp("Values", values));
            // Copy vector to LUT valarrays
            std::valarray<double> v_coords(coords.data(), coords.size());
            std::valarray<T> v_values(values.data(), values.size());
            lut.coords(v_coords);
            lut.values(v_values);
        }

        /** \brief Load polynomial coefficients from HDF5 product.
         *
         * @param[in] group         HDF5 group object.
         * @param[in] poly          Poly2d to be configured.
         * @param[in] name          Dataset name within group. */
        template <typename T>
        inline void loadFromH5(isce::io::IGroup & group, LUT1d<T> & lut,
                               std::string name_coords, std::string name_values) {
            // Valarrays for storing results
            std::valarray<double> x, y;
            // Load the LUT values
            isce::io::loadFromH5(group, name_values, y);
            // Load the LUT coordinates
            isce::io::loadFromH5(group, name_coords, x);
            // Set LUT data
            lut.coords(x);
            lut.values(y);
        }

        // ------------------------------------------------------------------------
        // Serialization for StateVector
        // ------------------------------------------------------------------------

        // Serialization save method
        template<class Archive>
        inline void save(Archive & archive, StateVector const & sv) {
            // Archive
            archive(cereal::make_nvp("Time", sv.date().isoformat()),
                    cereal::make_nvp("Position", sv.positionToString()),
                    cereal::make_nvp("Velocity", sv.velocityToString()));
        }

        // Serialization load method
        template<class Archive>
        inline void load(Archive & archive, StateVector & sv) {
            // Make strings for position, velocity, and datetime
            std::string position_string, velocity_string, datetime_string;
            // Load the archive
            archive(cereal::make_nvp("Time", datetime_string),
                    cereal::make_nvp("Position", position_string),
                    cereal::make_nvp("Velocity", velocity_string));
            // Send position/velocity strings to parser
            sv.fromString(position_string, velocity_string);
            // Send datetime string to datetime object parser
            sv.date(datetime_string);
        }
    }
}

#endif

// end of file
