# -*- Makefile -*-
#
# eric m. gurrola <eric.m.gurola@jpl.nasa.gov>
# (c) 2017-2018 all rights reserved

# global project settings
include isce.def
# package isce/core
PACKAGE = isce/core

# configuration
# the extension of the c++ sources
EXT_CXX = cpp

# the list of sources
PROJ_SRCS = \
    Attitude.cpp \
    Baseline.cpp \
    DateTime.cpp \
    Doppler.cpp \
    Ellipsoid.cpp \
    Interpolator.cpp \
    LUT2d.cpp \
    LinAlg.cpp \
    Metadata.cpp \
    Orbit.cpp \
    Peg.cpp \
    Pegtrans.cpp \
    Poly1d.cpp \
    Poly2d.cpp \
    Position.cpp \
    Projections.cpp \

# products
PROJ_SAR = $(BLD_LIBDIR)/lib$(PROJECT).$(PROJECT_MAJOR).$(PROJECT_MINOR).$(EXT_SAR)
PROJ_DLL = $(BLD_LIBDIR)/lib$(PROJECT).$(PROJECT_MAJOR).$(PROJECT_MINOR).$(EXT_SO)

# the private build space
PROJ_TMPDIR = $(BLD_TMPDIR)/${PROJECT}/lib/$(PROJECT)

# what to clean
PROJ_CLEAN += $(EXPORT_INCDIR)/$(PACKAGE)

# what to export
# the library
EXPORT_LIBS = $(PROJ_DLL)
# the headers
EXPORT_PKG_HEADERS = \
    public.h \
    Attitude.h \
    Baseline.h \
    Constants.h \
    DateTime.h \
    Doppler.h \
    Ellipsoid.h \
    Interpolator.h \
    LUT2d.h \
    LinAlg.h \
    Metadata.h \
    Orbit.h \
    Peg.h \
    Pegtrans.h \
    Poly1d.h \
    Poly2d.h \
    Position.h \
    Projections.h \
    Serialization.h \

# standard targets
all: export

export:: export-package-headers $(PROJ_DLL) export-libraries


# end-of-file
