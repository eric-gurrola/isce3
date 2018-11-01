# -*- Makefile -*-
# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

# project defaults
include isce.def

# stuff to clean
PROJ_CLEAN += [NS][0-9][0-9][EW][0-9][0-9][0-9].SRTMGL[13].hgt.zip srtmgl[13].map

# standard targets
all: test clean

test: srtm

srtm:
	${PYTHON} ./srtm.py
	${PYTHON} ./srtm_component.py
	${PYTHON} ./srtm_uris.py
	${PYTHON} ./srtm_tile.py
	${PYTHON} ./srtm_tile_download_available.py
	${PYTHON} ./srtm_tile_download_unavailable.py
	${PYTHON} ./srtm_tile_write.py
	${PYTHON} ./srtm_tile_dem.py

# end of file
