


########### next target ###############

set(systemtray_panelapplet_PART_SRCS systemtrayapplet.cpp )



kde4_add_plugin(systemtray_panelapplet ${systemtray_panelapplet_PART_SRCS})


target_link_libraries(systemtray_panelapplet kickermain ${KDE4_KIO_LIBS} X11 )

install(TARGETS systemtray_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES systemtrayapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

