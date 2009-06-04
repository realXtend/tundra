# Find, include and link Caelum
# Find is already called in the top-level CMakeLists

macro (FIND_CAELUM)
endmacro (FIND_CAELUM)

macro (INCLUDE_CAELUM)
	if (MSVC)
		include_directories (${REX_DEP_PATH}/Caelum/include)
		link_directories (${REX_DEP_PATH}/Caelum/lib)
	endif (MSVC)
endmacro (INCLUDE_CAELUM)

macro (LINK_CAELUM)
	if (MSVC)
		target_link_libraries (${TARGET_NAME}
			optimized caelum
			debug caelum_d)
	endif (MSVC)
endmacro (LINK_CAELUM)
