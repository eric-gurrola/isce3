# -*- Makefile -*-
#
# eric m. gurrola
# jet propulsion laboratory
# california institute of technology
# (c) 2013 all rights reserved
#


PROJECT = isce

#--------------------------------------------------------------------------
#

all: test

test: #estimate_dop

estimate_dop:
	${PYTHON} ./estimate_dop.py


# the list of directories to visit
RECURSE_DIRS = \
    isce \

#--------------------------------------------------------------------------
# the recursive targets

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

#--------------------------------------------------------------------------
#  shortcuts to building in subdirectories

$(RECURSE_DIRS):
	(cd $@; $(MM))

# end of file
