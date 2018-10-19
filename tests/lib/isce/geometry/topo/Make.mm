# -*- Makefile -*-
#
# Bryan V. Riel
# (c) 2017 all rights reserved
#

# project defaults
include isce.def

PROJ_CLEAN += \
    hdg.rdr \
    hdg.rdr.xml \
    inc.rdr \
    inc.rdr.xml \
    localInc.rdr \
    localInc.rdr.xml \
    localPsi.rdr \
    localPsi.rdr.xml \
    simamp.rdr \
    simamp.rdr.xml \
#    x.rdr \
#    x.rdr.xml \
#    y.rdr \
#    y.rdr.xml \
#    z.rdr \
#    z.rdr.xml \
#    topo.vrt

# the pile of tests
TESTS = \
    topo \

all: test clean

# testing
test: $(TESTS)
	@echo "testing:"
	@for testcase in $(TESTS); do { \
            echo "    $${testcase}" ; \
            ./$${testcase} || exit 1 ; \
            } done

# build
PROJ_CLEAN += $(TESTS)
PROJ_CXX_INCLUDES += $(EXPORT_ROOT)/include/$(PROJECT)-$(PROJECT_MAJOR).$(PROJECT_MINOR)
PROJ_LIBRARIES = -lisce.$(PROJECT_MAJOR).$(PROJECT_MINOR) -lgtest
LIBRARIES = $(PROJ_LIBRARIES) $(EXTERNAL_LIBS)

%: %.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LCXXFLAGS) $(LIBRARIES)

# end of file
