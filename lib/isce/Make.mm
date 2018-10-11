# -*- Makefile -*-
#
# eric m. gurrola <eric.m.gurrola@jpl.nasa.gov>
# Jet Propulsion Lab/Caltech
# (c) 2017 all rights reserved
#

# project defaults
include isce.def

# my subdirectories
RECURSE_DIRS = \
    $(PACKAGES)

# the ones that are always available
PACKAGES = \
    io \
    radar \
    product \
    core \
    image \
    geometry \

# the products
PROJ_DLL = $(BLD_LIBDIR)/lib$(PROJECT).$(PROJECT_MAJOR).$(PROJECT_MINOR).$(EXT_SO)
# the private build space
PROJ_TMPDIR = $(BLD_TMPDIR)/$(PROJECT)-$(PROJECT_MAJOR).$(PROJECT_MINOR)/lib/$(PROJECT)
# what to clean
PROJ_CLEAN += $(EXPORT_LIBS) $(EXPORT_INCDIR)
#LIBRARIES = $(EXTERNAL_LIBS)

# the sources
PROJ_SRCS = \
    version.cc \

# what to export
# the library
EXPORT_LIBS = $(PROJ_DLL)

PACKAGE = isce

# the products
PROJ_SAR = $(BLD_LIBDIR)/lib$(PROJECT).$(PROJECT_MAJOR).$(PROJECT_MINOR).$(EXT_SAR)
PROJ_DLL = $(BLD_LIBDIR)/lib$(PROJECT).$(PROJECT_MAJOR).$(PROJECT_MINOR).$(EXT_SO)
# the private build space
PROJ_TMPDIR = $(BLD_TMPDIR)/${PROJECT}/lib/$(PROJECT)

# what to clean
PROJ_CLEAN += $(EXPORT_LIBS) $(EXPORT_INCDIR)

# what to export
# the library
EXPORT_LIBS = $(PROJ_DLL)
# the top level headers
EXPORT_HEADERS = \
    core.h \
    image.h \
    isce.h \
    srtm.h \
    version.h \

# get today's date
TODAY = ${strip ${shell date -u}}
# get revision information (such as can be constructed for git)
#REVISION = ${strip ${shell bzr revno}}
C = ${strip ${shell git rev-list --full-history --all --abbrev-commit | wc -l | sed -e 's/^ *//'}}
H = ${strip ${shell git rev-list --full-history --all --abbrev-commit | head -1}}
REVISION = $C #:$H
# if not there
ifeq ($(REVISION),)
REVISION = 0
endif

# the standard targets
all: export

# project settings: do not remove core directory (core usually refers core dump file)
# filter-out info at: https://www.gnu.org/software/make/manual/html_node/index.html
PROJ_TIDY := ${filter-out core, $(PROJ_TIDY)}

export:: version.cc $(PROJ_DLL) export-headers export-package-headers export-libraries
	BLD_ACTION="export" $(MM) recurse
	@$(RM) version.cc

tidy::
        BLD_ACTION="tidy" $(MM) recurse

clean::
	BLD_ACTION="clean" $(MM) recurse

distclean::
	BLD_ACTION="distclean" $(MM) recurse

revision: version.cc $(PROJ_DLL) export-libraries
	@$(RM) version.cc

# construct my {version.cc}
#          -e "s|TODAY|$(TODAY)|g"  #

REVISION = ${strip ${shell git log --format=format:"%h" -n 1}}
version.cc: version Make.mm
	@sed \
          -e "s:MAJOR:$(PROJECT_MAJOR):g" \
          -e "s:MINOR:$(PROJECT_MINOR):g" \
          -e "s:REVISION:'$(REVISION)':g" \
          -e "s|TODAY|$(TODAY)|g" \
          version > version.cc

live:
	BLD_ACTION="live" $(MM) recurse

# end of file
