###################################################################################
# @brief    Cmake build script generated to handle shared dll compilation and distribution/
# @details  To incoporate this file into a larger CMake project add it as a subdirectory 
#			(e.g. add_subdirectory( "builds\shared_md\" )) to a CmakeLists.txt in a parent directory.
# @since    1.0.0
# @version  1.0.0
# @package  WinTestOMatic
###################################################################################

# Needed for uic header outputs 
include_directories( "${CMAKE_CURRENT_BINARY_DIR}" )

build_with_options( ${CMAKE_PROJECT_NAME} 
	SHAREDLIB 
	DYNAMICRT 
	FILES ${WinTestOMaticServer_SourceFiles} ${WinTestOMaticServer_HeaderFiles}
	LIBS ${Qt4_LIBRARIES}
	VERSIONFILE "${MBAM_SOURCE_DIR}/version.h" )

set( WinTestOMaticServer_ALL_TARGETS ${BUILD_WITH_OPTIONS_ALL_TARGETS} )