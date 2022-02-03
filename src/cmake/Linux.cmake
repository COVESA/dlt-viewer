# install(DIRECTORY
#     resources/icon/
#     COMPONENT dlt_viewer
#     DESTINATION "${DLT_RESOURCE_INSTALLATION_PATH}/icons"
#     PATTERN resources/icon/*.rc EXCLUDE)

# https://docs.appimage.org/reference/appdir.html#root-icon
install(FILES
    "resources/icon/256x256/org.genivi.DLTViewer.png"
    DESTINATION "${DLT_RESOURCE_INSTALLATION_PATH}/icons/hicolor/256x256/apps/"
    RENAME "dlt-viewer.png"
    COMPONENT dlt_viewer)
install(FILES
    "resources/icon/256x256/org.genivi.DLTViewer.png"
    DESTINATION "${DLT_APP_DIR_NAME}"
    RENAME ".DirIcon"
    COMPONENT dlt_viewer)
install(FILES
    "resources/icon/256x256/org.genivi.DLTViewer.png"
    DESTINATION "${DLT_APP_DIR_NAME}"
    RENAME "dlt-viewer.png"
    COMPONENT dlt_viewer)


install(PROGRAMS
    "resources/dlt-viewer.desktop"
    DESTINATION "${DLT_RESOURCE_INSTALLATION_PATH}/applications"
    COMPONENT dlt_viewer)
install(PROGRAMS
    "resources/dlt-viewer.desktop"
    DESTINATION "${DLT_APP_DIR_NAME}"
    COMPONENT dlt_viewer)
