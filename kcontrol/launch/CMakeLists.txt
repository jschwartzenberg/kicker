


########### next target ###############

set(kcm_launch_PART_SRCS kcmlaunch.cpp )

set(kdesktop_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/org.kde.krunner.App.xml)
QT4_ADD_DBUS_INTERFACE( kcm_launch_PART_SRCS ${kdesktop_xml} krunner_interface )



kde4_add_plugin(kcm_launch ${kcm_launch_PART_SRCS})


target_link_libraries(kcm_launch  ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kcm_launch  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmlaunch.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

