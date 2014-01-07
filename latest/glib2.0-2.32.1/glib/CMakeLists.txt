# CMAKE_C_FLAGS CMAKE_CXX_FLAGS
add_definitions(-O2 -Wall)
add_definitions(-DHAVE_CONFIG_H) 
add_definitions(-DGLIB_COMPILATION)

include_directories(
    .
    ..
    ${PROJECT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}/glib
)

link_directories(
)

set(glib_LIB_SRCS
    docs.c           gcharset.c   gfileutils.c  gkeyfile.c      gmessages.c
    grand.c         gtestutils.c
    gunibreak.c      gvariant-parser.c
    garray.c         gchecksum.c  ggettext.c    glib-init.c     gnode.c
    gregex.c        gthread.c
    gunicollate.c    gvariant-serialiser.c
    gasyncqueue.c    gconvert.c   ghash.c       glib-private.c  goption.c
    gscanner.c      gthreadpool.c
    gunidecomp.c     gvarianttype.c
    gatomic.c        gdataset.c   ghmac.c       glib-unix.c     gpattern.c
    gsequence.c     gthread-posix.c  guniprop.c
    gvarianttypeinfo.c
    gbacktrace.c     gdate.c      ghook.c       glist.c         gpoll.c
    gshell.c        gstdio.c                       
    gurifuncs.c      gversion.c
    gbase64.c        gdatetime.c  ghostutils.c  gmain.c         gprimes.c
    gslice.c        gstrfuncs.c                    gtimer.c         gutf8.c
    gwakeup.c
    gbitlock.c       gdir.c       giochannel.c  gmappedfile.c   gprintf.c
    gslist.c        gstring.c                      gtimezone.c      gutils.c
    gbookmarkfile.c  genviron.c   giounix.c     gmarkup.c       gqsort.c
    gspawn.c        gstringchunk.c                 gtrashstack.c    gvariant.c
    gbytes.c         gerror.c     gmem.c          gqueue.c
    gtester.c                      gtree.c
    gvariant-core.c
)

##
set(so_NAME glib)
#add_library(${so_NAME} SHARED ${glib_LIB_SRCS})
#set_target_properties(${so_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 
#set_target_properties(${so_NAME} PROPERTIES VERSION 1.0 SOVERSION 1)

set(a_NAME glib_static)
add_library(${a_NAME} STATIC ${glib_LIB_SRCS})
set_target_properties(${a_NAME} PROPERTIES OUTPUT_NAME ${so_NAME})
set_target_properties(${a_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/) 

#install(TARGETS ${so_NAME} LIBRARY DESTINATION lib)
install(TARGETS ${a_NAME}  ARCHIVE DESTINATION lib)

