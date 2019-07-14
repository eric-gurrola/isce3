# -*- Makefile -*-
#
# michael a.g. aïvázis
# orthologue
# (c) 1998-2019 all rights reserved
#

# project meta-data
isce.major := 3
isce.minor := 0

# isce consists of python packages
isce.packages := isce.pkg
# libraries
isce.libraries := isce.lib cereal.lib
# python extensions
isce.extensions := isce.ext
# and test suites
isce.tests :=

# the isce python package
isce.pkg.name := isce3
isce.pkg.root := packages/isce3/
isce.pkg.meta :=
isce.pkg.bin :=
isce.pkg.ext := extensions/

# the isce lib meta-data
isce.lib.root := lib/isce/
isce.lib.stem := isce-$(isce.major).$(isce.minor)
isce.lib.extern := gdal hdf5 mpi fftw pyre
isce.lib.prerequisites := cereal.lib
isce.lib.c++.flags += $($(compiler.c++).std.c++17)

# the isce extension
isce.ext.wraps := isce.lib
isce.ext.root := extensions/cython/isce/
isce.ext.module := isceextension
isce.ext.extern := python
isce.ext.capsule :=

# the cereal lib meta-data
cereal.lib.root := contrib/cereal/include/cereal/
cereal.lib.stem := cereal

# external package configuration
fftw.flavor := 3 3_threads 3f 3f_threads

# end of file
