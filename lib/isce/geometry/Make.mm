# -*- Makefile -*-

# global project settings
include isce.def
# package isce/geometry
PACKAGE = isce/geometry

# the list of sources
PROJ_SRCS = \
    DEMInterpolator.cpp \
    Geo2rdr.cpp \
    geometry.cpp \
    RTC.cpp \
    Topo.cpp \

# products
# the library
PROJ_DLL = $(BLD_LIBDIR)/lib$(PROJECT).$(PROJECT_MAJOR).$(PROJECT_MINOR).$(EXT_SO)
# the private build location
PROJ_TMPDIR = $(BLD_TMPDIR)/$(PROJECT)-$(PROJECT_MAJOR).$(PROJECT_MINOR)/lib

# what to export
EXPORT_LIBS = $(PROJ_DLL)
# the headers
EXPORT_PKG_HEADERS = \
    DEMInterpolator.h \
    Geo2rdr.h \
    Geo2rdr.icc \
    geometry.h \
    RTC.h \
    Topo.h \
    Topo.icc \
    TopoLayers.h \
    Serialization.h \

# build
PROJ_CXX_INCLUDES += $(EXPORT_ROOT)/include/$(PROJECT)-$(PROJECT_MAJOR).$(PROJECT_MINOR)

# standard targets
all: export

export:: $(PROJ_DLL) export-package-headers export-libraries

live:: live-headers live-package-headers live-libraries
	BLD_ACTION="live" $(MM) recurse

# configuration
# the extension of the c++ sources
EXT_CXX = cpp


# end-of-file
