include_directories( ${CMAKE_SOURCE_DIR}/kicker/core   )


########### next target ###############

set(media_panelapplet_PART_SRCS preferencesdialog.cpp mediumbutton.cpp mediaapplet.cpp )


kde4_add_plugin(media_panelapplet ${media_panelapplet_PART_SRCS})


target_link_libraries(media_panelapplet  konq kickermain ${KDE4_KFILE_LIBS} ${KDE4_KUTILS_LIBS}
   ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS media_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES mediaapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )

