# -*- Makefile -*-
# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

# project defaults
include isce.def
# the package
PACKAGE = extensions
# the module
MODULE = isce

# build a python extension
include std-pythonmodule.def

# adjust the build parameters
PROJ_LCXX_LIBPATH=$(BLD_LIBDIR)
# the list of extension source files
PROJ_SRCS = \
    exceptions.cc \
    metadata.cc \
    srtm.cc \

# the private build space
PROJ_TMPDIR = $(BLD_TMPDIR)/${PROJECT}/extensions/isce
# my dependencies
PROJ_LIBRARIES += -lisce -lpyre -ljournal $(LCXX_FORTRAN)

# the pile of things to clean
PROJ_CLEAN += \
    $(PROJ_CXX_LIB) \
    $(MODULE_DLL) \
    $(EXPORT_BINDIR)/$(MODULE)$(MODULE_SUFFIX)

# end of file
