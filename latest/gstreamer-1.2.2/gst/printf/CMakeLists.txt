# CMAKE_C_FLAGS CMAKE_CXX_FLAGS
add_definitions(-O2 -Wall)
add_definitions(-DHAVE_CONFIG_H) 
add_definitions(
    -DSTATIC=G_GNUC_INTERNAL
    -UHAVE_LONG_DOUBLE
    -UHAVE_LONG_LONG_FORMAT
    -UHAVE_WCHAR_T
    -UHAVE_WCSLEN
    -UHAVE_WINT_T
)

include_directories(
    .
)

link_directories(
)

set(gstprintf_LIB_SRCS
	asnprintf.c 
	printf-args.c 
	printf-args.h 
	printf-parse.c 
	printf-parse.h 
	vasnprintf.c 
	vasnprintf.h 
	printf.c 
	printf.h 
	printf-extension.c 
	printf-extension.h 
	gst-printf.h
)

set(a_NAME gstprintf)
add_library(${a_NAME} STATIC ${gstprintf_LIB_SRCS})
set_target_properties(${a_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/) 
install(TARGETS ${a_NAME}  ARCHIVE DESTINATION lib)

