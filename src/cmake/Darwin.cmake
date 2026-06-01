# set properties for the bundle Info.plist file
set(MACOSX_BUNDLE_EXECUTABLE_NAME "dlt-viewer")
set(MACOSX_BUNDLE_INFO_STRING "DLT Viewer")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${DLT_PROJECT_VERSION_MAJOR}.${DLT_PROJECT_VERSION_MINOR}.${DLT_PROJECT_VERSION_PATCH}")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${MACOSX_BUNDLE_SHORT_VERSION_STRING}")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "${MACOSX_BUNDLE_SHORT_VERSION_STRING}-${DLT_VERSION_SUFFIX}")
set(MACOSX_BUNDLE_BUNDLE_NAME "DLT Viewer")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.genivi.dlt-viewer")
set(MACOSX_BUNDLE_ICON_FILE icon)
set(MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2016, BMW AG - Alexander Wenzel &lt;alexander.aw.wenzel@bmw.de&gt;")

install(FILES
    resources/icon/icon.icns
    DESTINATION "${DLT_RESOURCE_INSTALLATION_PATH}"
    COMPONENT dlt_viewer)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../scripts/darwin/Info.plist.in" "${CMAKE_BINARY_DIR}/Info.plist" @ONLY)
install(PROGRAMS
  "${CMAKE_BINARY_DIR}/Info.plist"
  DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}/.."
  COMPONENT dlt_viewer)

# Install Qt offscreen platform plugin for silent/help mode on macOS.
# dlt-viewer sets QT_QPA_PLATFORM=offscreen for -s/--silent and -h/--help,
# so the plugin must be present in the standard Qt platform plugin bundle path.
get_target_property(DLT_QMAKE_LOCATION ${QT_PREFIX}::qmake LOCATION)
if(DLT_QMAKE_LOCATION)
    execute_process(
        COMMAND "${DLT_QMAKE_LOCATION}" -query QT_INSTALL_PLUGINS
        OUTPUT_VARIABLE DLT_QT_PLUGIN_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(NOT DLT_QT_PLUGIN_DIR)
    set(DLT_QT_PLUGIN_DIR "${DLT_QT_LIB_DIR}/../plugins")
endif()

set(DLT_QT_OFFSCREEN_PLUGIN
    "${DLT_QT_PLUGIN_DIR}/platforms/libqoffscreen.dylib")

if(EXISTS "${DLT_QT_OFFSCREEN_PLUGIN}")
    install(FILES
        "${DLT_QT_OFFSCREEN_PLUGIN}"
        DESTINATION "${DLT_APP_DIR_NAME}/Contents/PlugIns/platforms"
        COMPONENT qt_libraries)
else()
    message(FATAL_ERROR
        "Missing Qt offscreen platform plugin required for macOS silent/help mode: "
        "${DLT_QT_OFFSCREEN_PLUGIN}")
endif()

# COMMAND ${MAC_DEPLOY_TOOL} $<TARGET_FILE_DIR:dlt-viewer>/../.. -always-overwrite

# # enable high-DPI displays support
# add_custom_command(TARGET dlt-viewer POST_BUILD
#   COMMAND plutil -replace NSPrincipalClass -string NSApplication $<TARGET_FILE_DIR:dlt-viewer>/../Info.plist
#   COMMAND plutil -replace NSHighResolutionCapable -bool true $<TARGET_FILE_DIR:dlt-viewer>/../Info.plist)
