# Find, include and link Ogre
# Find is already called in the top-level CMakeLists

macro (FIND_PYTHON)
endmacro (FIND_PYTHON)

macro (INCLUDE_PYTHON)
	if (MSVC)
		include_directories (${PROJECT_SOURCE_DIR}/external_libs/Python/include)
		
		link_directories (
			${PROJECT_SOURCE_DIR}/external_libs/Python
		)
	endif (MSVC)
endmacro (INCLUDE_PYTHON)

macro (LINK_PYTHON)
	if (MSVC)
		target_link_libraries (${TARGET_NAME}
			debug Python26_d
			optimized Python26
		)
	endif (MSVC)
endmacro (LINK_PYTHON)
