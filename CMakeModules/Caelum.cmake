# Find, include and link Caelum
# Find is already called in the top-level CMakeLists

macro (FIND_CAELUM)

if (UNIX)
include(FindPkgMacros)
findpkg_begin(CAELUM)

# Construct search paths from enviromental hits and OS spesific guesses

# TODO: For MSVC, remove all this guessing, it's not how things are done in Windows. The search procedure is the following:
# 1. If $ENV{CAELUM_HOME} is defined, use that directory.
# 2. If not, use $ENV{REX_DEP_PATH}/caelum.
# There should be no guessing of what the outputted library name might be. Caelum might not even be built or even unzipped 
# by the time this script is run. -jj.
# TODO: Do the same for all other MSVC dependencies.

if (MSVC)
  set(CAELUM_PREFIX_GUESSES $ENV{REX_DEP_PATH}/Caelum
      C:/Caelum
      $ENV{PROGRAMFILES}/Caelum C:/caelum $ENV{REX_DEP_PATH}/caelum)
elseif (UNIX)
  set(CAELUM_PREFIX_GUESSES 
      /opt/Caelum
      /opt/caelum
      /usr/local
      /usr/lib/Caelum
      /usr/lib/caelum
      /usr/local/Caelum
      /usr/local/caelum
      /usr/local/include/Caelum
      $ENV{HOME}/Caelum
      $ENV{HOME}/caelum
      $ENV{REX_DEP_PATH}
      $ENV{REX_DEP_PATH}/caelum
      $ENV{REX_DEP_PATH}/Caelum)
endif()

set(CAELUM_PREFIX_PATH 
    ${CAELUM_HOME} $ENV{CAELUM_HOME} ${CAELUM_PREFIX_GUESSES})

create_search_paths(CAELUM)

# try to locate Caelum via pkg-config
use_pkgconfig(CAELUM "CAELUM")

# try to find framework on OSX
findpkg_framework(CAELUM)

if (CAELUM_FOUND)
  message(STATUS "Caelum found through pkg-config")
  # Not needed
  set(CAELUM_LIBRARY_DIR "${CAELUM_LIBRARY_DIRS}")
  set(CAELUM_INCLUDE_DIR "${CAELUM_INCLUDE_DIRS}")
else ()
  message(STATUS "try to find Caelum from guessed paths")
  find_path(CAELUM_INCLUDE_DIR Caelum.h HINTS ${CAELUM_INC_SEARCH_PATH} ${CAELUM_PKGC_INCLUDE_DIRS})
  if (MSVC)
    find_path(CAELUM_LIBRARY_DIR caelum.lib HINTS ${CAELUM_LIB_SEARCH_PATH})
  else ()
    find_path(CAELUM_LIBRARY_DIR libCaelum.a HINTS ${CAELUM_LIB_SEARCH_PATH})
  endif()
endif()

if (NOT MSVC AND NOT CAELUM_FOUND AND NOT CAELUM_INCLUDE_DIR OR NOT CAELUM_LIBRARY_DIR)
  message(STATUS "Caelum was not found either guessed paths or pkg-config, please add enviroment variable CAELUM_HOME")
endif()

endif()
endmacro (FIND_CAELUM)

macro (INCLUDE_CAELUM)
	if (MSVC)
	  include_directories (${REX_DEP_PATH}/Caelum/include)
	  link_directories (${REX_DEP_PATH}/Caelum/lib)
        elseif( NOT MSVC AND CAELUM_FOUND)
	  include_directories(${CAELUM_INCLUDE_DIRS})
	  link_directories(${CAELUM_LIBRARY_DIRS})
	else ()
	  include_directories(${CAELUM_INCLUDE_DIR})
	  link_directories(${CAELUM_LIBRARY_DIR})
	endif (MSVC)

endmacro (INCLUDE_CAELUM)

macro (LINK_CAELUM)
	
        if (MSVC)
	  target_link_libraries (${TARGET_NAME}
	    optimized caelum
	    debug caelum_d)
	endif (MSVC)
	
	if (NOT MSVC) 
	  target_link_libraries(${TARGET_NAME} Caelum)
	endif()

endmacro (LINK_CAELUM)
