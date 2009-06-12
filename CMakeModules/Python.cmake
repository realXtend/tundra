# Find, include and link Python
# Find is already called in the top-level CMakeLists

macro (FIND_PYTHON)
 
  if (UNIX)
    find_package (PkgConfig)
    pkg_search_module (python python)
    if (NOT python_INCLUDE_DIRS AND NOT python_LIBRARIES)
      message (FATAL_ERROR "Python not found by pkg-config")
  endif()

endmacro (FIND_PYTHON)

macro (INCLUDE_PYTHON)
	
  if (MSVC)
    include_directories (${REX_DEP_PATH}/Python/include)
    link_directories (${REX_DEP_PATH}/Python)
  else()
    include_directories(${python_INCLUDE_DIRS})
    link_directories(${python_LIBRARY_DIRS})
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
  else()
    target_link_libraries (${TARGET_NAME} ${python_LIBRARIES})
  endif (MSVC)
  

endmacro (LINK_PYTHON)
