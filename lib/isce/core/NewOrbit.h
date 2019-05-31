#pragma once
#ifndef ISCE_CORE_ORBIT_H
#define ISCE_CORE_ORBIT_H

#include <vector>

#include "DateTime.h"
#include "Linspace.h"
#include "StateVector.h"
#include "Vector.h"

namespace isce { namespace core {

// Proxy class for getting/setting StateVectors in an Orbit
struct OrbitPoint {
    OrbitPoint & operator=(const StateVector &);

    operator StateVector() const;

    const DateTime datetime;
    Vec3 & position;
    Vec3 & velocity;

private:
    OrbitPoint(const DateTime & datetime, Vec3 & position, Vec3 & velocity);

    friend class NewOrbit;
};

bool operator==(const OrbitPoint &, const StateVector &);
bool operator==(const StateVector &, const OrbitPoint &);
bool operator!=(const OrbitPoint &, const StateVector &);
bool operator!=(const StateVector &, const OrbitPoint &);

// Proxy class for getting StateVectors from a const Orbit
struct const_OrbitPoint {
    operator StateVector() const;

    const DateTime datetime;
    const Vec3 & position;
    const Vec3 & velocity;

private:
    const_OrbitPoint(const DateTime & datetime,
                     const Vec3 & position,
                     const Vec3 & velocity);

    friend class NewOrbit;
};

bool operator==(const const_OrbitPoint &, const StateVector &);
bool operator==(const StateVector &, const const_OrbitPoint &);
bool operator!=(const const_OrbitPoint &, const StateVector &);
bool operator!=(const StateVector &, const const_OrbitPoint &);

bool operator==(const const_OrbitPoint &, const OrbitPoint &);
bool operator==(const OrbitPoint &, const const_OrbitPoint &);
bool operator!=(const const_OrbitPoint &, const OrbitPoint &);
bool operator!=(const OrbitPoint &, const const_OrbitPoint &);

/**
 * Container for a set of platform position and velocity state vectors
 * sampled uniformly in time.
 */
class NewOrbit {
public:
    /** Create Orbit from vector of uniformly spacing StateVectors. */
    static
    NewOrbit from_statevectors(const std::vector<StateVector> &);

    NewOrbit() = default;

    /**
     * Constructor
     *
     * @param[in] refepoch datetime of the initial state vector sample
     * @param[in] spacing time interval between state vectors
     * @param[in] size number of state vectors
     */
    NewOrbit(const DateTime & refepoch, const TimeDelta & spacing, int size = 0);

    OrbitPoint operator[](int idx);

    const_OrbitPoint operator[](int idx) const;

    /** Return datetime of the first state vector. */
    const DateTime & refepoch() const;

    /** Return time interval between state vectors. */
    TimeDelta spacing() const;

    /** Return number of state vectors. */
    int size() const;

    /** Return sequence of timepoints relative to reference epoch. */
    const Linspace<double> & time() const;

    /** Return sequence of platform positions at each timepoint. */
    const std::vector<Vec3> & position() const;

    /** Return sequence of platform velocities at each timepoint. */
    const std::vector<Vec3> & velocity() const;

    /** Append a new state vector. */
    void push_back(const StateVector &);

    /** Resize the container. */
    void resize(int size);

    /** Check if there are no state vectors in the sequence. */
    bool empty() const;

private:
    DateTime _refepoch;
    Linspace<double> _time;
    std::vector<Vec3> _position;
    std::vector<Vec3> _velocity;
};

bool operator==(const NewOrbit &, const NewOrbit &);
bool operator!=(const NewOrbit &, const NewOrbit &);

}}

#define ISCE_CORE_ORBIT_ICC
#include "NewOrbit.icc"
#undef ISCE_CORE_ORBIT_ICC

#endif

