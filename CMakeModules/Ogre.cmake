# Find, include and link Ogre Find is already called in the top-level 
# CMakeLists

macro (FIND_OGRE)

include(FindPkgMacros)
findpkg_begin(OGRE)

# Construct search paths from enviromental hits and OS spesific guesses

if (MSVC)
  set(OGRE_PREFIX_GUESSES $ENV{REX_DEP_PATH}/Ogre
      C:/OgreSDK
      $ENV{PROGRAMFILES}/OgreSDK C:/Ogre $ENV{REX_DEP_PATH}/ogre)
elseif (UNIX)
  set(OGRE_PREFIX_GUESSES 
      /opt/OGRE
      /opt/ogre
      /usr/local
      /usr/lib/OGRE
      /usr/lib/ogre
      /usr/local/OGRE
      /usr/local/ogre
      $ENV{HOME}/Ogre
      $ENV{HOME}/OGRE
      $ENV{REX_DEP_PATH}/Ogre
      $ENV{REX_DEP_PATH}/ogre)
endif()

set(OGRE_PREFIX_PATH 
    ${OGRE_HOME} $ENV{OGRE_HOME} ${OGRE_PREFIX_GUESSES})

create_search_paths(OGRE)

# try to locate Ogre via pkg-config
use_pkgconfig(OGRE "OGRE")

# try to find framework on OSX
findpkg_framework(OGRE)

if (OGRE_FOUND)
  message(STATUS "Ogre found through pkg-config")
  # Not needed
  set(OGRE_LIBRARY_DIR "${OGRE_LIBRARY_DIRS}")
  set(OGRE_INCLUDE_DIR "${OGRE_INCLUDE_DIRS}")
else ()
  message(STATUS "try to find Ogre from guessed paths")
  find_path(OGRE_INCLUDE_DIR Ogre.h HINTS ${OGRE_INC_SEARCH_PATH} ${OGRE_PKGC_INCLUDE_DIRS})
  if (MSVC)
    find_path(OGRE_LIBRARY_DIR  OgreMain.lib HINTS ${OGRE_LIB_SEARCH_PATH})
  else ()
    find_path(OGRE_LIBRARY_DIR libOgreMain.so HINTS ${OGRE_LIB_SEARCH_PATH})
  endif()
endif()

if (NOT MSVC AND NOT OGRE_FOUND AND NOT OGRE_INCLUDE_DIR OR NOT OGRE_LIBRARY_DIR)
  message(STATUS "Ogre was not found either guessed paths or pkg-config, please add enviroment variable OGRE_HOME")
endif()

#	if (NOT MSVC)
#		find_package (PkgConfig)
#		pkg_search_module (OGRE OGRE)
#		if (NOT OGRE_INCLUDE_DIRS AND NOT OGRE_LIBRARIES)
#			message (FATAL_ERROR "Ogre not found by pkg-config")
#		endif (NOT OGRE_INCLUDE_DIRS AND NOT OGRE_LIBRARIES)
#		separate_arguments (OGRE_INCLUDE_DIRS)
#		separate_arguments (OGRE_LIBRARIES)
#	endif (NOT MSVC)

endmacro (FIND_OGRE)

macro (INCLUDE_OGRE)
	if (MSVC)
	  if (DEFINED ENV{OGRE_HOME})
	    include_directories ($ENV{OGRE_HOME}/include)
	    link_directories ($ENV{OGRE_HOME}/lib)
	  else()
	    include_directories (${REX_DEP_PATH}/Ogre/include)
	    link_directories (${REX_DEP_PATH}/Ogre/lib)
	  endif()
        elseif (NOT MSVC AND OGRE_FOUND)
	  include_directories(${OGRE_INCLUDE_DIRS})
	  link_directories(${OGRE_LIBRARY_DIRS})
	else()
	  include_directories(${OGRE_INCLUDE_DIR})
	  link_directories(${OGRE_LIBRARY_DIR})
	  #include_directories (${REX_DEP_PATH}/Ogre/include)
	  #link_directories (${REX_DEP_PATH}/Ogre/lib)
	endif()
		
	#else (MSVC)
	#	include_directories (${OGRE_INCLUDE_DIRS})
	#	link_directories (${OGRE_LIBDIR})
	#endif (MSVC)

endmacro (INCLUDE_OGRE)

macro (LINK_OGRE)

	if (MSVC)
	  target_link_libraries (${TARGET_NAME}
	    debug OgreMain_d
	    optimized OgreMain)
	elseif (NOT MSVC AND OGRE_FOUND)
	  target_link_libraries (${TARGET_NAME} ${OGRE_LIBRARIES})
	else()	
	  target_link_libraries(${TARGET_NAME} OgreMain)
	endif (MSVC)

endmacro (LINK_OGRE)
