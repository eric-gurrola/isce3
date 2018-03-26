# -*- Python -*-
# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#


# externals
import enum


# tile status
@enum.unique
class Availability(enum.Enum):
    """
    Tile status codes
    """

    unknown = 0     # we have never attempted to get this tile, so we know nothing about it
    unavailable = 1 # we know for a fact that the SRTM data set does not include this tile
    available = 2   # we know for a fact that the SRTM data set includes this tile
    cached = 3      # we have a local copy of this tile


# end of file
