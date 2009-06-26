# Find, include and link OIS
# Find is already called in the top-level CMakeLists

macro (FIND_OIS)
  
  if (NOT MSVC)
    
    find_package (PkgConfig)
    pkg_search_module (OIS OIS)
    if (NOT OIS_INCLUDE_DIRS AND NOT OIS_LIBRARIES)
      
      message (STATUS "OIS was not found by pkg-config")
      
      set(OIS_PREFIX_GUESSES 
          /opt/OIS
          /opt/ois
          /usr/local
          /usr/lib/ois
          /usr/lib/OIS
          /usr/local/ois
          /usr/local/OIS
          /usr/local/include/OIS
          $ENV{HOME}/ois
          $ENV{HOME}/OIS
          $ENV{REX_DEP_PATH}
          $ENV{REX_DEP_PATH}/ois
          $ENV{REX_DEP_PATH}/OIS)
      
      set(OIS_PREFIX_PATH 
	${OIS_HOME} $ENV{OIS_HOME} ${OIS_PREFIX_GUESSES})
      
      create_search_paths(OIS)
      message(STATUS "try to find ois from guessed paths")
      
      find_path(OIS_INCLUDE_DIR OIS.h HINTS ${OIS_INC_SEARCH_PATH} ${OIS_PKGC_INCLUDE_DIRS})
      find_path(OIS_LIBRARY_DIR libOIS.so HINTS ${OIS_LIB_SEARCH_PATH})
      
      set(OIS_INCLUDE_DIRS ${OIS_INCLUDE_DIR})
      set(OIS_LIBDIR ${OIS_LIBRARY_DIR})
      set(OIS_LIBRARIES "OIS")

      if (NOT OIS_INCLUDE_DIR OR NOT OIS_LIBRARY_DIR)
	message(STATUS "Ois was not found either from guessed paths or pkg-config. Please set OIS_HOME enviroment variable")
      endif()
    
    endif (NOT OIS_INCLUDE_DIRS AND NOT OIS_LIBRARIES)
    
    #separate_arguments (OIS_INCLUDE_DIRS)
    #separate_arguments (OIS_LIBRARIES)
    
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
