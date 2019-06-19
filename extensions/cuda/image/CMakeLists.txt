set(HEADERS
        ResampSlc.h)

set(SRCS
        ResampSlc.cu)

# end of file
add_isce_libdir(image "${SRCS}" "${HEADERS}")
