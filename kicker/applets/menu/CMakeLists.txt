


########### next target ###############

set(menu_panelapplet_PART_SRCS menuapplet.cpp )



kde4_add_plugin(menu_panelapplet ${menu_panelapplet_PART_SRCS})


target_link_libraries(menu_panelapplet  ${KDE4_KDEUI_LIBS} kickermain X11 )

install(TARGETS menu_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES menuapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )

