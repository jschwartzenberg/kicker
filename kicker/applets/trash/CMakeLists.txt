include_directories(${CMAKE_SOURCE_DIR}/kicker/core   )


########### next target ###############

set(trash_panelapplet_PART_SRCS trashbutton.cpp trashapplet.cpp )


kde4_add_plugin(trash_panelapplet ${trash_panelapplet_PART_SRCS})


target_link_libraries(trash_panelapplet  kickermain konq  ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KFILE_LIBS} ${KDE4_KUTILS_LIBS} )

install(TARGETS trash_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES trashapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )




