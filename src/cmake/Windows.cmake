set(QT_LIBS
  ${QT_PREFIX}::Core
  ${QT_PREFIX}::Gui
  ${QT_PREFIX}::Network
  ${QT_PREFIX}::PrintSupport
  ${QT_PREFIX}::SerialPort
  ${QT_PREFIX}::Widgets)

foreach(QT_LIB IN ITEMS ${QT_LIBS})
  get_target_property(LIBRARY_PATH ${QT_LIB} LOCATION)
  install(FILES
      "${LIBRARY_PATH}"
      DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}"
      COMPONENT qt_libraries)
endforeach()

if("${QT_PREFIX}" STREQUAL "Qt5")
install(FILES
    "${DLT_QT_LIB_DIR}/../plugins/bearer/qgenericbearer.dll"
    DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}/bearer"
    COMPONENT qt_libraries)
endif()
install(FILES
    "${DLT_QT_LIB_DIR}/../plugins/iconengines/qsvgicon.dll"
    DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}/iconengines"
    COMPONENT qt_libraries)
install(FILES
    "${DLT_QT_LIB_DIR}/../plugins/platforms/qwindows.dll"
    DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}/platforms"
    COMPONENT qt_libraries)
install(FILES
    "${DLT_QT_LIB_DIR}/../plugins/imageformats/qico.dll"
    DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}/imageformats"
    COMPONENT qt_libraries)
if("${QT_PREFIX}" STREQUAL "Qt5")
install(FILES
    "${DLT_QT_LIB_DIR}/../plugins/printsupport/windowsprintersupport.dll"
    DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}/printsupport"
    COMPONENT qt_libraries)
endif()
install(FILES
    "${DLT_QT_LIB_DIR}/../plugins/styles/qwindowsvistastyle.dll"
    DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}/styles"
    COMPONENT qt_libraries)

option(INCLUDE_VC_REDIST "Add vc_redist.x64.exe cmake install command" OFF)
if(INCLUDE_VC_REDIST)
  get_filename_component(MSVC_COMPILER_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY )
  set(VC_REDIST_PATH "${MSVC_COMPILER_DIR}/../../../../../../Redist/MSVC/v${MSVC_TOOLSET_VERSION}/vc_redist.x64.exe")
  install(FILES
      "${VC_REDIST_PATH}"
      DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}"
      COMPONENT vc_redist_x64)
endif()