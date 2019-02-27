#cython: language_level=3
#
# Author: Bryan V. Riel
# Copyright 2018
#

from libcpp cimport bool
from EulerAngles cimport EulerAngles

cdef class pyEulerAngles:
    cdef EulerAngles * c_eulerangles
    cdef bool __owner

# end of file
