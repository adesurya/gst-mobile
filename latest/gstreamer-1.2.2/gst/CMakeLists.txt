# CMAKE_C_FLAGS CMAKE_CXX_FLAGS
add_definitions(-O2 -Wall)
add_definitions(-DHAVE_CONFIG_H) 
add_subdirectory(printf)

include_directories(
    .
    ${PROJECT_BINARY_DIR}
)

link_directories(
)

set(gst_LIB_SRCS
    gstallocator.c    gst.c                gstcontrolsource.c   gstformat.c
    gstminiobject.c   gstplugin.c         gstregistrybinary.c  gsttaglist.c
    gsttypefind.c
    gstatomicqueue.c  gstcaps.c            gstdatetime.c        gstghostpad.c
    gstobject.c       gstpluginfeature.c  gstregistry.c        gsttagsetter.c
    gsttypefindfactory.c
    gstbin.c          gstcapsfeatures.c    gstdebugutils.c      gstinfo.c
    gstpad.c          gstpluginloader.c   gstregistrychunks.c  gsttask.c
    gsturi.c
    gstbuffer.c       gstchildproxy.c      gstelement.c         gstiterator.c
    gstpadtemplate.c  gstpoll.c           gstsample.c          gsttaskpool.c
    gstutils.c
    gstbufferlist.c   gstclock.c           gstelementfactory.c  gstmemory.c
    gstparamspecs.c   gstpreset.c         gstsegment.c         gsttoc.c
    gstvalue.c
    gstbufferpool.c   gstcontext.c         gsterror.c           gstmessage.c
    gstparse.c        gstquark.c          gststructure.c       gsttocsetter.c
    gstbus.c          gstcontrolbinding.c  gstevent.c           gstmeta.c
    gstpipeline.c     gstquery.c          gstsystemclock.c     gsttrace.c
)


##
set(so_NAME gst)
#add_library(${so_NAME} SHARED ${gst_LIB_SRCS})
#set_target_properties(${so_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 
#set_target_properties(${so_NAME} PROPERTIES VERSION 1.0 SOVERSION 1)

set(a_NAME gst_static)
add_library(${a_NAME} STATIC ${gst_LIB_SRCS})
set_target_properties(${a_NAME} PROPERTIES OUTPUT_NAME ${so_NAME})
set_target_properties(${a_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/) 

#install(TARGETS ${so_NAME} LIBRARY DESTINATION lib)
install(TARGETS ${a_NAME}  ARCHIVE DESTINATION lib)

