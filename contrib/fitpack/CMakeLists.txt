
enable_language(Fortran)

# FITPACK Fortran source
set(fitpack_SRC
    bispev.f
    fpback.f
    fpbisp.f
    fpchec.f
    fpcurf.f
    fpbspl.f
    fpdisc.f
    fpgivs.f
    fpgrre.f
    fpknot.f
    fporde.f
    fprank.f
    fprati.f
    fpregr.f
    fprota.f
    fpsurf.f
    parder.f
    splder.f
    splev.f
    surfit.f
    regrid.f
)

# Specify the shared library to be built
add_library(fitpack
            SHARED
            ${fitpack_SRC})

# Install shared library
install(TARGETS fitpack
        DESTINATION ${ISCE_LIBDIR}
        COMPONENT isce_libs)

#enable_language(CXX)
#
## C++ interface with Fortran
#include(FortranCInterface)
#FortranCInterface_HEADER(FCMangle.h
#                         MACRO_NAMESPACE "FC_"
#                         SYMBOL_NAMESPACE ""
#                         SYMBOLS bispev regrid)
#FortranCInterface_VERIFY(CXX)


#target_link_libraries(spline fitpack)
