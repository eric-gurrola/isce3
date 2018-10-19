
# Pull in the source files
include(io/CMakeLists.txt)
include(radar/CMakeLists.txt)
include(product/CMakeLists.txt)
include(core/CMakeLists.txt)
include(image/CMakeLists.txt)
include(geometry/CMakeLists.txt)

# Set the output library
set(PACKAGENAME isce)
set(${PACKAGENAME}_DLL isce.${ISCE_VERSION_MAJOR}.${ISCE_VERSION_MINOR})

# Library top level, public headers
set(${PACKAGENAME}_EXPORT_HEADERS core.h
                                  image.h
                                  isce.h
                                  srtm.h
                                  version.h)


###Install headers into build/include
###This is where regex can be used on the header files if needed
set(${PACKAGENAME}_BUILD_HEADERS "")
foreach(HFILE ${${PACKAGENAME}_EXPORT_HEADERS})
    configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/${HFILE}
        ${ISCE_BUILDINCLUDEDIR}/${LOCALPROJ}/${HFILE}
        COPYONLY
    )
    list(APPEND ${PACKAGENAME}_BUILD_HEADERS ${ISCE_BUILDINCLUDEDIR}/${LOCALPROJ}/${HFILE})
endforeach()

#Install headers as files
###This may be changed in future to install from build/include
install(FILES ${${PACKAGENAME}_BUILD_HEADERS}
    DESTINATION ${ISCE_INCLUDEDIR}/${LOCALPROJ}
    COMPONENT isce_headers)

# Link sources to library
add_library(${${PACKAGENAME}_DLL} SHARED
            ${io_SRCS}
            ${radar_SRCS}
            ${product_SRCS}
            ${core_SRCS}
            ${image_SRCS}
            ${geometry_SRCS})

# Add library dependencies
target_link_libraries(${${PACKAGENAME}_DLL} ${GDAL_LIBRARY} ${HDF5_LIBRARY} ${PYRE_LIBRARY}
                      ${PYRE_JOURNAL_LIBRARY})

# Add OpenMP if found
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

# Install shared library
install(TARGETS ${${PACKAGENAME}_DLL}
        DESTINATION ${ISCE_LIBDIR}
        COMPONENT isce_libs)

# end of file
