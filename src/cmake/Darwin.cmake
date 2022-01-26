# set some properties for the bundle Info.plist file
set (MACOSX_BUNDLE_BUNDLE_NAME "DLT Viewer" )
set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${PACKAGE_VERSION} )
set (MACOSX_BUNDLE_GUI_IDENTIFIER "org.genivi.dlt-viewer")
set (MACOSX_BUNDLE_ICON_FILE icon)

# configure the bundle icon file
set (ICON resources/icon/icon.icns)
set_property (SOURCE ${ICON} PROPERTY MACOSX_PACKAGE_LOCATION Resources)
target_sources (dlt-viewer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${ICON})

# set the executable target to be built as macOS app bundle
set_target_properties (dlt-viewer PROPERTIES
  MACOSX_BUNDLE True
  OUTPUT_NAME "DLT Viewer")

# get the path for the macdeployqt tool
get_target_property(MOC_LOCATION Qt5::moc LOCATION)
get_filename_component(MAC_DEPLOY_TOOL ${MOC_LOCATION}/../macdeployqt ABSOLUTE)

# convert the target into a self contained app bundle
add_custom_command(TARGET dlt-viewer POST_BUILD
  COMMAND ${MAC_DEPLOY_TOOL} $<TARGET_FILE_DIR:dlt-viewer>/../.. -always-overwrite)

# enable high-DPI displays support
add_custom_command(TARGET dlt-viewer POST_BUILD
  COMMAND plutil -replace NSPrincipalClass -string NSApplication $<TARGET_FILE_DIR:dlt-viewer>/../Info.plist
  COMMAND plutil -replace NSHighResolutionCapable -bool true $<TARGET_FILE_DIR:dlt-viewer>/../Info.plist)
