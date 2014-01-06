# For version number
set(VERSION_MAJOR 2)
set(VERSION_MINOR 32)
set(VERSION_REV 1)
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV}")

cmake_minimum_required(VERSION 2.8)

project(glib2 C CXX)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang)

# add extra header/function/type check
include(cmake_check.cmake)
add_subdirectory(glib)
#add_subdirectory(gio)
#add_subdirectory(gmodule)
#add_subdirectory(gobject)
#add_subdirectory(gthread)

