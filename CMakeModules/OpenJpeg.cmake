# Find, include and link OpenJpeg
# Find is already called in the top-level CMakeLists

macro (FIND_OPENJPEG)

include(FindPkgMacros)
findpkg_begin(OPENJPG)


# Construct search paths from enviromental hits and OS spesific guesses

if (MSVC)
  set(OPENJPG_PREFIX_GUESSES $ENV{REX_DEP_PATH}/OpenJpeg
    C:/OpenJpeg
    $ENV{PROGRAMFILES}/OpenJpeg C:/caelum $ENV{REX_DEP_PATH}/openjpeg)
elseif (UNIX)
  set(OPENJPEG_PREFIX_GUESSES 
    /opt/openjpeg
    /opt/OpenJpeg
    /usr/local
    /usr/lib/openjpeg
    /usr/lib/OpenJpeg
    /usr/local/openjpeg
    /usr/local/OpenJpeg
    $ENV{HOME}/OpenJpeg
    $ENV{HOME}/openjpeg
    $ENV{REX_DEP_PATH}/openjpeg
    $ENV{REX_DEP_PATH}/OpenJpeg)
endif()

set(OPENJPEG_PREFIX_PATH 
  ${OPENJPEG_HOME} $ENV{OPENJPEG_HOME} ${OPENJPEG_PREFIX_GUESSES})

create_search_paths(OPENJPEG)

# try to locate Caelum via pkg-config
use_pkgconfig(OPENJPEG "OPENJPEG")
# try to find framework on OSX
findpkg_framework(OPENJPEG)

if (OPENJPEG_FOUND)
  message(STATUS "OpenJpeg found through pkg-config")
  #Not needed
  set(OPENJPEG_LIBRARY_DIR "${OPENJPEG_LIBRARY_DIRS}")
  set(OPENJPEG_INCLUDE_DIR "${OPENJPEG_INCLUDE_DIRS}")
else ()
  message(STATUS "try to find OpenJpeg from guessed paths")
  find_path(OPENJPEG_INCLUDE_DIR openjpeg.h HINTS ${OPENJPEG_INC_SEARCH_PATH} ${OPENJPEG_PKGC_INCLUDE_DIRS})
  if (MSVC)
    find_path(OPENJPEG_LIBRARY_DIR OpenJPEG.lib HINTS ${OPENJPEG_LIB_SEARCH_PATH})
  else ()
    find_path(OPENJPEG_LIBRARY_DIR libopenjpeg.a HINTS ${OPENJPEG_LIB_SEARCH_PATH})
  endif()
endif()

if (NOT MSVC AND NOT OPENJPEG_FOUND AND NOT OPENJPEG_INCLUDE_DIR OR NOT OPENJPEG_LIBRARY_DIR)
  message(STATUS "OpenJpeg was not found either guessed paths or pkg-config, please add enviroment variable OPENJPEG_HOME")
endif()

endmacro (FIND_OPENJPEG)

macro (INCLUDE_OPENJPEG)
  if (MSVC)
    include_directories (${REX_DEP_PATH}/OpenJpeg/include)
    link_directories (${REX_DEP_PATH}/OpenJpeg/lib)
    include_directories (${REX_DEP_PATH}/OpenJpeg/libopenjpeg/)
    link_directories (${REX_DEP_PATH}/OpenJpeg/Debug)
    link_directories (${REX_DEP_PATH}/OpenJpeg/Release)
  elseif (NOT MSVC AND OPENJPEG_FOUND)
    include_directories(${OPENJPEG_INCLUDE_DIRS})
    link_directories(${OPENJPEG_LIBRARY_DIRS})
  else()
    include_directories(${OPENJPEG_INCLUDE_DIR})
    link_directories(${OPENJPEG_LIBRARY_DIR})
  endif(MSVC) 
  
endmacro (INCLUDE_OPENJPEG)

macro (LINK_OPENJPEG)
	if (MSVC)
		target_link_libraries (${TARGET_NAME}
			debug openjpegd
			optimized openjpeg)
        else (MSVC)
		target_link_libraries (${TARGET_NAME} openjpeg)
	endif (MSVC)
endmacro (LINK_OPENJPEG)
