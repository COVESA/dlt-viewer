# SPDX-License-Identifier: MPL-2.0
#
# Copyright (C) 2016, Jack S. Smith
#
# This file is part of COVESA DLT-Viewer project.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.global/.
#
# List of changes:
# 01.Oct.2016, Jack Smith <jack.smith@elektrobit.com>, Original Author
#

add_library(qdlt SHARED
    dlt_common.c
    qdltipconnection.cpp
    qdlttcpconnection.cpp
    qdltudpconnection.cpp
    qdltserialconnection.cpp
    qdltmsg.cpp
    qdltfilter.cpp
    qdltfile.cpp
    qdltcontrol.cpp
    qdltconnection.cpp
    qdltbase.cpp
    qdltargument.cpp
    qdltfilterlist.cpp
    qdltfilterindex.cpp
    qdltdefaultfilter.cpp
    qdltmessagedecoder.cpp
    qdltpluginmanager.cpp
    qdltplugin.cpp
    qdltsegmentedmsg.cpp
    qdltoptmanager.cpp
    qdltsettingsmanager.cpp)

target_compile_definitions(qdlt PRIVATE
    BYTE_ORDER=LITTLE_ENDIAN
    PLUGIN_INSTALLATION_PATH="${DLT_PLUGIN_INSTALLATION_PATH}")

if(WIN32)
    target_compile_definitions(qdlt PRIVATE QDLT_LIBRARY QT_VIEWER)
else()
    target_compile_definitions(qdlt PRIVATE _TTY_POSIX_ QT_VIEWER)
endif()

target_include_directories(qdlt PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(qdlt PUBLIC
    ${QT_PREFIX}::Core
    ${QT_PREFIX}::Network
    ${QT_PREFIX}::Widgets
    ${QT_PREFIX}::SerialPort)

if(WIN32)
    target_link_libraries(qdlt PUBLIC ws2_32)
endif()

if(DLT_USE_QT_RPATH)
    set_target_properties(qdlt PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        INSTALL_RPATH "$ORIGIN;$<$<BOOL:${DLT_USE_QT_RPATH}>:${DLT_QT_LIB_DIR}>")
 endif()

install(TARGETS qdlt
    LIBRARY DESTINATION "${DLT_LIBRARY_INSTALLATION_PATH}"
    RUNTIME DESTINATION "${DLT_LIBRARY_INSTALLATION_PATH}"
    COMPONENT qdlt)

# Install QDLT SDK

if(WIN32)
    set(DLT_ADDITIONAL_FILES_INSTALLATION_PATH "${DLT_EXECUTABLE_INSTALLATION_PATH}")
else()
    set(DLT_ADDITIONAL_FILES_INSTALLATION_PATH ".")
endif()

if(DLT_INSTALL_SDK)
    install(TARGETS qdlt
        DESTINATION "${DLT_ADDITIONAL_FILES_INSTALLATION_PATH}/sdk/lib"
        COMPONENT qdlt_sdk)
    install(DIRECTORY "."
        DESTINATION "${DLT_ADDITIONAL_FILES_INSTALLATION_PATH}/sdk/include/qdlt"
        COMPONENT qdlt_sdk
        FILES_MATCHING PATTERN "*.h")

foreach(SDK_EXAMPLE IN ITEMS ${SDK_EXAMPLES})
    install(DIRECTORY "../plugin/${SDK_EXAMPLE}"
        DESTINATION "${DLT_ADDITIONAL_FILES_INSTALLATION_PATH}/sdk/src"
        COMPONENT qdlt_sdk)
endforeach()
endif()

install(DIRECTORY "."
    DESTINATION "${DLT_ADDITIONAL_FILES_INSTALLATION_PATH}/include/qdlt"
    COMPONENT qdlt_sdk
    FILES_MATCHING PATTERN "*.h")

set(SDK_EXAMPLES
    dummycommandplugin
    dummycontrolplugin
    dummydecoderplugin
    dummyviewerplugin)

foreach(SDK_EXAMPLE IN ITEMS ${SDK_EXAMPLES})
    install(DIRECTORY "../plugin/${SDK_EXAMPLE}"
        DESTINATION "${DLT_ADDITIONAL_FILES_INSTALLATION_PATH}/src"
        COMPONENT qdlt_sdk)
endforeach()
