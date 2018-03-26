# -*- Makefile -*-
#
# eric m. gurrola
# jet propulsion lab/california institute of technology
# (c) 2017 all rights reserved
#

# get the global defaults
include isce.def
# the package name
PACKAGE = isce
# clean up
PROJ_CLEAN += $(EXPORT_MODULEDIR)

# the list of directories to visit
RECURSE_DIRS = \
    actions \
    extensions \
    shells \
    topography \

# the list of python modules
EXPORT_PYTHON_MODULES = \
    exceptions.py \
    meta.py \
    __init__.py

# get today's date
YEAR = ${strip ${shell date '+%Y'}}
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

#--------------------------------------------------------------------------
# the standard targets

all: export

tidy::
	BLD_ACTION="tidy" $(MM) recurse

clean::
	BLD_ACTION="clean" $(MM) recurse

distclean::
	BLD_ACTION="distclean" $(MM) recurse

export:: meta.py export-python-modules
	BLD_ACTION="export" $(MM) recurse
	@$(RM) meta.py

revision:: meta.py export-python-modules
	@$(RM) meta.py

live: live-python-modules
	BLD_ACTION="live" $(MM) recurse

# construct my {meta.py}
meta.py: meta Make.mm
	@sed \
          -e "s:MAJOR:$(PROJECT_MAJOR):g" \
          -e "s:MINOR:$(PROJECT_MINOR):g" \
          -e "s:REVISION:$(REVISION):g" \
          -e "s|YEAR|$(YEAR)|g" \
          -e "s|TODAY|$(TODAY)|g" \
          meta > meta.py


#--------------------------------------------------------------------------
#  shortcuts to building in subdirectories
.PHONY: $(RECURSE_DIRS)

$(RECURSE_DIRS):
	(cd $@; $(MM))

# end of file
