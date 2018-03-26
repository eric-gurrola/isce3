# -*- Makefile -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

# project defaults
include isce.def
# package name
PACKAGE = bin
# add this to the clean pile
PROJ_CLEAN += ${addprefix $(EXPORT_BINDIR)/, $(EXPORT_BINS)}

# export these
EXPORT_BINS = \
    isce

# standard targets
all: export

export:: $(EXPORT_BINS) export-binaries tidy

live: live-bin

# end of file
