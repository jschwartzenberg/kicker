
if (X11_Xft_FOUND)

  include_directories(
      ${FREETYPE_INCLUDE_DIR}
      ${FONTCONFIG_INCLUDE_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/lib)

  set(libkfontinstview_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/FontPreview.cpp 
                            ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/PreviewSelectAction.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/CharTip.cpp )
  set(libkfontinstjobrunner_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst/JobRunner.cpp
                                 ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst/ActionDialog.cpp )

  add_subdirectory( lib ) 
  add_subdirectory( kcmfontinst ) 
  add_subdirectory( strigi-analyzer )
  add_subdirectory( apps ) 
  add_subdirectory( kio ) 
  add_subdirectory( thumbnail )
  add_subdirectory( viewpart )

  kde4_install_icons( ${ICON_INSTALL_DIR} )

else (X11_Xft_FOUND)

  message(STATUS "Missing Xft - disabling font installer and font previews")

endif (X11_Xft_FOUND)
