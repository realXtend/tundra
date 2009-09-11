# Find, include and link xmlrpc-epi
# Find is already called in the top-level CMakeLists

macro (FIND_XMLRPCEPI)

if (UNIX)

include(FindPkgMacros)
findpkg_begin(XMLRPCEPI)


# Construct search paths from enviromental hits and OS spesific guesses

if (MSVC)
  set(XMLRPCEPI_PREFIX_GUESSES $ENV{NAALI_DEP_PATH}/xmlrpc-epi
      C:/xmlrpc-epi
      $ENV{PROGRAMFILES}/xmlrpc-epi)
elseif (UNIX)
  set(XMLRPCEPI_PREFIX_GUESSES 
      /opt/xmlrpc-epi
      /usr/include
      /usr/local
      /usr/lib/xmlrpc-epi
      /usr/local/xmlrpc-epi
      $ENV{HOME}/xmlrpc-epi
      $ENV{NAALI_DEP_PATH}
      $ENV{NAALI_DEP_PATH}/xmlrpc-epi)
endif()

set(XMLRPCEPI_PREFIX_PATH 
    ${XMLRPCEPI_HOME} $ENV{XMLRPCEPI_HOME} ${XMLRPCEPI_PREFIX_GUESSES})

create_search_paths(XMLRPCEPI)

# try to locate XMLRPC-EPI via pkg-config
use_pkgconfig(XMLRPC-EPI "XMLRPC-EPI")
# try to find framework on OSX
findpkg_framework(XMLRPC-EPI)

if (XMLRPC-EPI_FOUND)
  message(STATUS "xmlrpc-epi found through pkg-config")
  # Not needed
  set(XMLRPCEPI_LIBRARY_DIR "${XMLRPC-EPI_LIBRARY_DIRS}")
  set(XMLRPCEPI_INCLUDE_DIR "${XMLRPC-EPI_INCLUDE_DIRS}")
else ()
  message(STATUS "try to find xmlrpc-epi from guessed paths")
  find_path(XMLRPCEPI_INCLUDE_DIR xmlrpc.h HINTS ${XMLRPCEPI_INC_SEARCH_PATH} ${XMLRPCEPI_PKGC_INCLUDE_DIRS} ${XMLRPCEPI_PREFIX_PATH})
  if (MSVC)
    find_path(XMLRPCEPI_LIBRARY_DIR xmlrpcepi.lib HINTS ${XMLRPCEPI_LIB_SEARCH_PATH})
  else ()
    find_path(XMLRPCEPI_LIBRARY_DIR libxmlrpc-epi.a HINTS ${XMLRPCEPI_LIB_SEARCH_PATH})
  endif()
endif()

if (NOT MSVC AND NOT XMLRPC-EPI_FOUND AND NOT XMLRPCEPI_INCLUDE_DIR OR NOT XMLRPCEPI_LIBRARY_DIR)
  message(STATUS "xmlrpc-epi was not found either guessed paths or pkg-config, please add enviroment variable XMLRPCEPI_HOME")
endif()

endif()
endmacro (FIND_XMLRPCEPI)

macro (INCLUDE_XMLRPCEPI)
    if (MSVC)
      set (XMLRPCEPI_INCLUDE_DIRS ${NAALI_DEP_PATH}/xmlrpc-epi/src)
      set (XMLRPCEPI_LINK_DIRS ${NAALI_DEP_PATH}/xmlrpc-epi)
      include_directories (${XMLRPCEPI_INCLUDE_DIRS})
      link_directories (${XMLRPCEPI_LINK_DIRS})
    else(NOT MSVC AND XMLRPC-EPI_FOUND)
      include_directories (${XMLRPC-EPI_INCLUDE_DIRS})
      link_directories(${XMLRPC-EPI_LIBRARY_DIRS})
    else()  
      include_directories (${XMLRPCEPI_INCLUDE_DIR})
      link_directories(${XMLRPCEPI_LIBRARY_DIR})
    endif ()

endmacro (INCLUDE_XMLRPCEPI)

macro (LINK_XMLRPCEPI)
	if (MSVC)
		target_link_libraries (${TARGET_NAME}
			debug xmlrpcepid
			optimized xmlrpcepi)
	else (MSVC)
		target_link_libraries (${TARGET_NAME} xmlrpc-epi)
	endif (MSVC)
endmacro (LINK_XMLRPCEPI)
