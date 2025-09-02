if(NOT WIN32)
    return()
endif()

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${DLT_VERSION_SUFFIX}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "dlt-viewer")
set(CPACK_PACKAGE_NAME "DLTViewer")
set(CPACK_PACKAGE_VENDOR "COVESA")
set(CPACK_PACKAGE_EXECUTABLES "dlt-viewer" "DLT Viewer")

# 7Z
# Just Works.

# NSIS

# $LOCALAPPDATA is built-in NSIS variable https://nsis.sourceforge.io/Docs/Chapter4.html#varconstant
# Double escaping is needed
string(REGEX REPLACE "/" "\\\\\\\\" CPACK_NSIS_INSTALL_ROOT "$LOCALAPPDATA/Programs")

# No other way to set RequestExecutionLevel user
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/scripts/windows/nsis_cmake3.15.7)

set(CPACK_NSIS_MODIFY_PATH "ON")

# set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY ${CPACK_PACKAGE_INSTALL_DIRECTORY})
set(CPACK_NSIS_INSTALLED_ICON_NAME "dlt-viewer.exe")
# set(CPACK_NSIS_HELP_LINK ${APP_URL})
# set(CPACK_NSIS_URL_INFO_ABOUT ${APP_URL})
# set(CPACK_NSIS_CONTACT ${APP_EMAIL})

macro(addDocShortcut shortcutName fileName)
    LIST(APPEND CPACK_NSIS_CREATE_ICONS_EXTRA "  CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\${shortcutName}.lnk' '$INSTDIR\\\\doc\\\\${fileName}'")
    LIST(APPEND CPACK_NSIS_DELETE_ICONS_EXTRA "  Delete '$SMPROGRAMS\\\\$START_MENU\\\\${shortcutName}.lnk'")
endmacro()

addDocShortcut("DLT Viewer User Manual GENIVI" "dlt_viewer_user_manual.pdf")
addDocShortcut("DLT Viewer Plugins Programming Guide" "dlt_viewer_plugins_programming_guide.pdf")
addDocShortcut("DLT Viewer Plugins Overview B2B Confluence" "MGUROTO-DLTViewerPluginsOverview001.pdf")
addDocShortcut("DLT Viewer User Manual B2B Confluence" "MGUROTO-DLTViewerUserManual-120220-1029-4095.pdf")
addDocShortcut("HowTo DLTViewer SomeIP Decoding B2B Confluence" "MGUROTO-HowToDLTViewerSomeIPDecoding001.pdf")
string (REPLACE ";" "\n" CPACK_NSIS_CREATE_ICONS_EXTRA "${CPACK_NSIS_CREATE_ICONS_EXTRA}")
string (REPLACE ";" "\n" CPACK_NSIS_DELETE_ICONS_EXTRA "${CPACK_NSIS_DELETE_ICONS_EXTRA}")

# Avoid default bin subfolder
set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")

# Please be aware of Uninstall shortcut behavior https://stackoverflow.com/a/59876869

set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/icon/org.genivi.DLTViewer.ico")
string(REGEX REPLACE "/" "\\\\\\\\" CPACK_NSIS_MUI_ICON "${CPACK_NSIS_MUI_ICON}")

# Must be last
include(CPack)
