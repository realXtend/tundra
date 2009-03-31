# Find, include and link Python
# Find is already called in the top-level CMakeLists

macro (FIND_PYTHON)
endmacro (FIND_PYTHON)

macro (INCLUDE_PYTHON)
	if (MSVC)
		include_directories (${REX_DEP_PATH}/Python/include)
		link_directories (${REX_DEP_PATH}/Python)
	endif (MSVC)
endmacro (INCLUDE_PYTHON)

macro (LINK_PYTHON)
	if (MSVC)
		target_link_libraries (${TARGET_NAME}
			debug Python26_d
			optimized Python26)
	endif (MSVC)
endmacro (LINK_PYTHON)
