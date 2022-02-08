if(NOT LINUX)
    return()
endif()

set(CPACK_PACKAGE_VERSION "${DLT_PROJECT_VERSION_MAJOR}.${DLT_PROJECT_VERSION_MINOR}.${DLT_PROJECT_VERSION_PATCH}-${DLT_VERSION_SUFFIX}")

# Linuxdeploy wrapped in external CPack generator
# https://cmake.org/cmake/help/v3.15/cpack_gen/external.html
# See build.sh
set(CPACK_GENERATOR External)

# Linuxdeploy is mandatory, other packages are non relocatable
# See install.sh
find_program(LINUXDEPLOY_EXECUTABLE linuxdeploy HINTS "~/bin")
# find_program(LINUXDEPLOY_PLUGIN_QT_EXECUTABLE linuxdeploy-plugin-qt HINTS "~/bin")
find_program(QMAKE_EXECUTABLE qmake HINTS "/opt/qt512/bin")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux/linuxdeploy.cmake.in" "${CMAKE_BINARY_DIR}/linuxdeploy.cmake" @ONLY)

set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_BINARY_DIR}/linuxdeploy.cmake")

# Must be last
include(CPack)
