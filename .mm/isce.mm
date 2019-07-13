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
isce.packages :=
# libraries
isce.libraries := isce.lib cereal.lib
# python extensions
isce.extensions :=
# and test suites
isce.tests :=

# the isce lib meta-data
isce.lib.root := lib/isce/
isce.lib.stem := isce
isce.lib.extern := mpi hdf5 fftw pyre
isce.lib.prerequisites := cereal.lib

# the cereal lib meta-data
cereal.lib.root := contrib/cereal/include/cereal/
cereal.lib.stem := cereal

# external package configuration
fftw.flavor := 3 3_threads 3f 3f_threads

# end of file
