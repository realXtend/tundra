# Find, include and link curl
# Find is already called in the top-level CMakeLists

macro (FIND_CURL)
endmacro (FIND_CURL)

macro (INCLUDE_CURL)
	if (MSVC)
		include_directories (${PROJECT_SOURCE_DIR}/external_libs/libcurl/include)
		
		link_directories (
			${PROJECT_SOURCE_DIR}/external_libs/libcurl/lib/DLL-Debug
			${PROJECT_SOURCE_DIR}/external_libs/libcurl/lib/DLL-Release
		)		
	endif (MSVC)
endmacro (INCLUDE_CURL)

macro (LINK_CURL)
	if (MSVC)
		target_link_libraries (${TARGET_NAME}
			debug libcurld_imp
			optimized libcurl_imp
		)
	endif (MSVC)
endmacro (LINK_CURL)
