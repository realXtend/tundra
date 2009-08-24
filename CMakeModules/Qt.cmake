# Find, include and link Qt4 Find is already called in the top-level
# CMakeLists

macro (FIND_QT4)

if (MSVC)
FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtWebkit QtScript REQUIRED)

elseif (UNIX)

include(FindPkgMacros)
findpkg_begin(Qt4)


# Construct search paths from enviromental hits and OS spesific guesses

# PENDING fix version number depency. 
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
    $ENV{REX_DEP_PATH}
    $ENV{REX_DEP_PATH}/Qt4
    $ENV{REX_DEP_PATH}/qt4)

set(Qt4_PREFIX_PATH 
    ${Qt4_HOME} $ENV{Qt4_HOME} ${Qt4_PREFIX_GUESSES})

create_search_paths(Qt4)

# try to locate Qt4 via pkg-config
find_package( Qt4 )

if (QT4_FOUND )
  message(STATUS "Qt4 found through pkg-config")
else ()
  message(STATUS "try to find Qt4 from guessed paths")
  find_path(Qt4_INCLUDE_DIR QtCore/qstack.h HINTS ${Qt4_INC_SEARCH_PATH} ${Qt4_PKGC_INCLUDE_DIRS})
  find_path(Qt4_LIBRARY_DIR libQtCore.so HINTS ${Qt4_LIB_SEARCH_PATH})
endif()

if (NOT MSVC AND NOT QT4_FOUND AND NOT Qt4_INCLUDE_DIR OR NOT Qt4_LIBRARY_DIR)
  message(STATUS "Qt4 was not found either guessed paths or pkg-config, please add enviroment variable QT4_HOME")
endif()

endif()

endmacro (FIND_QT4)

macro (INCLUDE_QT4)
  
  if (MSVC)
    if (DEFINED ENV{QTDIR})
      include_directories ($ENV{QTDIR}/include ${QT_INCLUDE_DIR}
      ${QT_QTWEBKIT_INCLUDE_DIR}
      ${QT_QTCORE_INCLUDE_DIR}
      ${QT_QTDBUS_INCLUDE_DIR}
      ${QT_QTGUI_INCLUDE_DIR}
      ${QT_QTNETWORK_INCLUDE_DIR}
      ${QT_QTSCRIPT_INCLUDE_DIR}
      ${QT_QTSQL_DIR}
      ${QT_QTUITOOLS_INCLUDE_DIR}
      ${QT_QTXML_INCLUDE_DIR})
      link_directories ($ENV{QTDIR}/lib ${QT_LIBRARIES})
    else()
      include_directories (${REX_DEP_PATH}/Qt/include)
      link_directories (${REX_DEP_PATH}/Qt/lib)
    endif()
  elseif (NOT MSVC AND QT4_FOUND)
    include_directories(${QT_INCLUDE_DIR} ${QT_QTWEBKIT_INCLUDE_DIR}
      ${QT_QTCORE_INCLUDE_DIR}
      ${QT_QTDBUS_INCLUDE_DIR}
      ${QT_QTGUI_INCLUDE_DIR}
      ${QT_QTNETWORK_INCLUDE_DIR}
      ${QT_QTSCRIPT_INCLUDE_DIR}
      ${QT_QTSQL_DIR}
      ${QT_QTUITOOLS_INCLUDE_DIR}
      ${QT_QTXML_INCLUDE_DIR})
    link_directories(${QT_LIBRARY_DIRS})
  else()
    include_directories(${Qt4_INCLUDE_DIR})
    link_directories(${Qt4_LIBRARY_DIR})
  endif()


endmacro (INCLUDE_QT4)

macro (LINK_QT4)
  if (MSVC)
    target_link_libraries (${TARGET_NAME}
      ${QT_LIBRARIES}
      ${QT_QTGUI_LIBRARY}
      ${QT_QTCORE_LIBRARY}
      ${QT_QTWEBKIT_LIBRARY}
      ${QT_QTNETWORK_LIBRARY}
      ${QT_QTUITOOLS_LIBRARY}
      ${QT_QTXML_LIBRARY}
      ${QT_QTSCRIPT_LIBRARY}
      )
  elseif (NOT MSVC AND QT4_FOUND)
    target_link_libraries (${TARGET_NAME} ${QT_LIBRARIES} ${QT_QTGUI_LIBRARY}
      ${QT_QTWEBKIT_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY}
      ${QT_QTNETWORK_LIBRARY}
      ${QT_QTSCRIPT_LIBRARY}
      ${QT_QTSQL_LIBRARY}
      ${QT_QTUITOOLS_LIBRARY}
      ${QT_QTXML_LIBRARY}
      ${QT_QTSCRIPT_LIBRARY}
      )
  else()
    target_link_libraries(${TARGET_NAME} QtCore QtGui  qtmain  QtNetwork  QtWebKit QtUitools QtXml QtScript)
  endif (MSVC)

endmacro (LINK_QT4)
