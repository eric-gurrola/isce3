FILE(GLOB PXDS *.pxd)
add_isce_cythondir(serialization "${PXDS}")
