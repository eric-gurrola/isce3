# -*- Makefile -*-
#
# eric m. gurrola
# jet propulsion lab/california institute of technology
# (c) 2017 all rights reserved
#

# project defaults
include isce.def
# package name
PACKAGE = extensions
# the python modules
EXPORT_PYTHON_MODULES = \
    __init__.py

# standard targets
all: export

export:: export-package-python-modules
	BLD_ACTION="export" $(MM) recurse

live: live-package-python-modules

# the list of directories to visit
RECURSE_DIRS = \


# standard targets
all: export

export:: export-package-python-modules

live: live-package-python-modules

# end of file
