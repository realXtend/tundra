# Find, include and link Poco
# Find is already called in the top-level CMakeLists

macro (FIND_POCO)

if (UNIX)

include(FindPkgMacros)
findpkg_begin(POCO)


# Construct search paths from enviromental hits and OS spesific guesses

set(POCO_PREFIX_GUESSES 
    /opt/PoCo
    /opt/poco
    /opt/Poco
    /usr/local
    /usr/lib
    /usr/lib/PoCo
    /usr/lib/Poco
    /usr/local/include/Poco
    /usr/local/Poco
    $ENV{HOME}/PoCo
    $ENV{HOME}/Poco
    $ENV{HOME}/poco
    $ENV{REX_DEP_PATH}
    $ENV{REX_DEP_PATH}/poco
    $ENV{REX_DEP_PATH}/Poco
    $ENV{REX_DEP_PATH}/include/Poco
    $ENV{REX_DEP_PATH}/PoCo)

set(POCO_PREFIX_PATH 
    ${POCO_HOME} $ENV{POCO_HOME} ${POCO_PREFIX_GUESSES})

create_search_paths(POCO)

# try to locate Poco via pkg-config
use_pkgconfig(POCO "POCO")
# try to find framework on OSX
findpkg_framework(POCO)

if (POCO_FOUND)
  message(STATUS "Poco found through pkg-config")
  # Not needed
  set(POCO_LIBRARY_DIR "${POCO_LIBRARY_DIRS}")
  set(POCO_INCLUDE_DIR "${POCO_INCLUDE_DIRS}")
else ()
  message(STATUS "try to find PoCo from guessed paths")
  #message(STATUS "try to find PoCo from guessed ${POCO_INC_SEARCH_PATH} ${POCO_PKGC_INCLUDE_DIRS}")
  find_path(POCO_INCLUDE_DIR Poco.h HINTS ${POCO_INC_SEARCH_PATH} ${POCO_PKGC_INCLUDE_DIRS})
  if (MSVC)
    find_path(POCO_LIBRARY_DIR PocoFoundation.lib HINTS ${POCO_LIB_SEARCH_PATH})
  else ()
    find_path(POCO_LIBRARY_DIR libPocoFoundation.so HINTS ${POCO_LIB_SEARCH_PATH})
  endif()
endif()

if (NOT POCO_FOUND AND NOT POCO_INCLUDE_DIR OR NOT POCO_LIBRARY_DIR)
  message(STATUS "Poco was not found either guessed paths or pkg-config, please add enviroment variable POCO_HOME")
endif()

endif()
endmacro (FIND_POCO)

macro (INCLUDE_POCO)
	
        if (MSVC)
	  include_directories (${REX_DEP_PATH}/PoCo/include)
	  link_directories (${REX_DEP_PATH}/PoCo/lib)
        else(NOT MSVC AND POCO_FOUND)
	  include_directories(${POCO_INCLUDE_DIRS})
	  link_directories(${POCO_LIBRARY_DIRS})
	else()
	  include_directories(${POCO_INCLUDE_DIR})
	  link_directories(${POCO_LIBRARY_DIR})
	endif (MSVC)
	
endmacro (INCLUDE_POCO)

macro (LINK_POCO)
  
  if (NOT MSVC)
    target_link_libraries (${TARGET_NAME} PocoFoundation
      PocoNet
      PocoUtil
      PocoXML)
  endif()
	  

endmacro (LINK_POCO)
