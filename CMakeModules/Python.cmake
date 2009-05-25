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
    if(PYTHON_FORCE_RELEASE_VERSION)    
		target_link_libraries (${TARGET_NAME}
#			debug Python26 
#			optimized Python26)
			debug Python25 
			optimized Python25)
    else(PYTHON_FORCE_RELEASE_VERSION)
		target_link_libraries (${TARGET_NAME}
#			debug Python26_d 
#			optimized Python26)
			debug Python25_d 
			optimized Python25)
    endif(PYTHON_FORCE_RELEASE_VERSION)
	endif (MSVC)
endmacro (LINK_PYTHON)
