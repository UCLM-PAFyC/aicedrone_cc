option( PLUGIN_QTIDOPTOOLS "Check to install qTidopTools plugin" OFF )

if( PLUGIN_QTIDOPTOOLS )
	project( QTIDOPTOOLS_PLUGIN )
	
	AddPlugin( NAME ${PROJECT_NAME} )

	if( NOT TIDOP_SRC_BASE_PATH )
		message( FATAL_ERROR "tidop source base path dir is not specified (TIDOP_SRC_BASE_PATH)" )
	endif()
	if( NOT TIDOP_LIBS_BASE_PATH )
		message( FATAL_ERROR "tidop libs base path dir is not specified (TIDOP_LIBS_BASE_PATH)" )
	endif()
	set( TIDOP_INCLUDE_DIR ${TIDOP_SRC_BASE_PATH} )
	target_include_directories( ${PROJECT_NAME} PRIVATE ${TIDOP_INCLUDE_DIR} ) 
	set(TIDOP_LIBRARIES ${TIDOP_LIBS_BASE_PATH}/libParameters.lib)

	add_subdirectory( include )
	add_subdirectory( src )
	add_subdirectory( ui )

    find_package(Qt5 COMPONENTS Xml XmlPatterns REQUIRED)
	find_package(Boost COMPONENTS serialization iostreams REQUIRED)
    target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Xml Qt5::XmlPatterns ${Boost_LIBRARIES} ${TIDOP_LIBRARIES})

	
	if( WIN32 )
		if ( MSVC_VERSION GREATER_EQUAL 1900 ) # Visual Studio 2017
			target_compile_definitions( ${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN )
		endif()
		set(TIDOP_DLL_DIR ${TIDOP_LIBS_BASE_PATH})
		file( GLOB TIDOP_DLL_FILES ${TIDOP_LIBS_BASE_PATH}/libParameters.dll )
		copy_files( "${TIDOP_DLL_FILES}" "${CLOUDCOMPARE_DEST_FOLDER}" 1 )
		if( ${OPTION_BUILD_CCVIEWER} )
			copy_files( "${TIDOP_DLL_FILES}" "${CCVIEWER_DEST_FOLDER}" 1 )
		endif()
		unset( TIDOP_DLL_DIR )
	endif()

endif()
