include_directories( ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/kxkb )

include_directories(
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/kicker/libkicker/ )

link_directories(lib)

#set(kcm_keyboard_test_SRCS kcm_keyboard_test.cpp )


#kde4_add_executable(kcm_keyboard_test ${kcm_keyboard_test_SRCS})

#target_link_libraries(kcm_keyboard_test ${KDE4_KDECORE_LIBS})

#SET(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${CMAKE_BINARY_DIR}/lib/kcm_keyboard.so )


########### next target ###############

# set(LIBXKLAVIER_DEFINITIONS -I/usr/include -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/libxml2)
# set(XKBSUPPORT_LIB -lxklavier -lgobject-2.0 -lglib-2.0 -lxml2 -lz -lm)
# 
# set(kxkb_applt_SRCS_COMMON main.cpp ../kxkbwidget.cpp ../pixmap.cpp ../kxkbconfig.cpp ../rules.cpp ../x11helper.cpp ../xklavier_adaptor.cpp)
# set(kxkb_aplt_KDEINIT_SRCS ${kxkb_applt_SRCS_COMMON})
# 
# 
# ADD_DEFINITIONS(-DHAVE_XKLAVIER=1 ${LIBXKLAVIER_DEFINITIONS} ${_LibGLIB2Cflags})
# 
# 
# kde4_add_kdeinit_executable(kxkb_aplt ${kxkb_aplt_KDEINIT_SRCS})
# 
# target_link_libraries(kdeinit_kxkb_aplt ${KDE4_KDE3SUPPORT_LIBS} ${XKBSUPPORT_LIB})
# install(TARGETS kdeinit_kxkb_aplt  DESTINATION ${LIB_INSTALL_DIR} )

#target_link_libraries( kxkb_aplt kdeinit_kxkb_aplt )
#install(TARGETS kxkb_aplt DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############

# set(kxkb_panelapplet_PART_SRCS kxkb_applet.cpp ${kxkb_applt_SRCS_COMMON})
# 
# 
# kde4_add_plugin(kxkb_panelapplet ${kxkb_panelapplet_PART_SRCS})
# 
# 
# target_link_libraries(kxkb_panelapplet ${KDE4_KDE3SUPPORT_LIBS} kworkspace kickermain  )
# 
# install(TARGETS kxkb_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )
# 
# ########### install files ###############
# 
# install(FILES ../kxkb.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets)


########### KXKB part test ###############

project(kxkb_part_test)

find_package(KDE4 REQUIRED)

include_directories( ${KDE4_INCLUDES} )

set(mySources kxkb_part_test.cpp kxkb_part_test_v.cpp)

#kde4_automoc( ${mySources} )

kde4_add_executable(kxkb_part_test ${mySources})

target_link_libraries(kxkb_part_test ${KDE4_KPARTS_LIBS} )

#install(TARGETS ksimple DESTINATION bin)
#install(FILES kfoo.desktop DESTINATION ${XDG_APPS_DIR})

