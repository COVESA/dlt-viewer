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

cmake_minimum_required (VERSION 3.15)
# implicitly calls version checks
include(scripts/windows/version.cmake)
include(scripts/linux/version.cmake)

message(STATUS "COVESA DLT Viewer version: ${DLT_PROJECT_VERSION_MAJOR}.${DLT_PROJECT_VERSION_MINOR}.${DLT_PROJECT_VERSION_PATCH}")
if(NOT DLT_PROJECT_VERSION_MAJOR OR NOT DLT_PROJECT_VERSION_MINOR OR NOT DLT_PROJECT_VERSION_PATCH)
    set(DLT_PROJECT_VERSION_MAJOR 0)
    set(DLT_PROJECT_VERSION_MINOR 0)
    set(DLT_PROJECT_VERSION_PATCH 0)
endif()
message(STATUS "COVESA DLT Viewer version: ${DLT_PROJECT_VERSION_MAJOR}.${DLT_PROJECT_VERSION_MINOR}.${DLT_PROJECT_VERSION_PATCH}")

project(dlt-viewer
    VERSION ${DLT_PROJECT_VERSION_MAJOR}.${DLT_PROJECT_VERSION_MINOR}.${DLT_PROJECT_VERSION_PATCH}
    DESCRIPTION "DLT Viewer")
message(STATUS "DLT Viewer version: ${DLT_PROJECT_VERSION_MAJOR}.${DLT_PROJECT_VERSION_MINOR}.${DLT_PROJECT_VERSION_PATCH}-${DLT_VERSION_SUFFIX}")

file(WRITE "${CMAKE_BINARY_DIR}/version.txt" "${DLT_PROJECT_VERSION_MAJOR}.${DLT_PROJECT_VERSION_MINOR}.${DLT_PROJECT_VERSION_PATCH}")
file(WRITE "${CMAKE_BINARY_DIR}/full_version.txt" "${DLT_PROJECT_VERSION_MAJOR}.${DLT_PROJECT_VERSION_MINOR}.${DLT_PROJECT_VERSION_PATCH}-${DLT_VERSION_SUFFIX}")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(LINUX TRUE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)
set (CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

get_target_property(DLT_QT_LIBRARY_PATH ${QT_PREFIX}::Core LOCATION)
get_filename_component(DLT_QT_LIB_DIR ${DLT_QT_LIBRARY_PATH} DIRECTORY)

if(NOT WIN32)
    option(DLT_USE_QT_RPATH "Use RPATH for QT_LIBRARY_PATH to support non-standard QT install locations" ON)
    if (DLT_USE_QT_RPATH)
        # Add Qt to the RPATH, so that there is no need to set LD_LIBRARY_PATH at runtime if Qt is installed in a non-standard location
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${DLT_QT_LIB_DIR}")
    endif()
endif()

# Injection of the GCC-specific compilation flags
if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-variadic-macros -Wno-strict-aliasing -fPIC")
endif()

add_compile_definitions(QT5)
set(QT5_QT6_COMPAT_VERSION "5.14")
# add compiler definition to be used for >= Qt 5.14 code
if(${QT_PREFIX}Core_VERSION VERSION_GREATER_EQUAL ${QT5_QT6_COMPAT_VERSION})
    add_compile_definitions(QT5_QT6_COMPAT)
endif()

option(DLT_USE_STANDARD_INSTALLATION_LOCATION "Use standard GNU installation locations" OFF)
option(DLT_INSTALL_SDK "Install in SDK location" ON)
if(NOT WIN32)
    if(NOT DLT_APP_DIR_NAME)
        set(DLT_APP_DIR_NAME "DLTViewer")
        if(APPLE)
            set(DLT_APP_DIR_NAME "DLTViewer.app")
        endif()
    endif()
    if(DLT_USE_STANDARD_INSTALLATION_LOCATION)
        include(GNUInstallDirs)
        set(DLT_PLUGIN_INSTALLATION_PATH "${CMAKE_INSTALL_FULL_LIBDIR}/dlt-viewer/plugins")
        set(DLT_RESOURCE_INSTALLATION_PATH "${CMAKE_INSTALL_FULL_DATADIR}")
        set(DLT_EXECUTABLE_INSTALLATION_PATH "${CMAKE_INSTALL_FULL_BINDIR}")
        set(DLT_LIBRARY_INSTALLATION_PATH "${CMAKE_INSTALL_FULL_LIBDIR}")
    else()
        if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
            set(CMAKE_INSTALL_PREFIX "DLTViewer")
        endif()
        if(NOT DLT_PLUGIN_INSTALLATION_PATH)
            set(DLT_PLUGIN_INSTALLATION_PATH "${DLT_APP_DIR_NAME}/usr/bin/plugins")
        endif()
        if(NOT DLT_RESOURCE_INSTALLATION_PATH)
            set(DLT_RESOURCE_INSTALLATION_PATH "${DLT_APP_DIR_NAME}/usr/share")
        endif()
        if(NOT DLT_EXECUTABLE_INSTALLATION_PATH)
            set(DLT_EXECUTABLE_INSTALLATION_PATH "${DLT_APP_DIR_NAME}/usr/bin")
        endif()
        if(NOT DLT_LIBRARY_INSTALLATION_PATH)
            set(DLT_LIBRARY_INSTALLATION_PATH "${DLT_APP_DIR_NAME}/usr/lib")
        endif()
    endif()
else()
    if(NOT DLT_APP_DIR_NAME)
        set(DLT_APP_DIR_NAME "./")
    endif()
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        # AppData is a modern install location. No Admin rights required.
        file(TO_CMAKE_PATH "$ENV{LOCALAPPDATA}/Programs/dlt-viewer" CMAKE_INSTALL_PREFIX)
        set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "..." FORCE)
    endif()

    if(NOT DLT_PLUGIN_INSTALLATION_PATH)
        set(DLT_PLUGIN_INSTALLATION_PATH "plugins")
    endif()
    if(NOT DLT_RESOURCE_INSTALLATION_PATH)
        set(DLT_RESOURCE_INSTALLATION_PATH ".")
    endif()
    if(NOT DLT_EXECUTABLE_INSTALLATION_PATH)
        set(DLT_EXECUTABLE_INSTALLATION_PATH ".")
    endif()
    if(NOT DLT_LIBRARY_INSTALLATION_PATH)
        set(DLT_LIBRARY_INSTALLATION_PATH ".")
    endif()
endif()

option(DLT_PARSER "Build DLT Parser" OFF)

if(DLT_PARSER)
    add_subdirectory(parser)
endif()

add_subdirectory(qdlt)
add_subdirectory(src)
add_subdirectory(plugin)
add_subdirectory(commander)

message(STATUS "\n\t** DLT Viewer Build Summary **")
message(STATUS "\tCMAKE_INSTALL_PREFIX:         ${CMAKE_INSTALL_PREFIX}")
message(STATUS "\tCMAKE_BUILD_TYPE:             ${CMAKE_BUILD_TYPE}")
message(STATUS "\tCMAKE_SYSTEM_PROCESSOR:       ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "\tCMAKE_SYSTEM_NAME:            ${CMAKE_SYSTEM_NAME}")
message(STATUS "\tDLT_QT_LIB_DIR:               ${DLT_QT_LIB_DIR}\n")

# Must be last
include(scripts/windows/package.cmake)
include(scripts/linux/package.cmake)
include(scripts/darwin/package.cmake)
