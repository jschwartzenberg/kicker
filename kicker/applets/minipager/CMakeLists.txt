include_directories( ${KDEBASE_WORKSPACE_SOURCE_DIR}/kicker/taskmanager ${KDEBASE_WORKSPACE_SOURCE_DIR}/kicker/taskbar/  )


########### next target ###############

set(minipager_panelapplet_PART_SRCS pagerapplet.cpp pagerbutton.cpp)


kde4_add_kcfg_files(minipager_panelapplet_PART_SRCS pagersettings.kcfgc)

qt4_add_dbus_interface(minipager_panelapplet_PART_SRCS ../../../kdesktop/org.kde.kdesktop.Background.xml kdesktop_background_interface)

kde4_add_plugin(minipager_panelapplet ${minipager_panelapplet_PART_SRCS})


target_link_libraries(minipager_panelapplet ${KDE4_KDEUI_LIBS} taskmanager kickermain)

install(TARGETS minipager_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES minipagerapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )

