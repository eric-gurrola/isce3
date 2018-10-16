#cython: language_level=3
#
# Author: Bryan V. Riel
# Copyright 2018
#

from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp cimport bool

from Orbit cimport Orbit
from Ellipsoid cimport Ellipsoid
from Attitude cimport Attitude

cdef extern from "isce/core/Doppler.h" namespace "isce::core":
    cdef cppclass Doppler:
        vector[double] satxyz
        vector[double] satvel
        vector[double] satllh
        Doppler(Orbit &, Attitude *, Ellipsoid &, double) except +
        double centroid(double, double, string, int, int, bool)

# end of file
