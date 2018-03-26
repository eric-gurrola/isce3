#! /usr/bin/env python3
# -*- coding: utf-8 -*-
#
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

"""
Sanity check: verify that the SRTM component is accessible
"""

def test():
    # get the component
    import isce.topography.srtm3
    # build an instance of the archive accessor
    srtm = isce.topography.srtm3.srtm(name='srtm')

    # assemble its configuration
    doc = '\n'.join(srtm.pyre_showConfiguration())
    # grab the journal
    import journal
    # make a channel and show me the configuration
    journal.debug("isce.topography").log(doc)

    # all done
    return


# main
if __name__ == "__main__":
    test()

# end of file
