#! /usr/bin/env python3
# -*- coding: utf-8 -*-
#
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

"""
Attempt to download an SRTM tile that is known to be unavailable
"""

def test():
    # get the module
    import isce.topography.srtm3
    # build a tile
    tile = isce.topography.srtm3.tile(point=(0,0))
    # attempt to
    try:
        # get the data
        contents = tile.download()
    # if we failed because we didn't present authentication credentials
    except tile.AuthenticationError:
        # no worries
        return
    # check that the tile is unavailable
    assert tile.status is tile.availability.unavailable
    # and that the contents are empty
    assert contents == b''

    # all done
    return


# main
if __name__ == "__main__":
    test()

# end of file
