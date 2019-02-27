#cython: language_level=3
#
# Author: Joshua Cohen, Bryan V. Riel
# Copyright 2017-2018
#

cimport cython
import numpy as np
cimport numpy as np
from libcpp cimport bool
from libcpp.vector cimport vector
from Interpolator cimport *
from Matrix cimport Matrix, valarray

cdef valarray[double] numpyToValarray(np.ndarray[double, ndim=1] a):
    """
    Utility function to create an std::valarray<double> copy of a numpy array.
    """
    cdef int i
    cdef int n = a.shape[0]
    cdef valarray[double] v = valarray[double](n)
    for i in range(n):
        v[i] = a[i]
    return v

cdef np.ndarray[double, ndim=1] valarrayToNumpy(valarray[double] & v):
    cdef int i
    cdef int n = v.size()
    cdef np.ndarray[double, ndim=1] v_np = np.zeros(n)
    for i in range(n):
        v_np[i] = v[i]
    return v_np

cdef Matrix[double] numpyToMatrix(np.ndarray[double, ndim=2] a):
    """
    Utility function to create an isce::core::Matrix 'view' of a numpy array.
    """
    cdef int nrows, ncols
    nrows, ncols = a.shape[0], a.shape[1] 
    return Matrix[double](&a[0,0], nrows, ncols)

cdef class pyInterpolator:
    """
    Cython class for creating and calling all Interpolator classes.

    Args:
        method (Optional[str]): Interpolation method to use from
                                ('bilinear', 'bicubic', 'spline')
        order (Optional[int]): Order of 2D spline if using spline interpolator.
    """
    cdef int order
    cdef string method

    def __init__(self, method='bilinear', order=6):

        # Validate the method
        assert method in ('bilinear', 'bicubic', 'spline'), \
            'Unsupported interpolation method'
        self.method = pyStringToBytes(method)

        # Validate the order (for spline only)
        assert order > 2 and order < 21, 'Invalid interpolation order'
        self.order = order
        
    def interpolate(self, x, y, np.ndarray[double, ndim=2] z):
        """
        Interpolate at specified coordinates.

        Args:
            x (float or ndarray): X coordinates at which to interpolate
            y (float or ndarray): Y coordinates at which to interpolate
            z (ndarray): 2D array of values to interpolate at x and y coordinates

        Returns:
            values (float or ndarray): Interpolated values
        """
        # Convert numpy array to isce::core::Matrix
        cdef Matrix[double] zmat = numpyToMatrix(z)

        # Make sure coordinates are numpy arrays
        cdef np.ndarray[double, ndim=1] x_np = np.array(x).squeeze()
        cdef np.ndarray[double, ndim=1] y_np = np.array(y).squeeze()
        cdef np.ndarray[double, ndim=1] values = np.empty_like(x_np, dtype=np.float64)
        cdef int n_pts = x_np.shape[0]
        
        # Dynamically create interpolation object
        cdef Interpolator[double] * c_interp;
        if self.method == b'bilinear':
            c_interp = new BilinearInterpolator[double]()
        elif self.method == b'bicubic':
            c_interp = new BicubicInterpolator[double]()
        elif self.method == b'spline':
            c_interp = new Spline2dInterpolator[double](self.order)

        # Call interpolator for all points
        cdef int i
        for i in range(n_pts):
            values[i] = c_interp.interpolate(x_np[i], y_np[i], zmat)

        # Done
        del c_interp
        return values.squeeze()

    #def quadInterpolate(self, x, y, double xintp):
    #    cdef int i, N
    #    N = x.size
    #    cdef valarray[double] x_array = valarray[double](N)
    #    cdef valarray[double] y_array = valarray[double](N)
    #    for i in range(N):
    #        x_array[i] = x[i]
    #        y_array[i] = y[i]
    #    return self.c_interp.quadInterpolate(x_array, y_array, xintp)

    #def akima(self, double x, double y, np.ndarray[np.float32_t, ndim=2] c):
    #    cdef Matrix[float] cmat = Matrix[float](c.shape[0], c.shape[1])
    #    numpyToMatrixFloat(c, cmat)
    #    return self.c_interp.akima(x, y, cmat)

# end of file
