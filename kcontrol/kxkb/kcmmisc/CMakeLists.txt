#add_definitions (-DQT3_SUPPORT)

# TODO: add this piece to kdeinit

########### KCM ###############

set(kcm_keyboard_PART_SRCS 
  kcmmisc.cpp
  ${kxkb_COMMON_SRCS}
)


kde4_add_ui_files(kcm_keyboard_PART_SRCS kcmmiscwidget.ui )

kde4_add_plugin(kcm_keyboard ${kcm_keyboard_PART_SRCS})


target_link_libraries(kcm_keyboard
    ${KDE4_KIO_LIBS} xkbfile ${QT_QT3SUPPORT_LIBRARY}
    ${XKB_SUPPORT_LIB}
)
if(X11_XTest_FOUND)
  target_link_libraries(kcm_keyboard ${X11_XTest_LIB})
endif(X11_XTest_FOUND)

install(TARGETS kcm_keyboard  DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES keyboard.desktop DESTINATION  ${SERVICES_INSTALL_DIR} )
