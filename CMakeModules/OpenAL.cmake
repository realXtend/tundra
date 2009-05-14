# Find, include and link OpenAL Find is already called in the top-level 
# CMakeLists

macro (FIND_OPENAL)
	if (NOT MSVC)
		find_package (PkgConfig)
		pkg_search_module (OPENAL OPENAL)
		if (NOT OPENAL_INCLUDE_DIRS AND NOT OPENAL_LIBRARIES)
			message (FATAL_ERROR "OpenAL not found by pkg-config")
		endif (NOT OPENAL_INCLUDE_DIRS AND NOT OPENAL_LIBRARIES)
		separate_arguments (OPENAL_INCLUDE_DIRS)
		separate_arguments (OPENAL_LIBRARIES)
	endif (NOT MSVC)
endmacro (FIND_OPENAL)

macro (INCLUDE_OPENAL)
	if (MSVC)
		include_directories (${REX_DEP_PATH}/OpenAL/include)
		link_directories (${REX_DEP_PATH}/OpenAL/libs/Win32)
		
		
	else (MSVC)
		include_directories (${OPENAL_INCLUDE_DIRS})
		link_directories (${OPENAL_LIBDIR})
	endif (MSVC)
endmacro (INCLUDE_OPENAL)

macro (LINK_OPENAL)
	if (MSVC)
		target_link_libraries (${TARGET_NAME}
			debug OpenAL32
			optimized OpenAL32)
	else (MSVC)
		target_link_libraries (${TARGET_NAME} ${OPENAL_LIBRARIES})
	endif (MSVC)
endmacro (LINK_OPENAL)
