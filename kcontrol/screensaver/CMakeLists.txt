
check_library_exists(GL glXChooseVisual "" HAVE_GLXCHOOSEVISUAL)
configure_file (config-screensaver.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-screensaver.h )

add_definitions (-DQT3_SUPPORT)

########### next target ###############

set(kcm_screensaver_PART_SRCS 
   scrnsave.cpp 
   testwin.cpp 
   saverlist.cpp 
   saverconfig.cpp 
   advanceddialog.cpp 
   kswidget.cpp )

set(screensaver_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/org.freedesktop.ScreenSaver.xml)
QT4_ADD_DBUS_INTERFACE( kcm_screensaver_PART_SRCS ${screensaver_xml} screensaver_interface )



kde4_add_ui_files(kcm_screensaver_PART_SRCS advanceddialogimpl.ui )

kde4_add_plugin(kcm_screensaver ${kcm_screensaver_PART_SRCS})


target_link_libraries(kcm_screensaver  ${KDE4_KDE3SUPPORT_LIBS})

if(HAVE_GLXCHOOSEVISUAL)
   target_link_libraries(kcm_screensaver ${OPENGL_gl_LIBRARY})
endif(HAVE_GLXCHOOSEVISUAL)


install(TARGETS kcm_screensaver  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES screensaver.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

