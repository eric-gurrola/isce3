# -*- Python -*-
# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

# get the package
import isce


# topography
@isce.foundry(implements=isce.action, tip="download and assemble digital elevation models")
def dem():
    # get the command panel
    from .Topography import Topography
    # attach the docstring
    __doc__ = Topography.__doc__
    # and return the panel
    return Topography


@isce.foundry(implements=isce.action, tip="manage the local cache of the SRTM data store")
def srtm():
    # get the command panel
    from .SRTM import SRTM
    # attach the docstring
    __doc__ = SRTM.__doc__
    # and return the panel
    return SRTM


# administrivia
@isce.foundry(implements=isce.action, tip="display information about this application")
def about():
    # get the command panel
    from .About import About
    # attach the docstring
    __doc__ = About.__doc__
    # and return the panel
    return About


# end of file
