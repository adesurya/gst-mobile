# CMAKE_C_FLAGS CMAKE_CXX_FLAGS
add_definitions(-O2 -Wall)
add_definitions(-DHAVE_CONFIG_H) 
add_definitions(-DGIO_COMPILATION)

include_directories(
    .
    ..
    ../glib
    ../gmodule
    ${PROJECT_BINARY_DIR}
)

link_directories(
)

set(gio_LIB_SRCS
    gaction.c                     gdbusinterface.c            gfileinfo.c
    glocalvfs.c                 gseekable.c
    gtlsclientconnection.c
    gactiongroup.c                gdbusinterfaceskeleton.c
    gfileinputstream.c         gmemoryinputstream.c        gsettingsbackend.c
    gtlsconnection.c
    gactiongroupexporter.c        gdbusintrospection.c        gfileiostream.c
    gmemoryoutputstream.c       gsettings.c                 gtlsdatabase.c
    gactionmap.c                  gdbusmenumodel.c            gfilemonitor.c
    gmemorysettingsbackend.c    gsettings-mapping.c         gtlsfiledatabase.c
    gappinfo.c                    gdbusmessage.c
    gfilenamecompleter.c       gmenu.c                     gsettingsschema.c
    gtlsinteraction.c
    gapplication.c                gdbusmethodinvocation.c
    gfileoutputstream.c        gmenuexporter.c             gsettings-tool.c
    gtlspassword.c
    gapplicationcommandline.c     gdbusnameowning.c
    gfilterinputstream.c       gmenumodel.c                gsimpleaction.c
    gtlsserverconnection.c
    gapplicationimpl-dbus.c       gdbusnamewatching.c
    gfilteroutputstream.c      gmount.c
    gsimpleactiongroup.c        gunionvolumemonitor.c
    gasynchelper.c                gdbusobject.c               gicon.c
    gmountoperation.c           gsimpleasyncresult.c        gunixconnection.c
    gasyncinitable.c              gdbusobjectmanager.c        ginetaddress.c
    gnativevolumemonitor.c      gsimplepermission.c
    gunixcredentialsmessage.c
    gasyncresult.c                gdbusobjectmanagerclient.c
    ginetaddressmask.c         gnetworkaddress.c           gsocketaddress.c
    gunixfdlist.c
    gbufferedinputstream.c        gdbusobjectmanagerserver.c
    ginetsocketaddress.c       gnetworkmonitorbase.c
    gsocketaddressenumerator.c  gunixfdmessage.c
    gbufferedoutputstream.c       gdbusobjectproxy.c          ginitable.c
    gnetworkmonitor.c           gsocket.c                   gunixinputstream.c
    gcancellable.c                gdbusobjectskeleton.c       ginputstream.c
    gnetworkmonitornetlink.c    gsocketclient.c             gunixmount.c
    gcharsetconverter.c           gdbusprivate.c              gioenumtypes.c
    gnetworkservice.c           gsocketconnectable.c        gunixmounts.c
    gcontenttype.c                gdbusproxy.c                gioerror.c
    gsocketconnection.c
    gunixoutputstream.c
    gconverter.c                  gdbusserver.c               giomodule.c
    gnullsettingsbackend.c      gsocketcontrolmessage.c
    gunixsocketaddress.c
    gconverterinputstream.c       gdbus-tool.c
    gio-querymodules.c         goutputstream.c
    gsocketinputstream.c        gunixvolume.c
    gconverteroutputstream.c      gdbusutils.c                gioscheduler.c
    gpermission.c               gsocketlistener.c
    gunixvolumemonitor.c
    gcredentials.c                gdelayedsettingsbackend.c   giostream.c
    gpollableinputstream.c      gsocketoutputstream.c       gvfs.c
    gdatainputstream.c            gdesktopappinfo.c
    gkeyfilesettingsbackend.c  gpollableoutputstream.c     gsocketservice.c
    gvolume.c
    gdataoutputstream.c           gdrive.c
    glib-compile-resources.c   gpollfilemonitor.c          gsocks4aproxy.c
    gvolumemonitor.c
    gdbusactiongroup.c            gdummyfile.c
    glib-compile-schemas.c     gproxyaddress.c             gsocks4proxy.c
    gdbusaddress.c                gdummyproxyresolver.c       gloadableicon.c
    gproxyaddressenumerator.c   gsocks5proxy.c
    gdbusauth.c                   gdummytlsbackend.c
    glocaldirectorymonitor.c   gproxy.c                    gsrvtarget.c
    gdbusauthmechanismanon.c      gemblem.c                   glocalfile.c
    gproxyresolver.c            gtcpconnection.c
    gdbusauthmechanism.c          gemblemedicon.c
    glocalfileenumerator.c     
    gtcpwrapperconnection.c     
    gdbusauthmechanismexternal.c  gfileattribute.c            glocalfileinfo.c
    gremoteactiongroup.c        gthemedicon.c               gzlibcompressor.c
    gdbusauthmechanismsha1.c      gfile.c
    glocalfileinputstream.c    gresolver.c                 gthreadedresolver.c
    gzlibdecompressor.c
    gdbusauthobserver.c           gfiledescriptorbased.c
    glocalfileiostream.c       gresource.c
    gthreadedsocketservice.c    strinfo.c
    gdbusconnection.c             gfileenumerator.c
    glocalfilemonitor.c        gresourcefile.c             gtlsbackend.c
    gdbuserror.c                  gfileicon.c
    glocalfileoutputstream.c   gresource-tool.c            gtlscertificate.c
)

##
set(so_NAME gio)
#add_library(${so_NAME} SHARED ${gio_LIB_SRCS})
#set_target_properties(${so_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 
#set_target_properties(${so_NAME} PROPERTIES VERSION 1.0 SOVERSION 1)

set(a_NAME gio_static)
add_library(${a_NAME} STATIC ${gio_LIB_SRCS})
set_target_properties(${a_NAME} PROPERTIES OUTPUT_NAME ${so_NAME})
set_target_properties(${a_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1) 

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/) 

#install(TARGETS ${so_NAME} LIBRARY DESTINATION lib)
install(TARGETS ${a_NAME}  ARCHIVE DESTINATION lib)

