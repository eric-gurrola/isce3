
set(LISCE isce.${ISCE_VERSION_MAJOR}.${ISCE_VERSION_MINOR})

foreach (TESTNAME interp1d)
    add_executable(${TESTNAME} ${TESTNAME}.cpp)
    target_link_libraries(${TESTNAME} PUBLIC ${LISCE} gtest)
    target_include_directories(${TESTNAME} PUBLIC
        ${ISCE_BUILDINCLUDEDIR}
        ${GDAL_INCLUDE_DIR}
        ${PYRE_INCLUDE_DIR})
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
endforeach ()

include(CTest)
