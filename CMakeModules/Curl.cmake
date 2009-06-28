# Find, include and link curl
# Find is already called in the top-level CMakeLists

macro (FIND_CURL)

if (UNIX)

include(FindPkgMacros)
findpkg_begin(CURL)

# Construct search paths from enviromental hits and OS spesific guesses

set(CURL_PREFIX_GUESSES 
    /opt/curl
    /opt/libcurl
    /usr/local
    /usr/lib/curl
    /usr/lib/libcurl
    /usr/include/curl
    /usr/include/libcurl
    /usr/local/curl
    /usr/local/libcurl
    /usr/local/include/curl
    $ENV{HOME}/curl
    $ENV{HOME}/libcurl
    $ENV{HOME}/Curl
    $ENV{REX_DEP_PATH}
    $ENV{REX_DEP_PATH}/curl
    $ENV{REX_DEP_PATH}/libcurl)

set(CURL_PREFIX_PATH 
    ${CURL_HOME} $ENV{CURL_HOME} ${CURL_PREFIX_GUESSES})

create_search_paths(CURL)

# try to locate CURL via pkg-config
use_pkgconfig(CURL "CURL")
# try to find framework on OSX
findpkg_framework(CURL)

if (CURL_FOUND)
  message(STATUS "Curl found through pkg-config")
  # Not needed
  set(CURL_LIBRARY_DIR "${CURL_LIBRARY_DIRS}")
  set(CURL_INCLUDE_DIR "${CURL_INCLUDE_DIRS}")
else ()
  message(STATUS "try to find Curl from guessed paths")
  find_path(CURL_INCLUDE_DIR curl.h HINTS ${CURL_INC_SEARCH_PATH} ${CURL_PKGC_INCLUDE_DIRS})
  if (MSVC)
    find_path(CURL_LIBRARY_DIR libcurl.dll HINTS ${CURL_LIB_SEARCH_PATH})
  else ()
    find_path(CURL_LIBRARY_DIR libcurl.a HINTS ${CURL_LIB_SEARCH_PATH})
    find_path(CURL_LIBRARY_DIR libcurl.so HINTS ${CURL_LIB_SEARCH_PATH})
  endif()
endif()

if (NOT MSVC AND NOT CURL_FOUND AND NOT CURL_INCLUDE_DIR OR NOT CURL_LIBRARY_DIR)
  message(STATUS "Curl was not found either guessed paths or pkg-config, please add enviroment variable CURL_HOME")
endif()

endif()
endmacro (FIND_CURL)

macro (INCLUDE_CURL)
  
  if (MSVC)
    include_directories (${REX_DEP_PATH}/libcurl/include)
    link_directories (${REX_DEP_PATH}/libcurl/lib/DLL-Debug ${REX_DEP_PATH}/libcurl/lib/DLL-Release)		
  elseif(NOT MSVC AND CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    link_directories(${CURL_LIBRARY_DIRS})
  else()
    include_directories(${CURL_INCLUDE_DIR})
    link_directories(${CURL_LIBRARY_DIR})
  endif (MSVC)
    
endmacro (INCLUDE_CURL)
  
macro (LINK_CURL)
  if (MSVC)
    target_link_libraries (${TARGET_NAME}
      debug libcurld_imp
      optimized libcurl_imp)
  else (MSVC)
    target_link_libraries (${TARGET_NAME} curl)
  endif (MSVC)
endmacro (LINK_CURL)
