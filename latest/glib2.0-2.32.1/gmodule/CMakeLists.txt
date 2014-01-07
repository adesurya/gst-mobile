# CMAKE_C_FLAGS CMAKE_CXX_FLAGS
add_definitions(-O2 -Wall)
add_definitions(-DHAVE_CONFIG_H) 

include_directories(
    .
    ..
    ../glib
    ../gmodule
    ${PROJECT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}/glib
    ${PROJECT_BINARY_DIR}/gmodule
)

link_directories(
)


#    gmodule-ar.c  gmodule-beos.c  gmodule.c  gmodule-dl.c  gmodule-dld.c
#    gmodule-dyld.c  gmodule-os2.c  gmodule-win32.c
set(gmodule_LIB_SRCS
    gmodule.c
)

##
set(so_NAME gmodule)
#add_library(${so_NAME} SHARED ${gmodule_LIB_SRCS})
#set_target_properties(${so_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 
#set_target_properties(${so_NAME} PROPERTIES VERSION 1.0 SOVERSION 1)

set(a_NAME gmodule_static)
add_library(${a_NAME} STATIC ${gmodule_LIB_SRCS})
set_target_properties(${a_NAME} PROPERTIES OUTPUT_NAME ${so_NAME})
set_target_properties(${a_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/) 

#install(TARGETS ${so_NAME} LIBRARY DESTINATION lib)
install(TARGETS ${a_NAME}  ARCHIVE DESTINATION lib)

