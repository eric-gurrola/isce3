#! /usr/bin/env python3
# -*- coding: utf-8 -*-
#
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

"""
Download an SRTM tile that is known to be available
"""

def test():
    # get the module
    import isce.topography.srtm3
    # build a tile
    tile = isce.topography.srtm3.tile(point=(34,-118))
    # attempt to
    try:
        # get the data
        contents = tile.download()
    # if we failed because we didn't present authentication credentials
    except tile.AuthenticationError:
        # no worries
        return
    # check that the tile is available
    assert tile.status is tile.availability.available
    # decompress
    dem = tile.decompress(contents=contents)
    # verify that the contents are the right size
    assert len(dem) == 2*3601*3601

    # all done
    return


# main
if __name__ == "__main__":
    test()

# end of file
