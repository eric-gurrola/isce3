# -*- Python -*-
# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

# get the package
import isce

# grab the protocol
from .DEM import DEM as dem


# SRTMv3 support
@isce.foundry(implements=dem, tip='assembles a DEM using SRTMv3 tiles')
def srtm():
    """
    Component that assembles digital elevation models from SRTMv3 tiles
    """
    # grab the factory
    from . import srtm3
    # and return it
    return srtm3.srtm


# end of file
