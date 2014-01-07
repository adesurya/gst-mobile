# For version number
set(VERSION_MAJOR 1)
set(VERSION_MINOR 2)
set(VERSION_REV 2)
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV}")

cmake_minimum_required(VERSION 2.8)

project(gstreamer C CXX)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang)

# add extra header/function/type check
include(cmake_check.cmake)

include_directories(
    ${PROJECT_SOURCE_DIR}/../glib2.0-2.32.1
    ${PROJECT_SOURCE_DIR}/../glib2.0-2.32.1/glib
    ${PROJECT_SOURCE_DIR}/../glib2.0-2.32.1/gmodule
    ${PROJECT_SOURCE_DIR}/../glib2.0-2.32.1/cmkbld/glib
)

add_subdirectory(gst)
#add_subdirectory(libs)
#add_subdirectory(plugins)

# CMAKE_C_FLAGS CMAKE_CXX_FLAGS
add_definitions(-O2 -Wall)
add_definitions(-DHAVE_CONFIG_H) 

set(gstreamer_LIB_SRCS
    test.cpp
)
link_directories(
)
link_libraries(gstreamer pthread)


##
## for output
set(so_NAME gstreamer)
add_library(${so_NAME} SHARED ${gstreamer_LIB_SRCS})
set_target_properties(${so_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 
#set_target_properties(${so_NAME} PROPERTIES VERSION 1.0 SOVERSION 1)

set(a_NAME gstreamer_static)
add_library(${a_NAME} STATIC ${gstreamer_LIB_SRCS})
set_target_properties(${a_NAME} PROPERTIES OUTPUT_NAME ${so_NAME})
set_target_properties(${a_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/) 

##
## for install path
install(TARGETS ${so_NAME} LIBRARY DESTINATION lib)
install(TARGETS ${a_NAME}  ARCHIVE DESTINATION lib)

