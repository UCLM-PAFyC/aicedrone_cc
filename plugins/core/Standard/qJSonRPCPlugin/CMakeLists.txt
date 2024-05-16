# CloudCompare Json RPC plugin based on example for standard plugins

# Add an option to CMake to control whether we build this plugin or not
option( PLUGIN_STANDARD_QJSONRPC "Install Json RPC plugin" OFF )

if ( PLUGIN_STANDARD_QJSONRPC )
	project( QJSON_RPC_PLUGIN )
	 
	AddPlugin( NAME ${PROJECT_NAME} )

    find_package(Qt5 COMPONENTS Network WebSockets REQUIRED)
    target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Network Qt5::WebSockets)

	add_subdirectory( include )
	add_subdirectory( src )
 
    target_include_directories( ${PROJECT_NAME}
								PRIVATE
								${CMAKE_CURRENT_SOURCE_DIR}
    )

	if (WIN32)
		# copy Qt's Network abd WebSockets DLLs
		get_target_property( Qt5_NETWORK_LIB_LOCATION Qt5::Network LOCATION_RELEASE )
		copy_files(${Qt5_NETWORK_LIB_LOCATION} "${CLOUDCOMPARE_DEST_FOLDER}" 0) #mind the quotes!
		
		get_target_property( Qt5_WEBSOCKET_LIB_LOCATION Qt5::WebSockets LOCATION_RELEASE )
		copy_files(${Qt5_WEBSOCKET_LIB_LOCATION} "${CLOUDCOMPARE_DEST_FOLDER}" 0) #mind the quotes!

		if (CMAKE_CONFIGURATION_TYPES)
			#debug DLLs
			get_target_property( Qt5_NETWORK_LIB_DEBUG_LOCATION Qt5::Network LOCATION_DEBUG )
			copy_files(${Qt5_NETWORK_LIB_DEBUG_LOCATION} "${CLOUDCOMPARE_DEST_FOLDER}" 2) #mind the quotes!
			
			get_target_property( Qt5_WEBSOCKET_LIB_DEBUG_LOCATION Qt5::WebSockets LOCATION_DEBUG )
			copy_files(${Qt5_WEBSOCKET_LIB_DEBUG_LOCATION} "${CLOUDCOMPARE_DEST_FOLDER}" 2) #mind the quotes!
		endif()

	endif()

endif()
