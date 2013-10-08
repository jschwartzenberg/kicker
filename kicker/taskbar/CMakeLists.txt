

########### next target ###############

set(taskbar_LIB_SRCS taskcontainer.cpp taskbar.cpp tasklmbmenu.cpp taskrmbmenu.cpp )



kde4_add_kcfg_files(taskbar_LIB_SRCS taskbarsettings.kcfgc )

kde4_add_library(taskbar SHARED ${taskbar_LIB_SRCS})

target_link_libraries(taskbar ${KDE4_KUTILS_LIBS} ${KDE4WORKSPACE_KWORKSPACE_LIBS} ${KDE4WORKSPACE_TASKMANAGER_LIBS} ${BLITZ_LIBRARIES} kickermain )

set_target_properties(taskbar PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS taskbar  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES taskbar.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )

