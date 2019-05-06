#cython: language_level=3
#
# Author: Bryan Riel, Heresh Fattahi, Liang Yu
# Copyright 2017-2019
#

from libcpp cimport bool
from cython.operator cimport dereference as deref

from LUT1d cimport LUT1d

from cuCrossmul cimport gpuCrossmul

cdef class pyCrossmul:
    '''
    Python wrapper for isce::signal::cuda::gpuCrossmul

    Args:

    '''
    cdef gpuCrossmul * c_crossmul
    cdef bool __owner

    def __cinit__(self):
        self.c_crossmul = new gpuCrossmul()
        self.__owner = True

    def __dealloc__(self):
        if self.__owner:
            del self.c_crossmul

    @staticmethod
    def bind(pyCrossmul crossmul):
        new_crossmul = pyCrossmul()
        del new_crossmul.c_crossmul
        new_crossmul.c_crossmul = crossmul.c_crossmul
        new_crossmul.__owner = False
        return new_crossmul

    # Run crossmul
    def crossmul(self, pyRaster referenceSLC, pyRaster secondarySLC,
                 pyRaster interferogram, pyRaster coherence, pyRaster rngOffset=None,
                 refDoppler=None, secDoppler=None, int rangeLooks=1, int azimuthLooks=1,
                 double prf=1.0, double azimuthBandwidth=1.0):
        '''
        Run crossmul to generate interferogram and coherence image.

        Args:

        Returns:
            None
        '''
        # Check if dopplers are provided for azimuth commonband filtering
        cdef pyLUT2d refdoppler2d
        cdef pyLUT2d secdoppler2d
        cdef LUT1d[double] c_refdoppler1d
        cdef LUT1d[double] c_secdoppler1d
        
        if refDoppler is not None and secDoppler is not None:

            # Convert Dopplers to LUT1d
            refdoppler = <pyLUT2d> refDoppler
            secdoppler = <pyLUT2d> secDoppler
            c_refdoppler1d = LUT1d[double](deref(refdoppler.c_lut))
            c_secdoppler1d = LUT1d[double](deref(secdoppler.c_lut))

            # Set the dopplers
            self.c_crossmul.doppler(c_refdoppler1d, c_secdoppler1d)
            self.c_crossmul.doCommonAzimuthBandFiltering(True)

            # Set the PRF
            self.c_crossmul.prf(prf)

            # Set the azimuth bandwidth
            self.c_crossmul.commonAzimuthBandwidth(azimuthBandwidth)

        # Set the number of looks
        if rangeLooks > 1:
            self.c_crossmul.rangeLooks(rangeLooks)
        if azimuthLooks > 1:
            self.c_crossmul.azimuthLooks(azimuthLooks)

        # If range offset raster provided, run crossmul with range commonband filtering
        if rngOffset is not None:
            self.c_crossmul.doCommonRangeBandFiltering(True)
            self.c_crossmul.crossmul(
                deref(referenceSLC.c_raster),
                deref(secondarySLC.c_raster),
                deref(rngOffset.c_raster),
                deref(interferogram.c_raster),
                deref(coherence.c_raster)
            )

        # Else, run normal crossmul
        else:
            self.c_crossmul.crossmul(
                deref(referenceSLC.c_raster),
                deref(secondarySLC.c_raster),
                deref(interferogram.c_raster),
                deref(coherence.c_raster)
            )

        return

# end of file 
