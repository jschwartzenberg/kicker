add_subdirectory( pics )
add_subdirectory( kcmmisc )
#add_subdirectory( tests ) 
KDE4_NO_ENABLE_FINAL(kxkb)
#add_definitions (-DQT3_SUPPORT)

OPTION(USE_XKLAVIER "Use libxklavier to get keyboard layouts configuration" ON)

MESSAGE(STATUS "option: USE_XKLAVIER  <${USE_XKLAVIER}>")

if( USE_XKLAVIER STREQUAL ON )
  macro_optional_find_package(LibXKlavier)
  macro_optional_find_package(GLIB2)
  macro_optional_find_package(GObject)

  if(LIBXKLAVIER_FOUND AND GLIB2_FOUND AND GOBJECT_FOUND)

#MESSAGE("xklav:: ${LIBXKLAVIER_DEFINITIONS}")
#MESSAGE("xklav:: ${LIBXKLAVIER_LIBRARIES}")
	SET(CMAKE_REQUIRED_DEFINITIONS ${LIBXKLAVIER_DEFINITIONS})
	SET(CMAKE_REQUIRED_LIBRARIES ${LIBXKLAVIER_LIBRARIES})

	SET(XKB_SUPPORT_SRC xklavier_adaptor.cpp)
	SET(XKB_SUPPORT_LIB ${LIBXKLAVIER_LIBRARIES} ${GOBJECT_LIBRARIES})
	ADD_DEFINITIONS(-DHAVE_XKLAVIER=1 ${LIBXKLAVIER_DEFINITIONS} ${_LibGLIB2Cflags})
	include_directories(${GLIB2_INCLUDE_DIR})
  else(LIBXKLAVIER_FOUND AND GLIB2_FOUND AND GOBJECT_FOUND)
	MESSAGE ("Could not find glib, gobject or libxklavier >= 2.91 - old xkbfile code will be used to get keyboard layout configuration!")
  endif(LIBXKLAVIER_FOUND AND GLIB2_FOUND AND GOBJECT_FOUND)
endif( USE_XKLAVIER STREQUAL ON )


########### common sources ############

set(kxkb_COMMON_SRCS 
  rules.cpp
  kxkbconfig.cpp
  extension.cpp
  x11helper.cpp
  pixmap.cpp
  layoutmap.cpp
  ${XKB_SUPPORT_SRC}
)


########### KCM ###############

set(kcm_keyboard_layout_PART_SRCS 
  kcmlayout.cpp
  ${kxkb_COMMON_SRCS}
)

kde4_add_ui_files(kcm_keyboard_layout_PART_SRCS kcmlayoutwidget.ui )

kde4_add_plugin(kcm_keyboard_layout ${kcm_keyboard_layout_PART_SRCS})

SET_TARGET_PROPERTIES(kcm_keyboard_layout PROPERTIES COMPILE_FLAGS "-DQT3_SUPPORT")

target_link_libraries(kcm_keyboard_layout
    ${KDE4_KIO_LIBS} xkbfile ${QT_QT3SUPPORT_LIBRARY}
    ${XKB_SUPPORT_LIB}
)
if(X11_XTest_FOUND)
  target_link_libraries(kcm_keyboard_layout ${X11_XTest_LIB})
endif(X11_XTest_FOUND)

install(TARGETS kcm_keyboard_layout DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES keyboard_layout.desktop DESTINATION  ${SERVICES_INSTALL_DIR} )


########### KXKB kdeinit ###############

set(kxkb_KDEINIT_SRCS ${kxkb_COMMON_SRCS}
  kxkbcore.cpp
  kxkbapp.cpp
  kxkbwidget.cpp
  kxkb_adaptor.cpp
  kxkb_part.cpp
)

kde4_add_kdeinit_executable( kxkb ${kxkb_KDEINIT_SRCS})

target_link_libraries(kdeinit_kxkb ${X11_X11_LIB} ${X11_Xext_LIB} ${KDE4_KDEUI_LIBS} xkbfile ${XKB_SUPPORT_LIB})

install(TARGETS kdeinit_kxkb  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kxkb kdeinit_kxkb )
install(TARGETS kxkb DESTINATION ${BIN_INSTALL_DIR})
#install(FILES kxkb_component.h DESTINATION ${INCLUDE_INSTALL_DIR})


########### KXKB Applet ###############

if(BUILD_KICKER)
set(kxkb_panelapplet_PART_SRCS kxkb_applet.cpp)

kde4_add_plugin(kxkb_panelapplet ${kxkb_panelapplet_PART_SRCS})

target_link_libraries(kxkb_panelapplet kworkspace kickermain kdeinit_kxkb
    ${KDE4_KIO_LIBS} xkbfile
    ${XKB_SUPPORT_LIB}
)
if(X11_XTest_FOUND)
  target_link_libraries(kxkb_panelapplet ${X11_XTest_LIB})
endif(X11_XTest_FOUND)

install(TARGETS kxkb_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES kxkb.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets)
endif(BUILD_KICKER)


########### KXKB KPart ###############
#
#set(kxkb_part_PART_SRCS kxkb_part.cpp)
#
#kde4_add_plugin(kxkb_part ${kxkb_part_PART_SRCS})
#
#target_link_libraries(kxkb_part kdeinit_kxkb
#    ${KDE4_KIO_LIBS} xkbfile
#    ${XKB_SUPPORT_LIB}
#)
#if(X11_XTest_FOUND)
#  target_link_libraries(kxkb_part ${X11_XTest_LIB})
#endif(X11_XTest_FOUND)
#
#install( TARGETS kxkb_part DESTINATION ${PLUGIN_INSTALL_DIR} )
#install( FILES kxkb_part.desktop DESTINATION  ${SERVICES_INSTALL_DIR} )
##install( FILES kxkb_part.rc DESTINATION  ${DATA_INSTALL_DIR}/kxkb_part )


########### install files ###############

install( FILES keyboard_layout.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
