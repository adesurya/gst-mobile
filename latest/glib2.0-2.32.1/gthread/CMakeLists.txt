# CMAKE_C_FLAGS CMAKE_CXX_FLAGS
add_definitions(-O2 -Wall)
add_definitions(-DHAVE_CONFIG_H) 

include_directories(
    .
    ..
    ../glib
    ../gthread
    ${PROJECT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}/glib
    ${PROJECT_BINARY_DIR}/gthread
)

link_directories(
)


set(gthread_LIB_SRCS
    gthread-impl.c
)

##
set(so_NAME gthread)
#add_library(${so_NAME} SHARED ${gthread_LIB_SRCS})
#set_target_properties(${so_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 
#set_target_properties(${so_NAME} PROPERTIES VERSION 1.0 SOVERSION 1)

set(a_NAME gthread_static)
add_library(${a_NAME} STATIC ${gthread_LIB_SRCS})
set_target_properties(${a_NAME} PROPERTIES OUTPUT_NAME ${so_NAME})
set_target_properties(${a_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/) 

#install(TARGETS ${so_NAME} LIBRARY DESTINATION lib)
install(TARGETS ${a_NAME}  ARCHIVE DESTINATION lib)

