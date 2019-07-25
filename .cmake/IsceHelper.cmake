#Find pyre and journal libraries
find_library(LPYRE pyre HINTS ${PYRE_LIB_DIR})
find_library(LJOURNAL journal HINTS ${PYRE_LIB_DIR})

# Create a new ctest for TESTNAME.cpp
# Additional include directories can be specified after TESTNAME
function(add_isce_test TESTNAME)
    # Only enable for the specified build configurations
    if(ARGN)
        if(NOT ${CMAKE_BUILD_TYPE} IN_LIST ARGN)
            # Otherwise create a dummy target and bail
            add_custom_target(${TESTNAME})
            return()
        endif()
    endif()

    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${TESTNAME}.cpp)
        add_executable(${TESTNAME} ${TESTNAME}.cpp)
    elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${TESTNAME}.cu)
        add_executable(${TESTNAME} ${TESTNAME}.cu)
    else()
        message(FATAL_ERROR "Could not add isce test for ${TESTNAME}.{cpp,cu}")
    endif()
    target_link_libraries(${TESTNAME} PUBLIC ${LISCE} ${LPYRE} ${LJOURNAL} gtest)
    # If we're compiling against the CUDA ISCE library...
    if(DEFINED LISCECUDA)
        # Make CUDA libraries and headers available
        target_link_libraries(${TESTNAME} PUBLIC ${LISCECUDA})
        target_include_directories(${TESTNAME} PUBLIC ${ISCE_BUILDINCLUDEDIR_CUDA})
    endif()

    target_include_directories(${TESTNAME} PUBLIC
        ${ISCE_BUILDINCLUDEDIR}
        ${GDAL_INCLUDE_DIR}
        ${HDF5_INCLUDE_DIR}
        ${PYRE_INCLUDE_DIR}
        ${ISCE_SOURCE_DIR}/contrib/cereal/include
        ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
        gtest)
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
endfunction()

# Add a library subdirectory, containing source .cpp files and headers
function(add_isce_libdir PKGNAME CPPS HEADERS)
    string(TOLOWER ${PROJECT_NAME} LOCALPROJ)

    # Prefix current path to each source file
    unset(SRCS)
    foreach(CPP ${CPPS})
        list(APPEND SRCS ${CMAKE_CURRENT_LIST_DIR}/${CPP})
    endforeach()

    # Add sources to library build requirements
    unset(SUBDIR)
    if(DEFINED LISCECUDA)
        set(SUBDIR "cuda")
        target_sources(${LISCECUDA} PRIVATE ${SRCS})
    else()
        target_sources(${LISCE} PRIVATE ${SRCS})
    endif()

    # Install headers to build/include
    # This is where regex can be used on headers if needed
    unset(BUILD_HEADERS)
    foreach(HFILE ${HEADERS})
        list(APPEND BUILD_HEADERS "${CMAKE_CURRENT_LIST_DIR}/${HFILE}")
    endforeach()

    # Install headers as files
    # May be changed in the future to install from build/include
    install(FILES ${BUILD_HEADERS}
            DESTINATION ${ISCE_INCLUDEDIR}/${LOCALPROJ}/${SUBDIR}/${PKGNAME}
            COMPONENT isce_headers)
endfunction()

# Add a Cython source directory for exporting pxd (header) files
function(add_isce_cythondir PKGNAME PXDS)
    # Install headers as files
    # May be changed in the future to install from build/include
    install(FILES ${PXDS}
            DESTINATION ${ISCE_PACKAGESDIR}/isce3/include/${PKGNAME}
            COMPONENT isce_headers)
endfunction()
