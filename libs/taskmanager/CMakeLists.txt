


########### next target ###############

set(taskmanager_LIB_SRCS taskmanager.cpp) 

kde4_add_library(taskmanager SHARED ${taskmanager_LIB_SRCS})

target_link_libraries(taskmanager  ${KDE4_KDE3SUPPORT_LIBRARY} ${KDE4_KDEUI_LIBS} )
if (X11_Xfixes_FOUND)
  target_link_libraries(taskmanager ${X11_Xfixes_LIB})
endif (X11_Xfixes_FOUND)
if (X11_Xrender_FOUND)
  target_link_libraries(taskmanager ${X11_Xrender_LIB})
endif (X11_Xrender_FOUND)
if (X11_Xcomposite_FOUND)
  target_link_libraries(taskmanager ${X11_Xcomposite_LIB})
endif (X11_Xcomposite_FOUND)

set_target_properties(taskmanager PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS taskmanager  DESTINATION ${LIB_INSTALL_DIR} )

install( FILES taskmanager.h DESTINATION ${INCLUDE_INSTALL_DIR}/plasma )


