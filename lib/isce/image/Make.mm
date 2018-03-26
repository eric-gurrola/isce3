# -*- Makefile -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

# project defaults
include isce.def
# the name of the package
PACKAGE = image

# the sources
PROJ_SRCS =

# the private build space
PROJ_TMPDIR = $(BLD_TMPDIR)/${PROJECT}/lib/$(PROJECT)
# what to clean
PROJ_CLEAN += $(EXPORT_INCDIR)/$(PACKAGE)

# what to export
# the package headers
EXPORT_PKG_HEADERS = \
    DirectImage.h DirectImage.icc \
    Image.h Image.icc \
    public.h


# the standard targets
all: export

export:: export-package-headers

# end of file
