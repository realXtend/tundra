# Find, include and link Qt4 Find is already called in the top-level 
# CMakeLists

macro (FIND_QT4)

include(FindPkgMacros)
findpkg_begin(Qt4)

# Construct search paths from enviromental hits and OS spesific guesses

# PENDING fix version number depency. 

if (MSVC)
  set(Qt4_PREFIX_GUESSES $ENV{REX_DEP_PATH}/Qt4
      C:/Trolltech/Qt-4.5.1/
      $ENV{PROGRAMFILES}/Trolltech/Qt-4.5.1 C:/Ogre $ENV{REX_DEP_PATH}/Qt)
elseif (UNIX)
  set(Qt4_PREFIX_GUESSES 
      /opt/Trolltech/Qt-4.5.1
      /opt/Trolltech/Qt4
      /usr/local/Trolltech/Qt
      /usr/local/Trolltech/Qt-4.5.1
      /usr/local/Trolltech/Qt-4.5.1/lib
      /usr/local/Trolltech/Qt-4.5.1/include
      /usr/lib/
      /usr/local/include
      $ENV{HOME}/Qt4
      $ENV{HOME}/Qt
      $ENV{REX_DEP_PATH}/Qt4
      $ENV{REX_DEP_PATH}/qt4)
endif()

set(Qt4_PREFIX_PATH 
    ${Qt4_HOME} $ENV{Qt4_HOME} ${Qt4_PREFIX_GUESSES})

create_search_paths(Qt4)

# try to locate Ogre via pkg-config
use_pkgconfig(Qt4 "Qt4")

# try to find framework on OSX
findpkg_framework(Qt4)

if (Qt4_FOUND)
  message(STATUS "Qt4 found through pkg-config")
  # Not needed
  set(Qt4_LIBRARY_DIR "${QT4_LIBRARY_DIRS}")
  set(Qt4_INCLUDE_DIR "${QT4_INCLUDE_DIRS}")
else ()
  message(STATUS "try to find Qt4 from guessed paths")
  find_path(Qt4_INCLUDE_DIR QtCore/qstack.h HINTS ${Qt4_INC_SEARCH_PATH} ${Qt4_PKGC_INCLUDE_DIRS})
  if (MSVC)
    find_path(Qt4_LIBRARY_DIR  QtCore4.lib HINTS ${Qt4_LIB_SEARCH_PATH})
  else ()
    find_path(Qt4_LIBRARY_DIR libQtCore4.so HINTS ${Qt4_LIB_SEARCH_PATH})
  endif()
endif()

if (NOT MSVC AND NOT Qt4_FOUND AND NOT Qt4_INCLUDE_DIR OR NOT Qt4_LIBRARY_DIR)
  message(STATUS "Qt4 was not found either guessed paths or pkg-config, please add enviroment variable QT4_HOME")
endif()

endmacro (FIND_QT4)

macro (INCLUDE_QT4)
	
  INCLUDE(${QT_USE_FILE})
  
  if (MSVC)
    if (DEFINED ENV{Qt4_HOME})
      include_directories ($ENV{Qt4_HOME}/include)
      link_directories ($ENV{Qt4_HOME}/lib)
    else()
      include_directories (${REX_DEP_PATH}/Qt/include)
      link_directories (${REX_DEP_PATH}/Qt/lib)
    endif()
  elseif (NOT MSVC AND Qt4_FOUND)
    include_directories(${Qt4_INCLUDE_DIRS})
    link_directories(${Qt4_LIBRARY_DIRS})
  else()
    include_directories(${Qt4_INCLUDE_DIR})
    link_directories(${Qt4_LIBRARY_DIR})
  endif()
  
	
endmacro (INCLUDE_QT4)

macro (LINK_QT4)

	if (MSVC)
	  target_link_libraries (${TARGET_NAME}
	    QtCore4  QtGui4  qtmain  QtNetwork4  QtWebKit4
	    )
	elseif (NOT MSVC AND Qt4_FOUND)
	  target_link_libraries (${TARGET_NAME} ${Qt4_LIBRARIES})
	else()	
	  target_link_libraries(${TARGET_NAME} QtCore4 QtGui4  qtmain  QtNetwork4  QtWebKit4)
	endif (MSVC)

endmacro (LINK_QT4)
