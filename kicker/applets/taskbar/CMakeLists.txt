include_directories( ${KDEBASE_WORKSPACE_SOURCE_DIR}/kicker/taskbar/ ${KDEBASE_WORKSPACE_SOURCE_DIR}/kicker/taskmanager  )


########### next target ###############

set(taskbar_panelapplet_PART_SRCS taskbarapplet.cpp )


kde4_add_plugin(taskbar_panelapplet ${taskbar_panelapplet_PART_SRCS})


target_link_libraries(taskbar_panelapplet  ${KDE4_KDEUI_LIBS} taskbar kickermain )

install(TARGETS taskbar_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES taskbarapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )

