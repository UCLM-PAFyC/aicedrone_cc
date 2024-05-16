option( PLUGIN_QAICEDRONETOOLS "Check to install qAicedroneTools plugin" OFF )

if( PLUGIN_QAICEDRONETOOLS )
	project( QAICEDRONETOOLS_PLUGIN )
	
	AddPlugin( NAME ${PROJECT_NAME} )

	if( NOT AICEDRONE_SRC_BASE_PATH )
		message( FATAL_ERROR "aicedrone source base path dir is not specified (AICEDRONE_SRC_BASE_PATH)" )
	endif()
	if( NOT AICEDRONE_LIBS_BASE_PATH )
		message( FATAL_ERROR "aicedrone libs base path dir is not specified (AICEDRONE_LIBS_BASE_PATH)" )
	endif()
	set( AICEDRONE_INCLUDE_DIR ${AICEDRONE_SRC_BASE_PATH} )
	target_include_directories( ${PROJECT_NAME} PRIVATE ${AICEDRONE_INCLUDE_DIR} ) 
	set(AICEDRONE_LIBRARIES 
		      ${AICEDRONE_LIBS_BASE_PATH}/libCRS.lib
			  ${AICEDRONE_LIBS_BASE_PATH}/libICGAL.lib
			  ${AICEDRONE_LIBS_BASE_PATH}/libIGDAL.lib
			  ${AICEDRONE_LIBS_BASE_PATH}/libLicenseManager.lib
		      ${AICEDRONE_LIBS_BASE_PATH}/libParameters.lib
		      ${AICEDRONE_LIBS_BASE_PATH}/libPointCloudFileManager.lib
              ${AICEDRONE_LIBS_BASE_PATH}/libProcessTools.lib
			  ${AICEDRONE_LIBS_BASE_PATH}/libWidgets.lib
			  ${AICEDRONE_LIBS_BASE_PATH}/libAicedroneModelDb.lib
			  ${AICEDRONE_LIBS_BASE_PATH}/libSystemInfo.lib)
			  # ${AICEDRONE_LIBS_BASE_PATH}/libPCRansacShapeDetection.lib)

	add_subdirectory( include )
	add_subdirectory( src )
	add_subdirectory( ui )

    find_package(Qt5 COMPONENTS Xml XmlPatterns REQUIRED)
	find_package(Boost COMPONENTS serialization iostreams REQUIRED)
    target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Xml Qt5::XmlPatterns ${Boost_LIBRARIES} ${AICEDRONE_LIBRARIES})
	
	if( WIN32 )
		if ( MSVC_VERSION GREATER_EQUAL 1900 ) # Visual Studio 2017
			target_compile_definitions( ${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN )
		endif()
		set(AICEDRONE_DLL_DIR ${AICEDRONE_LIBS_BASE_PATH})
		file( GLOB AICEDRONE_DLL_FILES 
		      ${AICEDRONE_LIBS_BASE_PATH}/libCRS.dll
			  ${AICEDRONE_LIBS_BASE_PATH}/libICGAL.dll
			  ${AICEDRONE_LIBS_BASE_PATH}/libIGDAL.dll
			  ${AICEDRONE_LIBS_BASE_PATH}/libLicenseManager.dll
		      ${AICEDRONE_LIBS_BASE_PATH}/libParameters.dll
		      ${AICEDRONE_LIBS_BASE_PATH}/libPointCloudFileManager.dll
              ${AICEDRONE_LIBS_BASE_PATH}/libProcessTools.dll
              ${AICEDRONE_LIBS_BASE_PATH}/libWidgets.dll
              ${AICEDRONE_LIBS_BASE_PATH}/libAicedroneModelDb.dll
              ${AICEDRONE_LIBS_BASE_PATH}/libSystemInfo.dll
              # ${AICEDRONE_LIBS_BASE_PATH}/libPCRansacShapeDetection.dll
			  ${AICEDRONE_LIBS_BASE_PATH}/quazip1-qt5d.dll
			  ${AICEDRONE_LIBS_BASE_PATH}/quazip1-qt5.dll)
		copy_files( "${AICEDRONE_DLL_FILES}" "${CLOUDCOMPARE_DEST_FOLDER}" 1 )
		if( ${OPTION_BUILD_CCVIEWER} )
			copy_files( "${AICEDRONE_DLL_FILES}" "${CCVIEWER_DEST_FOLDER}" 1 )
		endif()
		unset( AICEDRONE_DLL_DIR )
	endif()

endif()
