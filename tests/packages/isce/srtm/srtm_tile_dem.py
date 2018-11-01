#! /usr/bin/env python3
# -*- coding: utf-8 -*-
#
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

"""
Sanity check: verify that the SRTM tile constructor is accessible
"""

def test():
    # get the module
    import isce.topography.srtm3
    # build a tile
    tile = isce.topography.srtm3.tile(point=(34,-118))

    # get the filesrver
    fs = isce.executive.fileserver
    # get the cwd
    cwd = fs[fs.STARTUP_DIR]

    # attempt to
    try:
        # read the compressed stream
        contents = tile.read(cache=cwd)
    # the file may not be here since the SRTM data archive now requires authentication
    except cwd.NotFoundError:
        # no worries
        return

    # decompress
    hgt = tile.decompress(contents=contents)

    # make a grid
    dem = tile.dem(hgt=hgt)
    # check that we can access the low level object correctly
    assert dem.shape == (3601, 3601)

    # all done
    return


# main
if __name__ == "__main__":
    test()

# end of file
