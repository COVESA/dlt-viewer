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



cmake_minimum_required (VERSION 2.6)

PROJECT(dlt-speed-app)

FIND_PACKAGE(PkgConfig)
FIND_PACKAGE(automotive-dlt)

###############################################################################
# is using the 'pkg-config' tool
pkg_check_modules(DLT	automotive-dlt)  
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_BUILD_TYPE Rlease)

##############################################################################
# general stuff
#project(DLT Testsuite)

include_directories(
   src
  ${GLIB_INCLUDE_DIRS}
  ${DLT_INCLUDE_DIRS}
)

link_directories(
    ${LINK_DIRECTORIES}
    ${DLT_LIBRARY_DIRS}
)


file(GLOB SOURCES
   "*.cpp"
   "*.c"
)

add_executable(
   dlt-speed-app
    ${SOURCES}
)

TARGET_LINK_LIBRARIES(
   dlt-speed-app
   ${GLIB_LIBRARIES}
   ${DLT_LIBRARIES}
)


##############################################################################
# installing files

INSTALL(TARGETS dlt-speed-app
  RUNTIME DESTINATION usr/local/bin
)


#INSTALL(FILES scripts/ergo-commander
#	DESTINATION etc/rc.d/init.d
#		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
#			GROUP_EXECUTE GROUP_READ
#			WORLD_EXECUTE WORLD_READ
#)

##############################################################################
# rpm configuration
set(CPACK_SET_DESTDIR "on")
set(CPACK_INSTALL_PREFIX "/")
#set(CPACK_RPM_PACKAGE_DEBUG "on")

set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/package-description")
set(CPACK_PACKAGE_VENDOR "BMW Car IT GmbH")
set(CPACK_PACKAGE_VERSION 1.2.0)
set(CPACK_SYSTEM_NAME "i586")

set(CPACK_RPM_PACKAGE_NAME dlt-speed-app)
set(CPACK_RPM_PACKAGE_RELEASE 1)
set(CPACK_RPM_PACKAGE_SUMMARY "Adapter connecting a BMW ErgoCommander with a CAN-Interface to a virtual MOST")
set(CPACK_RPM_PACKAGE_PACKAGER "Timo Mueller <timo.mueller@bmw-carit.de>")
set(CPACK_RPM_PACKAGE_ARCHITECTURE i586)

set(CPACK_RPM_PACKAGE_GROUP "Applications/BMW")
set(CPACK_RPM_PACKAGE_BUILD_REQUIRES "glibmm-devel, cmake, automotive-dlt-devel")
set(CPACK_RPM_PACKAGE_REQUIRES
	"glibmm, automotive-dlt")

set(CPACK_RPM_PACKAGE_CHANGELOG_FILE "${CMAKE_SOURCE_DIR}/changelog")

set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}.${CPACK_SYSTEM_NAME}")
set(CPACK_GENERATOR "RPM")
#include(CPack)


