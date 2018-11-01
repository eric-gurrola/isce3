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
    tile = isce.topography.srtm3.tile(point=(0,0))

    # let's check the basics
    assert tile.name == 'N00E000'
    assert tile.filename == 'N00E000.SRTMGL1.hgt.zip'

    # all done
    return


# main
if __name__ == "__main__":
    test()

# end of file
