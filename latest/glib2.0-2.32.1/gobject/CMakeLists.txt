# CMAKE_C_FLAGS CMAKE_CXX_FLAGS
add_definitions(-O2 -Wall)
add_definitions(-DHAVE_CONFIG_H) 
add_definitions(-DGOBJECT_COMPILATION)

include_directories(
    .
    ..
    ../glib
    ../gobject
    ${PROJECT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}/glib
    ${PROJECT_BINARY_DIR}/gobject
)

link_directories(
)


set(gobject_LIB_SRCS
    gatomicarray.c  gboxed.c    genums.c           gmarshal.c
    gobjectnotifyqueue.c  gparam.c       gsignal.c         gtype.c
    gtypeplugin.c  gvalue.c           gvaluetypes.c
    gbinding.c      gclosure.c  glib-genmarshal.c  gobject.c   gobject-query.c
    gparamspecs.c  gsourceclosure.c  gtypemodule.c  gvaluearray.c
    gvaluetransform.c  testgobject.c
)

##
set(so_NAME gobject)
#add_library(${so_NAME} SHARED ${gobject_LIB_SRCS})
#set_target_properties(${so_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 
#set_target_properties(${so_NAME} PROPERTIES VERSION 1.0 SOVERSION 1)

set(a_NAME gobject_static)
add_library(${a_NAME} STATIC ${gobject_LIB_SRCS})
set_target_properties(${a_NAME} PROPERTIES OUTPUT_NAME ${so_NAME})
set_target_properties(${a_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/) 

#install(TARGETS ${so_NAME} LIBRARY DESTINATION lib)
install(TARGETS ${a_NAME}  ARCHIVE DESTINATION lib)

