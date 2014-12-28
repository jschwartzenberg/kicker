project(KICKER)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules )

#search packages used by KDE
find_package(KDE4 REQUIRED)
find_package(KDE4Workspace REQUIRED)
find_package(Strigi REQUIRED)
find_package(Blitz REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)
include (MacroOptionalFindPackage)
include(MacroOptionalAddSubdirectory)


#include(ConfigureChecks.cmake)
configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )
configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h )
configure_file (config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h )
configure_file (startkde.cmake ${CMAKE_CURRENT_BINARY_DIR}/startkde  @ONLY)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

include_directories(
    ${CMAKE_BINARY_DIR}/workspace/kicker/libkicker
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/libkicker
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/taskbar
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)
# libs
add_subdirectory(cmake)
add_subdirectory(libs)

# core apps
#add_subdirectory( ktip )
#add_subdirectory( klipper )
#add_subdirectory( kmenuedit )
#add_subdirectory( krunner )


# apps we need to figure out what to do with
add_subdirectory( kcontrol )

# data
#add_subdirectory( doc )
#add_subdirectory( menu )

add_subdirectory( kicker)

SET(krunner_DEPENDS "plasma")

########### install files ###############
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/startkde DESTINATION ${BIN_INSTALL_DIR})
