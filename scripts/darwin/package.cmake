if(NOT APPLE)
    return()
endif()

# See build.sh and src/cmake/Darwin.cmake
set(CPACK_GENERATOR External)

get_target_property(MOC_LOCATION Qt5::moc LOCATION)
get_filename_component(MACDEPLOYQT_EXECUTABLE ${MOC_LOCATION}/../macdeployqt ABSOLUTE)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/scripts/darwin/macdeployqt.cmake.in" "${CMAKE_BINARY_DIR}/macdeployqt.cmake" @ONLY)

set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_BINARY_DIR}/macdeployqt.cmake")

# Must be last
include(CPack)
