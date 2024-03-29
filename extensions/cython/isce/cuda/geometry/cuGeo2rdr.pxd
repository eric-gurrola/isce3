#cython: language_level=3
#
# Author: Bryan Riel
# Copyright 2017-2018
#

from libcpp cimport bool
from libcpp.string cimport string

# Cython declarations for isce::core objects
from isceextension cimport Raster
from isceextension cimport Product
from isceextension cimport orbitInterpMethod

cdef extern from "isce/cuda/geometry/Geo2rdr.h" namespace "isce::cuda::geometry":

    # Geo2rdr class
    cdef cppclass Geo2rdr:

        # Constructor
        Geo2rdr(Product & product, char frequency, bool nativeDoppler,
                size_t numberAzimuthLooks, size_t numberRangeLooks) except +

        # Set options
        void threshold(double)
        void numiter(int);
        void orbitMethod(orbitInterpMethod)

        # Run geo2rdr with offsets and internally created offset rasters
        void geo2rdr(Raster &, const string &, double, double)

        # Run geo2rdr with offsets and externally created offset rasters
        void geo2rdr(Raster &, Raster &, Raster &, double, double)

# end of file
