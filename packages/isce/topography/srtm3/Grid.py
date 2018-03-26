# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis
# orthologue
# (c) 2003-2018 all rights reserved
#


# support
import isce.extensions.isce as libisce


# declaration
class Grid:
    """
    Indexed access to the elevation data in an SRTM tile
    """

    # public data
    @property
    def shape(self):
        """
        Return the dimensions of the underlying tile
        """
        # easy enough
        return isce.extensions.isce.srtmTileShape(self.raw)


    # meta-methods
    def __init__(self, hgt, resolution, **kwds):
        # chain up
        super().__init__(**kwds)
        # save a reference to the byte stream so that the tile has a valid buffer to visit
        self.data = hgt
        # the capsule
        self.raw = isce.extensions.isce.srtmTile(hgt, resolution)
        # all done
        return


    def __getitem__(self, index):
        """
        Access the elevation data at the given {index}, assumed to be a pair of non-negative
        integers
        """
        # delegate
        return isce.extensions.isce.srtmTileGet(self.raw, index)


# end of file
