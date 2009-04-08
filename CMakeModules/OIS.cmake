# Find, include and link OIS
# Find is already called in the top-level CMakeLists

macro (FIND_OIS)
endmacro (FIND_OIS)

macro (INCLUDE_OIS)
	if (MSVC)
		include_directories (${REX_DEP_PATH}/ois/includes)
		link_directories (${REX_DEP_PATH}/ois/dll)
	endif (MSVC)
endmacro (INCLUDE_OIS)

macro (LINK_OIS)
	if (MSVC)
		target_link_libraries (${TARGET_NAME}
			debug OIS_d
			optimized OIS)
	endif (MSVC)
endmacro (LINK_OIS)