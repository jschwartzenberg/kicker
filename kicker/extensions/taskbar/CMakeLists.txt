include_directories( ${KDEBASE_WORKSPACE_SOURCE_DIR}/kicker/taskbar ${KDEBASE_WORKSPACE_SOURCE_DIR}/kicker/taskmanager/   )

########### next target ###############

set(taskbar_panelextension_PART_SRCS taskbarextension.cpp )



kde4_add_plugin(taskbar_panelextension ${taskbar_panelextension_PART_SRCS})


target_link_libraries(taskbar_panelextension  ${KDE4_KPARTS_LIBS} kickermain taskmanager taskbar )

install(TARGETS taskbar_panelextension  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES taskbarextension.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/extensions )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

