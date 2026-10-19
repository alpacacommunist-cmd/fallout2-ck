function(setup_fallout2_ce_alias)
	add_library(fallout2-ce-alias INTERFACE)

	target_include_directories(fallout2-ce-alias INTERFACE
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>"
	)

	if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/fallout2-ce-includes")
		file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/fallout2-ce-includes")
		file(CREATE_LINK
			"${CMAKE_CURRENT_SOURCE_DIR}/fallout2-ce/src"
			"${CMAKE_CURRENT_BINARY_DIR}/fallout2-ce-includes/fallout2-ce"
			SYMBOLIC
		)
	endif()

	target_include_directories(fallout2-ce-alias INTERFACE
		"${CMAKE_CURRENT_BINARY_DIR}/fallout2-ce-includes"
	)
endfunction()
