# -*- Python -*-
# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

# externals
import itertools, math
# support
import isce

# declaration
class Mosaic(list):
    """
    A grid of SRTM tiles
    """

    # types
    from .Tile import Tile as tile


    # interface
    def cover(self, region):
        """
        Make sure this mosaic covers the {region} of interest
        """
        # if this is an empty region
        if not region:
            # give me a trivial shape
            self.shape = (0,0)
            # and reset my NW corner
            self.nw = (None, None)
            # all done
            return

        # we line up the grid in "book order": NW to SE, with the first row W to E, which means
        # that we must treat lats and lons differently; so unpack them
        lats, lons = zip(*region)

        # compute the corners of the bounding box, taking care to project the values inside the
        # valid map region

        # the northernmost latitude
        latN = self.projectLatitude(lat=max(lats))
        # the southernmost latitude
        latS = self.projectLatitude(lat=min(lats))
        # the easternmost longitude
        lonE = self.projectLongitude(lon=max(lons))
        # the westernmost longitude
        lonW = self.projectLongitude(lon=min(lons))

        # save my NW corner
        self.nw = (latN, lonW)
        # set up my shape
        self.shape = (latN - latS + 1, lonE - lonW + 1)
        # get me resolution
        resolution = self.resolution

        # remove everything
        self.clear()
        # build the grid
        self.extend(
            # by adding tiles
            self.tile(point=point, resolution=resolution)
            # at the specified point
            for point in itertools.product(range(latN, latS-1, -1), range(lonW, lonE+1)))

        # all done
        return


    # meta-methods
    def __init__(self, region=[], resolution=True, **kwds):
        """
        Create a grid of SRTM tiles that cover the bounding box of the {region} of interest

          parameters:
            region: a cloud of (lat, lon) pairs
        """
        # chain up
        super().__init__(**kwds)
        # set my resolution
        self.resolution = resolution # arcseconds per pixel
        # figure out the region of interest
        self.cover(region=region)
        # all done
        return


    def __getitem__(self, point):
        """
        Return the tile that contains the (lat, lon) pair in {point}
        """
        # unpack the {point} and convert it into a tile spec
        lat, lon = map(math.floor, point)

        # project inside the valid region
        lat = self.projectLatitude(lat)
        lon = self.projectLongitude(lon)

        # my NW corner tile is at grid (0,0); get its (lat, lon)
        latNW, lonNW = self.nw
        # my shape
        rows, cols = self.shape

        # form the grid index
        row, col = (latNW - lat), (lon - lonNW)

        # check the latitude
        if row < 0 or row >= rows:
            # complain
            raise ValueError('latitude {} not in this mosaic'.format(lat))
        # check the longitude
        if col < 0 or col >= cols:
            # complain
            raise ValueError('longitude {} not in this mosaic'.format(lon))

        # convert the index into an offset
        offset = row * cols + col
        # and fetch the tile
        return super().__getitem__(offset)


    # implementation details
    def projectLatitude(self, lat):
        """
        Project {lat} inside the valid map latitude values
        """
        # the range is [-90, 89]
        return min(89, max(-90, math.floor(lat)))


    def projectLongitude(self, lon):
        """
        Project {lon} inside the valid map longitude values
        """
        # the range is [-90, 89]
        return min(179, max(-180, math.floor(lon)))


    # private data
    # the (lat, lon) of the tile at the NW corner of the mosaic
    nw = None
    # the shape of the mosaic
    shape = None
    # the encoding of the mosaic resolution
    resolution = None


# end of file
