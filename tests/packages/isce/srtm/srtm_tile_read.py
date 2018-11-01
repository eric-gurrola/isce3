#! /usr/bin/env python3
# -*- coding: utf-8 -*-
#
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

"""
Read the local copy of an SRTM tile
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

    # cache the compressed stream
    contents = tile.read(cache=cwd)
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
