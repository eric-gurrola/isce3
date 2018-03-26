# -*- Makefile -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

# project defaults
include isce.def
# package name
PACKAGE = topography/srtm3

# the python modules
EXPORT_PYTHON_MODULES = \
    Availability.py \
    AvailabilityMap.py \
    Grid.py \
    Mosaic.py \
    Retriever.py \
    SRTM.py \
    Tile.py \
    exceptions.py \
    __init__.py

# standard targets
all: export

export:: export-package-python-modules

live: live-package-python-modules

# end of file
