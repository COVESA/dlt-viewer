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
    dlt_common.h
    dlt_common.c
    dlt_common_cfg.h
    dlt_protocol.h
    dlt_types.h
    dlt_user.h
    qdltipconnection.h
    qdltipconnection.cpp
    qdlttcpconnection.h
    qdlttcpconnection.cpp
    qdltudpconnection.h
    qdltudpconnection.cpp
    qdltserialconnection.h
    qdltserialconnection.cpp
    qdltmsg.h
    qdltmsg.cpp
    qdltfilter.h
    qdltfilter.cpp
    qdltfile.h
    qdltfile.cpp
    qdltcontrol.h
    qdltcontrol.cpp
    qdltconnection.h
    qdltconnection.cpp
    qdltbase.h
    qdltbase.cpp
    qdltargument.h
    qdltargument.cpp
    qdltfilterlist.h
    qdltfilterlist.cpp
    qdltfilterindex.h
    qdltfilterindex.cpp
    qdltdefaultfilter.h
    qdltdefaultfilter.cpp
    qdltmessagedecoder.h
    qdltmessagedecoder.cpp
    qdltpluginmanager.h
    qdltpluginmanager.cpp
    qdltplugin.h
    qdltplugin.cpp
    qdltsegmentedmsg.h
    qdltsegmentedmsg.cpp
    qdltoptmanager.h
    qdltoptmanager.cpp
    qdltsettingsmanager.h
    qdltsettingsmanager.cpp
    qdltexporter.h
    qdltexporter.cpp
    qdltimporter.h
    qdltimporter.cpp
    fieldnames.h
    fieldnames.cpp
    dltmessagematcher.cpp
    dltmessagematcher.h
    qdltlrucache.hpp
    export_c_rules.h
    export_rules.h
    qdltctrlmsg.cpp
    qdltctrlmsg.h
    plugininterface.h
    qdlt.h
    qdltmsgwrapper.h
    qdltmsgwrapper.cpp
)

target_compile_definitions(qdlt PRIVATE
    BYTE_ORDER=LITTLE_ENDIAN
    PLUGIN_INSTALLATION_PATH="${PLUGIN_INSTALLATION_PATH}")

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
    if(NOT DLT_ADDITIONAL_FILES_INSTALLATION_PATH)
        # Default to current directory for non-Windows systems
        # This is where additional files will be installed
        # e.g., documentation, examples, etc.
        # It can be overridden by the user via CMake options
        set(DLT_ADDITIONAL_FILES_INSTALLATION_PATH ".")
    endif()
endif()

if(DLT_INSTALL_SDK)
    install(TARGETS qdlt
        DESTINATION "${DLT_ADDITIONAL_FILES_INSTALLATION_PATH}/sdk/lib"
        COMPONENT qdlt_sdk)
    install(DIRECTORY "."
        DESTINATION "${DLT_ADDITIONAL_FILES_INSTALLATION_PATH}/sdk/include/qdlt"
        COMPONENT qdlt_sdk
        FILES_MATCHING PATTERN "*.h"
        PATTERN "tests" EXCLUDE
        PATTERN "tests/*" EXCLUDE)
    set(SDK_EXAMPLES
        dummycommandplugin
        dummycontrolplugin
        dummydecoderplugin
        dummyviewerplugin
    )
    foreach(SDK_EXAMPLE IN ITEMS ${SDK_EXAMPLES})
        install(DIRECTORY "../plugin/${SDK_EXAMPLE}"
            DESTINATION "${DLT_ADDITIONAL_FILES_INSTALLATION_PATH}/sdk/src"
            COMPONENT qdlt_sdk)
    endforeach()
endif()

install(DIRECTORY "."
    DESTINATION "${DLT_ADDITIONAL_FILES_INSTALLATION_PATH}/include/qdlt"
    COMPONENT qdlt_sdk
    FILES_MATCHING PATTERN "*.h"
    PATTERN "tests" EXCLUDE
    PATTERN "tests/*" EXCLUDE)

find_package(GTest)
# configure unit tests only if gtest found on the system
if (GTest_FOUND)
    message(STATUS "Tests enabled")
    enable_testing()
    add_subdirectory(tests)
endif()
