#######
# Dlt Speed App - Example Spped Application
# @licence make begin@
 #
 # Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 # 
 # This program is free software; you can redistribute it and/or modify it under the terms of the 
 # GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 # This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 # the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 # Public License, version 2.1, for more details.
 # 
 # You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 # with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 # 
 # Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 # also be applicable to programs even in cases in which the program is not a library in the technical sense.
 # 
 # Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 # license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 # license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 # may use the program under the following exception.
 # 
 # As a special exception, the copyright holders of DLT give you permission to combine DLT 
 # with software programs or libraries that are released under any license unless such a combination is not
 # permitted by the license of such a software program or library. You may copy and distribute such a 
 # system following the terms of the GNU Lesser General Public License, version 2.1, including this
 # special exception, for DLT and the licenses of the other code concerned.
 # 
 # Note that people who make modified versions of DLT are not obligated to grant this special exception 
 # for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 # version 2.1, gives permission to release a modified version without this exception; this exception 
 # also makes it possible to release a modified version which carries forward this exception.
 #
 # @licence end@
########


# Minimum cmake version required
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)


# Name for the entire project
PROJECT(dlt-speed-app)


# Set version parameters
SET( ${PROJECT_NAME}_MAJOR_VERSION 0 )
SET( ${PROJECT_NAME}_MINOR_VERSION 2 )
SET( ${PROJECT_NAME}_PATCH_LEVEL 0 )
SET( ${PROJECT_NAME}_VERSION_STATE "RELEASE" )
SET( PRINT_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_LEVEL})
SET( PRINT_VERSION_STATE ${${PROJECT_NAME}_VERSION_STATE})

# Print version information
MESSAGE("VERSION ${PRINT_VERSION}")
MESSAGE("VERSION_STATE ${PRINT_VERSION_STATE}")



# Use PkgConfig to check for automtoive-dlt
FIND_PACKAGE(PkgConfig)
pkg_check_modules(DLT REQUIRED automotive-dlt)
  

# Set CMAKE variables to given values
SET(CMAKE_CXX_FLAGS "-Wall")
SET(CMAKE_BUILD_TYPE Release)



# Add include directories to the build
INCLUDE_DIRECTORIES(
  src
  ${GLIB_INCLUDE_DIRS}
  ${DLT_INCLUDE_DIRS}
)


# Specify directories in which the linker will look for libraries
LINK_DIRECTORIES(
    ${LINK_DIRECTORIES}
    ${DLT_LIBRARY_DIRS}
)


# GLOB will generate a list of all files that match the globbing expressions and store it into "SOURCES"
FILE(GLOB SOURCES
   "*.c"
)

# Add an executable to the project using the specified source files
ADD_EXECUTABLE(
   dlt-speed-app
    ${SOURCES}
)


# Link a target to given libraries
TARGET_LINK_LIBRARIES(
   dlt-speed-app
   ${GLIB_LIBRARIES}
   ${DLT_LIBRARIES}
)


# Installing files
INSTALL(TARGETS dlt-speed-app
  RUNTIME DESTINATION bin
)
