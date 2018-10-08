#cython: language_level=3
#
# Author: Bryan V. Riel
# Copyright 2017-2018
#

from libcpp.string cimport string
from IH5 cimport IH5File

from ComplexImagery cimport ComplexImagery
from Metadata cimport Metadata

cdef extern from "isce/product/Product.h" namespace "isce::product":

    # Product class
    cdef cppclass Product:

        # Constructors
        Product(IH5File &) except +

        # Complex imagery
        ComplexImagery & complexImagery()
    
        # Metadata
        Metadata & metadata()

        # The filename of the HDF5 file
        string filename()

# end of file
