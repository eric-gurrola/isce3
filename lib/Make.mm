# -*- Makefile -*-
#
# eric m. gurrola <eric.m.gurrola@jpl.nasa.gov>
# jet propulsion lab/california institute of technology
# (c) 2017 all rights reserved
#


# project defaults
include isce.def

# the list of directories to visit
RECURSE_DIRS = \
    isce \

# the standard targets

all:
	BLD_ACTION="all" $(MM) recurse

tidy::
	BLD_ACTION="tidy" $(MM) recurse

clean::
	BLD_ACTION="clean" $(MM) recurse

distclean::
	BLD_ACTION="distclean" $(MM) recurse

export::
	BLD_ACTION="export" $(MM) recurse

release::
	BLD_ACTION="release" $(MM) recurse

#  shortcuts to building in subdirectories
.PHONY: $(RECURSE_DIRS)

$(RECURSE_DIRS):
	(cd $@; $(MM))

# end of file
