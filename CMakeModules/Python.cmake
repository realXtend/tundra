# Find, include and link Python
# Find is already called in the top-level CMakeLists

macro (FIND_PYTHON)
 
  if (UNIX)
    find_package(PythonLibs)
    if (NOT PYTHON_LIBRARIES OR NOT_PYTHON_INCLUDE_PATH)
      message (FATAL_ERROR "Python not found by pkg-config")
    endif()
  endif()

endmacro (FIND_PYTHON)

macro (INCLUDE_PYTHON)
	
  if (MSVC)
    include_directories (${ENV_NAALI_DEP_PATH}/Python/include)
    link_directories (${ENV_NAALI_DEP_PATH}/Python/lib)
  else()
    include_directories(${PYTHON_INCLUDE_PATH})
    link_directories(${PYTHON_LIBRARY_DIR})
  endif (MSVC)
  
endmacro (INCLUDE_PYTHON)

macro (LINK_PYTHON)
  if (MSVC)
    if(PYTHON_FORCE_RELEASE_VERSION)    
      target_link_libraries (${TARGET_NAME}
				debug Python26 
				optimized Python26)
    else(PYTHON_FORCE_RELEASE_VERSION)
      target_link_libraries (${TARGET_NAME}
				debug Python26_d 
				optimized Python26)
    endif(PYTHON_FORCE_RELEASE_VERSION)
  else()
    target_link_libraries (${TARGET_NAME} ${PYTHON_LIBRARIES})
  endif (MSVC)  

endmacro (LINK_PYTHON)
