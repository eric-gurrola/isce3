#! /usr/bin/env python3
# -*- coding: utf-8 -*-
#
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

"""
Build an app with an SRTM component
"""

# grab the package
import isce

# teh app
class SRTM(isce.application, family='isce.applications.srtm'):
    """
    A sample application that uses SRTM as the default elevation model assembler
    """

    # user configurable state
    dem = isce.topography.dem(default=isce.topography.srtm())
    dem.tip = 'the assembler of the digital elevation model'

    # the entry point
    @isce.export
    def main(self, *args, **kwds):
        """
        The main entry point for the application
        """
        # assemble my configuration
        doc = '\n'.join(srtm.pyre_showConfiguration())
        # and show me
        self.debug.log(doc)
        # all done
        return 0


# main
if __name__ == "__main__":
    # make an app
    srtm = SRTM(name='srtm')
    # run it
    status = srtm.run()
    # and share the exit code
    raise SystemExit(status)


# end of file
