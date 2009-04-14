# Find, include and link OpenJpeg
# Find is already called in the top-level CMakeLists

macro (FIND_OPENJPEG)
endmacro (FIND_OPENJPEG)

macro (INCLUDE_OPENJPEG)
	if (MSVC)
		include_directories (${REX_DEP_PATH}/OpenJpeg/include)
		link_directories (${REX_DEP_PATH}/OpenJpeg/lib)
	endif (MSVC)
endmacro (INCLUDE_OPENJPEG)

macro (LINK_OPENJPEG)
	if (MSVC)
		target_link_libraries (${TARGET_NAME} OpenJpeg)
	else (MSVC)
		target_link_libraries (${TARGET_NAME} openjpeg)
	endif (MSVC)
endmacro (LINK_OPENJPEG)