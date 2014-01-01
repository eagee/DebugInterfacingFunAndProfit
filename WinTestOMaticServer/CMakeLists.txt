###################################################################################
# @file     WinTestOMaticServer/CMakeLists.txt
# @brief    Cmake build script for the WinTestOMaticServer project.
# @since    1.0.0
# @version  1.0.0
# @package  WinTestOMaticServer
###################################################################################

# Configure the settings for this project
project( WinTestOMaticServer )
set ( PROJECT_NAME, WinTestOMaticServer )

cmake_minimum_required( VERSION 2.8.0 )

set( 	WinTestOMaticServer_SourceFiles     	
		"${CMAKE_SOURCE_DIR}/src/dllmain.cpp" 
		"${CMAKE_SOURCE_DIR}/src/stdafx.cpp" 
		"${CMAKE_SOURCE_DIR}/src/WinTestOMaticServer.cpp" 
		"${CMAKE_SOURCE_DIR}/src/LocalSocketIpcServer.cpp" 
		"${CMAKE_SOURCE_DIR}/src/TestOMaticServer.cpp" 
	)
set( 	WinTestOMaticServer_HeaderFiles 		
		"${CMAKE_SOURCE_DIR}/include/WinTestOMaticServer/stdafx.h"
		"${CMAKE_SOURCE_DIR}/include/WinTestOMaticServer/targetver.h"
		"${CMAKE_SOURCE_DIR}/include/WinTestOMaticServer/LocalSocketIpcServer.h"
		"${CMAKE_SOURCE_DIR}/include/WinTestOMaticServer/TestOMaticServer.h"
	)

include( $ENV{CMAKE_SUPPORT_DIR}/Default.cmake )

# Attempt to locate and load settings for the Qt4 library. If the library is not found display a message suggesting a solution to the user.
# if the lib is found add support for MOC, UIC, and RCC (see: http://developer.qt.nokia.com/quarterly/view/using_cmake_to_build_qt_projects )
find_package( Qt4 REQUIRED QtMain QtCore QtGui QtNetwork )
if( NOT QT4_FOUND )
	Common_SuggestAddingPaths( Qt4 )
endif()
include( ${QT_USE_FILE} )
add_definitions( ${QT_DEFINITIONS} )
set( Qt4_LIBRARIES ${QT_LIBRARIES} )
set( Qt4_INCLUDE_DIRS ${QT_INCLUDES} )
set( Qt4_LIBRARY_DIRS ${QT_LIBRARY_DIR} )
set( CMAKE_AUTOMOC TRUE )

## Sets ${MBAM_SOURCE_DIR}
Common_CaptureSourceDirectory()

## Adds these the project path
include_directories( "include/" "src/" )

## Set the executable path
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/tests )

#add_subdirectory( "builds/static-md" )
## Stores the shared lib output
#get_directory_property( WinTestOMaticServer_STATIC_ALL_TARGETS DIRECTORY "builds/static-md" DEFINITION "BUILD_WITH_OPTIONS_TARGET_NAME" )

add_subdirectory( "builds/shared-md" )
# Stores the dynamic lib output
get_directory_property( WinTestOMaticServer_DYNAMIC_ALL_TARGETS DIRECTORY  "builds/shared-md" DEFINITION "BUILD_WITH_OPTIONS_TARGET_NAME" )
