# Find, include and link OIS
# Find is already called in the top-level CMakeLists

macro (FIND_OIS)
	if (NOT MSVC)
		find_package (PkgConfig)
		pkg_search_module (OIS OIS)
		if (NOT OIS_INCLUDE_DIRS AND NOT OIS_LIBRARIES)
			message (FATAL_ERROR "OIS not found by pkg-config")
		endif (NOT OIS_INCLUDE_DIRS AND NOT OIS_LIBRARIES)
		separate_arguments (OIS_INCLUDE_DIRS)
		separate_arguments (OIS_LIBRARIES)
	endif (NOT MSVC)
endmacro (FIND_OIS)

macro (INCLUDE_OIS)
	if (MSVC)
		include_directories (${REX_DEP_PATH}/ois/includes)
		link_directories (${REX_DEP_PATH}/ois/dll)
	else (MSVC)
		include_directories (${OIS_INCLUDE_DIRS})
		link_directories (${OIS_LIBDIR})
	endif (MSVC)
endmacro (INCLUDE_OIS)

macro (LINK_OIS)
	if (MSVC)
		target_link_libraries (${TARGET_NAME}
			debug OIS_d
			optimized OIS)
	else (MSVC)
		target_link_libraries (${TARGET_NAME} ${OIS_LIBRARIES})
	endif (MSVC)
endmacro (LINK_OIS)
