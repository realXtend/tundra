# =============================================================================
# per-dependency configuration macros
#
# All per-dependency configuration (or hacks) should go here. All per-module
# build instructions should go in <Module>/CMakeLists.txt. The rest should
# remain generic.

macro(configure_boost)

if (MSVC)
    set(Boost_USE_MULTITHREADED TRUE)
    set(Boost_USE_STATIC_LIBS TRUE)
else ()
    set(Boost_USE_MULTITHREADED FALSE)
    set(Boost_USE_STATIC_LIBS FALSE)
endif ()

# Boost lookup rules:
# 1. If a CMake variable BOOST_ROOT was set before calling configure_boost(), that directory is used.
# 2. Otherwise, if an environment variable BOOST_ROOT was set, use that.
# 3. Otherwise, use Boost from the Tundra deps directory.

if ("${BOOST_ROOT}" STREQUAL "")
    file (TO_CMAKE_PATH "$ENV{BOOST_ROOT}" BOOST_ROOT)
endif()

if ("${BOOST_ROOT}" STREQUAL "")
    SET(BOOST_ROOT ${ENV_TUNDRA_DEP_PATH}/boost)
endif()

message("** Configuring Boost")
message(STATUS "Using BOOST_ROOT = " ${BOOST_ROOT})

# We build boost from custom deps directory, so don't look up boost from system.
if (ANDROID)
   set(Boost_NO_SYSTEM_PATHS TRUE)
endif()

set(Boost_FIND_REQUIRED TRUE)
set(Boost_FIND_QUIETLY TRUE)
set(Boost_DEBUG FALSE)
set(Boost_USE_MULTITHREADED TRUE)
set(Boost_DETAILED_FAILURE_MSG FALSE)
set(Boost_ADDITIONAL_VERSIONS "1.39.0" "1.40.0" "1.41.0" "1.42.0" "1.43.0" "1.44.0" "1.46.1")

if (APPLE OR MSVC)
   find_package(Boost 1.39.0 COMPONENTS thread regex)
else()
   find_package(Boost 1.39.0 COMPONENTS system thread regex) # Some Ubuntu 12.10 installs require system, others do not. OSX fails with system. Not needed on MSVC
endif()

if (Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
   link_directories(${Boost_LIBRARY_DIRS})
   
   message(STATUS "-- Include Directories")
   foreach(include_dir ${Boost_INCLUDE_DIRS})
      message (STATUS "       " ${include_dir})
   endforeach()
   message(STATUS "-- Library Directories")
   foreach(library_dir ${Boost_LIBRARY_DIRS})
      message (STATUS "       " ${library_dir})
   endforeach()
   message("")
else()
   message(FATAL_ERROR "Boost not found!")
endif()

# On Android, pthread library does not exist. Remove it if mistakenly added to boost libraries
if (ANDROID)
    list(REMOVE_ITEM Boost_LIBRARIES "pthread")
endif()

endmacro (configure_boost)

macro(configure_qt4)
    if (NOT ANDROID)
        sagase_configure_package(QT4
            NAMES Qt4 4.6.1
            COMPONENTS QtCore QtGui QtWebkit QtScript QtScriptTools QtXml QtNetwork QtUiTools
        PREFIXES ${ENV_QT_DIR} ${ENV_TUNDRA_DEP_PATH})
    else()
        find_package(Qt4 COMPONENTS QtCore QtGui QtXml QtNetwork QtScript QtUiTools)
    endif()

    # FindQt4.cmake
    if (QT4_FOUND AND QT_USE_FILE)
        include(${QT_USE_FILE})
        set(QT4_INCLUDE_DIRS 
            ${QT_INCLUDE_DIR}
            ${QT_QTCORE_INCLUDE_DIR}
            ${QT_QTGUI_INCLUDE_DIR}
            ${QT_QTUITOOLS_INCLUDE_DIR}
            ${QT_QTNETWORK_INCLUDE_DIR}
            ${QT_QTXML_INCLUDE_DIR}
            ${QT_QTSCRIPT_INCLUDE_DIR}
            ${QT_QTWEBKIT_INCLUDE_DIR})
#            ${QT_DECLARATIVE_INCLUDE_DIR}
#            ${QT_QTSCRIPTTOOLS_INCLUDE_DIR}
#            ${QT_PHONON_INCLUDE_DIR}
        
        set(QT4_LIBRARY_DIR ${QT_LIBRARY_DIR})

        if (ANDROID)
            set(QT4_LIBRARIES ${QT_LIBRARIES})
        else()
            set(QT4_LIBRARIES 
                ${QT_LIBRARIES}
                ${QT_QTCORE_LIBRARY}
                ${QT_QTGUI_LIBRARY}
                ${QT_QTUITOOLS_LIBRARY}
                ${QT_QTNETWORK_LIBRARY}
                ${QT_QTXML_LIBRARY}
                ${QT_QTSCRIPT_LIBRARY}
                ${QT_QTWEBKIT_LIBRARY})
#                ${QT_DECLARATIVE_LIBRARY}
#                ${QT_QTSCRIPTTOOLS_LIBRARY}
#                ${QT_PHONON_LIBRARY}
        endif()
    endif ()
    sagase_configure_report(QT4)
endmacro(configure_qt4)

macro (configure_python)
    sagase_configure_package (PYTHON
        NAMES PythonLibs Python python Python26 python26 Python2.6 python2.6
        COMPONENTS Python python Python26 python Python2.6 python2.6
        PREFIXES ${ENV_TUNDRA_DEP_PATH})

    # FindPythonLibs.cmake
    if (PYTHONLIBS_FOUND)
        set (PYTHON_LIBRARIES ${PYTHON_LIBRARY})
        set (PYTHON_INCLUDE_DIRS ${PYTHON_INCLUDE_PATH})
        #unset (PYTHON_DEBUG_LIBRARIES ${PYTHON_DEBUG_LIBRARY})
    endif ()
    
    # FindPythonLibs.cmake prefers the system-wide Python, which does not
    # include debug libraries, so we force to TUNDRA_DEP_PATH.
    if (MSVC)
        set (PYTHON_LIBRARY_DIRS ${ENV_TUNDRA_DEP_PATH}/Python/lib)
        set (PYTHON_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/Python/include)
        set (PYTHON_LIBRARIES python26)
        set (PYTHON_DEBUG_LIBRARIES python26_d)
    endif()
    
    sagase_configure_report (PYTHON)
endmacro (configure_python)

macro (configure_python_qt)
    sagase_configure_package (PYTHON_QT
        NAMES PythonQt
        COMPONENTS PythonQt PythonQt_QtAll
        PREFIXES ${ENV_TUNDRA_DEP_PATH})

    sagase_configure_report (PYTHON_QT)
endmacro (configure_python_qt)

macro (configure_qtpropertybrowser)
    if (NOT MSVC AND NOT APPLE)
      sagase_configure_package (QT_PROPERTY_BROWSER
          NAMES QtPropertyBrowser QtSolutions_PropertyBrowser-2.5 QtSolutions_PropertyBrowser-head
          COMPONENTS QtPropertyBrowser QtSolutions_PropertyBrowser-2.5 QtSolutions_PropertyBrowser-head
          PREFIXES ${ENV_TUNDRA_DEP_PATH})

      sagase_configure_report (QT_PROPERTY_BROWSER)
    endif()
endmacro (configure_qtpropertybrowser)

macro(configure_openal)
    if (CMAKE_CL_64)
        SET(WIN_PLATFORM Win64)
    else()
        set(WIN_PLATFORM Win32)
    endif()
    sagase_configure_package(OPENAL
        NAMES OpenAL openal
        COMPONENTS al OpenAL32
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/OpenAL ${ENV_TUNDRA_DEP_PATH}/OpenAL/libs/${WIN_PLATFORM})

        if (OPENAL_FOUND)
            set (OPENAL_LIBRARIES ${OPENAL_LIBRARY})
            set (OPENAL_INCLUDE_DIRS ${OPENAL_INCLUDE_DIR})
        endif()

        # Force include dir on MSVC
        if (MSVC)
             set (OPENAL_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/OpenAL/include)
        endif ()
    sagase_configure_report (OPENAL)
endmacro (configure_openal)

macro(use_package_knet)
    # kNet look up rules:
    # 1. Use cmake cached KNET_DIR.
    # 2. Use env variable KNET_DIR and cache it.
    # 3. Assume kNet from deps path.

    message("** Configuring kNet")
    if ("${KNET_DIR}" STREQUAL "")
        file (TO_CMAKE_PATH "$ENV{KNET_DIR}" KNET_DIR)
        # Cache KNET_DIR for runs that dont define $ENV{KNET_DIR}.
        set (KNET_DIR ${KNET_DIR} CACHE PATH "KNET_DIR dependency path" FORCE)
    endif ()
    if ("${KNET_DIR}" STREQUAL "")
        if (MSVC OR APPLE)
            set(KNET_DIR ${ENV_TUNDRA_DEP_PATH}/kNet)
        else() # Remove this when Linux build directories are unified with the other OSes
            set(KNET_DIR ${ENV_TUNDRA_DEP_PATH})
        endif()
    endif()
    message (STATUS "Using KNET_DIR = ${KNET_DIR}")
    
    include_directories (${KNET_DIR}/include)
    link_directories (${KNET_DIR}/lib)
endmacro()

macro(link_package_knet)
    target_link_libraries(${TARGET_NAME} optimized kNet)
    target_link_libraries(${TARGET_NAME} debug kNet)
endmacro()

macro(use_package_bullet)
    # Bullet look up rules:
    # 1. Use cmake cached BULLET_DIR.
    # 2. Use env variable BULLET_DIR and cache it.
    # 3. Assume Bullet from deps path.

    message("** Configuring Bullet")
    if ("${BULLET_DIR}" STREQUAL "")
        file (TO_CMAKE_PATH "$ENV{BULLET_DIR}" BULLET_DIR)
        # Cache BULLET_DIR for runs that dont define $ENV{BULLET_DIR}.
        set (BULLET_DIR ${BULLET_DIR} CACHE PATH "BULLET_DIR dependency path" FORCE)
    endif ()
    if ("${BULLET_DIR}" STREQUAL "")
        if (MSVC OR APPLE)
            set(BULLET_DIR ${ENV_TUNDRA_DEP_PATH}/Bullet)
        else() # Remove this when Linux build directories are unified with the other OSes
            set(BULLET_DIR ${ENV_TUNDRA_DEP_PATH})
        endif()
    endif()
    message (STATUS "Using BULLET_DIR = ${BULLET_DIR}")

    if (WIN32 OR ANDROID)
        include_directories(${BULLET_DIR}/src) # For full-built source deps.
        if (NOT ANDROID)
            link_directories(${BULLET_DIR}/lib)
        else ()
            link_directories(${BULLET_DIR}/libs/${ANDROID_ABI})
        endif()
    else () # Linux and mac
        include_directories(${BULLET_DIR}/include/bullet)
        link_directories(${BULLET_DIR}/lib)
    endif ()
endmacro()

macro(link_package_bullet)
    if (WIN32) # Full-build deps 
        target_link_libraries(${TARGET_NAME} debug LinearMath.lib debug BulletDynamics.lib BulletCollision.lib)
        target_link_libraries(${TARGET_NAME} optimized LinearMath.lib optimized BulletDynamics.lib optimized BulletCollision.lib)
    else()
        target_link_libraries(${TARGET_NAME} optimized BulletDynamics optimized BulletCollision optimized LinearMath)
    endif()
endmacro()

macro(use_package_ogg)
    # Using full-built deps
    include_directories(${ENV_TUNDRA_DEP_PATH}/ogg/include)
    link_directories(${ENV_TUNDRA_DEP_PATH}/ogg/lib)
endmacro()

macro(link_package_ogg)
    if (MSVC)
        # Always use ENV_TUNDRA_DEP_PATH as its read from cache. $ENV{TUNDRA_DEP_PATH} is not and can be empty/incorrect.
        if (MSVC90)
            set(VS2008_OR_VS2010 "VS2008")
        else()
            set(VS2008_OR_VS2010 "VS2010")
        endif()
        target_link_libraries(${TARGET_NAME} optimized ${ENV_TUNDRA_DEP_PATH}/ogg/win32/${VS2008_OR_VS2010}/$(Platform)/Release/libogg_static.lib)
        target_link_libraries(${TARGET_NAME} debug ${ENV_TUNDRA_DEP_PATH}/ogg/win32/${VS2008_OR_VS2010}/$(Platform)/Debug/libogg_static.lib)
    else()
        target_link_libraries(${TARGET_NAME} general ogg)
    endif()
endmacro()

macro(use_package_vorbis)
    # Using full-built deps made from fullbuild.
    include_directories(${ENV_TUNDRA_DEP_PATH}/vorbis/include)
    link_directories(${ENV_TUNDRA_DEP_PATH}/vorbis/lib)
endmacro()

macro(link_package_vorbis)
    if (MSVC)
        # Always use ENV_TUNDRA_DEP_PATH as its read from cache. $ENV{TUNDRA_DEP_PATH} is not and can be empty/incorrect.
        if (MSVC90)
            set(VS2008_OR_VS2010 "VS2008")
        else()
            set(VS2008_OR_VS2010 "VS2010")
        endif()
        target_link_libraries(${TARGET_NAME} optimized ${ENV_TUNDRA_DEP_PATH}/vorbis/win32/${VS2008_OR_VS2010}/$(Platform)/Release/libvorbis_static.lib)
        target_link_libraries(${TARGET_NAME} optimized ${ENV_TUNDRA_DEP_PATH}/vorbis/win32/${VS2008_OR_VS2010}/$(Platform)/Release/libvorbisfile_static.lib)
        target_link_libraries(${TARGET_NAME} debug ${ENV_TUNDRA_DEP_PATH}/vorbis/win32/${VS2008_OR_VS2010}/$(Platform)/Debug/libvorbis_static.lib)
        target_link_libraries(${TARGET_NAME} debug ${ENV_TUNDRA_DEP_PATH}/vorbis/win32/${VS2008_OR_VS2010}/$(Platform)/Debug/libvorbisfile_static.lib)
    else()
        target_link_libraries(${TARGET_NAME} general vorbis general vorbisfile)
    endif()
endmacro()

macro(use_package_theora)
    # Using full-built deps made from fullbui
    include_directories(${ENV_TUNDRA_DEP_PATH}/theora/include)
    link_directories(${ENV_TUNDRA_DEP_PATH}/theora/lib)
endmacro()

macro(link_package_theora)
    if (MSVC)
        # Always use ENV_TUNDRA_DEP_PATH as its read from cache. $ENV{TUNDRA_DEP_PATH} is not and can be empty/incorrect.
        # Using full-built deps.
        target_link_libraries(${TARGET_NAME} optimized ${ENV_TUNDRA_DEP_PATH}/theora/win32/VS2008/$(Platform)/Release_SSE2/libtheora_static.lib)
        target_link_libraries(${TARGET_NAME} debug ${ENV_TUNDRA_DEP_PATH}/theora/win32/VS2008/$(Platform)/Debug/libtheora_static.lib)
    else()
        target_link_libraries(${TARGET_NAME} general theora)
    endif()
endmacro()

macro(use_package_qtpropertybrowser)
    include_directories(${ENV_TUNDRA_DEP_PATH}/qt-solutions/qtpropertybrowser/src) # For full-built deps.
    link_directories(${ENV_TUNDRA_DEP_PATH}/qt-solutions/qtpropertybrowser/lib) # For full-built deps.
endmacro()

macro(link_package_qtpropertybrowser)
    if (MSVC)
        target_link_libraries(${TARGET_NAME} debug QtSolutions_PropertyBrowser-headd.lib)
        target_link_libraries(${TARGET_NAME} optimized QtSolutions_PropertyBrowser-head.lib)
    endif()
endmacro()

macro(use_package_assimp)
    if (WIN32 OR APPLE)
        if ("${ENV_ASSIMP_DIR}" STREQUAL "")
           message (STATUS "-- Defaulting to ${ENV_TUNDRA_DEP_PATH}/assimp")
           set(ASSIMP_DIR ${ENV_TUNDRA_DEP_PATH}/assimp)
        else ()
            message (STATUS "-- Using from env variable ASSIMP_DIR")
            set(ASSIMP_DIR ${ENV_ASSIMP_DIR})
        endif()
        include_directories(${ASSIMP_DIR}/include)
        link_directories(${ASSIMP_DIR}/lib)
    elseif (LINUX)
        if ("${ENV_ASSIMP_DIR}" STREQUAL "")
            set(ASSIMP_DIR ${ENV_TUNDRA_DEP_PATH})
        else ()
            message (STATUS "-- Using from env variable ASSIMP_DIR")
            set(ASSIMP_DIR ${ENV_ASSIMP_DIR})
        endif()
        include_directories(${ASSIMP_DIR}/include/assimp)
        link_directories(${ASSIMP_DIR}/lib)
    endif()
endmacro()

macro(link_package_assimp)
    target_link_libraries(${TARGET_NAME} optimized assimp)
    if (WIN32)
        target_link_libraries(${TARGET_NAME} debug assimpd)
    endif()
endmacro()
