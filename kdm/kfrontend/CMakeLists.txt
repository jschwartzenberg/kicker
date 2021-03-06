include(MacroAddFileDependencies)

add_subdirectory( themer )
add_subdirectory( themes )
add_subdirectory( pics )
add_subdirectory( sessions )

include_directories(
	${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/background
	${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kdm
	${BLITZ_INCLUDES}
)

set(kdmthemer_SRCS
	themer/kdmthemer.cpp
	themer/kdmthemer.h
	themer/kdmitem.cpp
	themer/kdmitem.h
	themer/kdmpixmap.cpp
	themer/kdmpixmap.h
	themer/kdmrect.cpp
	themer/kdmrect.h
	themer/kdmlist.cpp
	themer/kdmlist.h
	themer/kdmlabel.cpp
	themer/kdmlabel.h
	themer/kdmlayout.cpp
	themer/kdmlayout.h
	themer/parse.cpp
	themer/parse.h
)
set(kdm_greet_SRCS
	kdm_greet.c
	kdm_greet.h
	kdmconfig.cpp
	kdmconfig.h
	kdmclock.cpp
	kdmclock.h
	kfdialog.cpp
	kfdialog.h
	kgdialog.cpp
	kgdialog.h
	kgverify.cpp
	kgverify.h
	kdmshutdown.cpp
	kdmshutdown.h
	kgreeter.cpp
	kgreeter.h
	kgapp.cpp
	kgapp.h
	utils.cpp
	utils.h
	${kdmthemer_SRCS}
)

if (XDMCP)
	set(kdm_greet_SRCS ${kdm_greet_SRCS}
		kchooser.cpp
		kchooser.h
	)
endif (XDMCP)

if (WITH_KDM_XCONSOLE)
	set(kdm_greet_SRCS ${kdm_greet_SRCS}
		kconsole.cpp
		kconsole.h
	)
endif (WITH_KDM_XCONSOLE)

macro_add_file_dependencies(kdm_greet.h ${confci})
kde4_add_executable(kdm_greet ${kdm_greet_SRCS})
target_link_libraries(kdm_greet ${KDE4_KDEUI_LIBS} ${POSIX4_LIBRARIES})
if (X11_XTest_FOUND)
  target_link_libraries(kdm_greet ${X11_XTest_LIB})
endif (X11_XTest_FOUND)
if (WITH_KDM_XCONSOLE)
  target_link_libraries(kdm_greet ${KDE4_KPTY_LIBS})
endif (WITH_KDM_XCONSOLE)

install(TARGETS kdm_greet DESTINATION ${BIN_INSTALL_DIR})

set(kdm_config_SRCS kdm_config.c)
macro_add_file_dependencies(kdm_config.c ${confci})
kde4_add_executable(kdm_config NOGUI ${kdm_config_SRCS})
macro_add_compile_flags(kdm_config -U_REENTRANT)
target_link_libraries(kdm_config ${SOCKET_LIBRARIES} ${RESOLV_LIBRARIES} ${POSIX4_LIBRARIES})
install(TARGETS kdm_config DESTINATION ${BIN_INSTALL_DIR})

set(krootimage_SRCS krootimage.cpp krootimage.h ${backgroundlib_SRCS})
kde4_add_executable(krootimage ${krootimage_SRCS})
# needs qt3support for q3ptrvector in background/bgrender.cpp
target_link_libraries(krootimage ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KIO_LIBS} ${QT_QTCORE_LIBRARY}
${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${BLITZ_LIBRARIES})
install(TARGETS krootimage DESTINATION ${BIN_INSTALL_DIR})

set(genkdmconf_SRCS genkdmconf.c)
macro_add_file_dependencies(genkdmconf.c ${confci})
kde4_add_executable(genkdmconf NOGUI RUN_UNINSTALLED ${genkdmconf_SRCS})
macro_add_compile_flags(genkdmconf -U_REENTRANT)
target_link_libraries(genkdmconf ${X11_LIBRARIES})
install(TARGETS genkdmconf DESTINATION ${BIN_INSTALL_DIR})

set(kdmctl_SRCS kdmctl.c)
kde4_add_executable(kdmctl ${kdmctl_SRCS})
macro_add_compile_flags(kdmctl -U_REENTRANT)
target_link_libraries(kdmctl ${SOCKET_LIBRARIES})
install(TARGETS kdmctl DESTINATION ${BIN_INSTALL_DIR})

build_confci()

## use 'GENKDMCONF_FLAGS=... make install' to override
install(CODE "
exec_program(${CMAKE_CURRENT_BINARY_DIR}/genkdmconf ARGS --in \$DESTDIR${CONFIG_INSTALL_DIR}/kdm --face-src ${CMAKE_CURRENT_SOURCE_DIR}/pics \$GENKDMCONF_FLAGS)
")
