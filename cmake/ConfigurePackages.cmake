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
    set(Boost_USE_STATIC_LIBS FALSE)
endif ()

# Boost lookup rules:
# 1. If a CMake variable BOOST_ROOT was set before calling configure_boost(), that directory is used.
# 2. Otherwise, if an environment variable BOOST_ROOT was set, use that.
# 3. Otherwise, use Boost from the Tundra deps directory.

if ("${BOOST_ROOT}" STREQUAL "")
   SET(BOOST_ROOT $ENV{BOOST_ROOT})
endif()

if ("${BOOST_ROOT}" STREQUAL "")
   if (NOT APPLE)
      SET(BOOST_ROOT ${ENV_TUNDRA_DEP_PATH}/Boost)
   else()
      SET(BOOST_ROOT ${ENV_TUNDRA_DEP_PATH}/include)
   endif()
endif()

message(STATUS "BOOST_ROOT set to " ${BOOST_ROOT})

set(Boost_FIND_REQUIRED TRUE)
set(Boost_FIND_QUIETLY TRUE)
set(Boost_DEBUG FALSE)
set(Boost_USE_MULTITHREADED TRUE)
set(Boost_DETAILED_FAILURE_MSG FALSE)
set(Boost_ADDITIONAL_VERSIONS "1.39.0" "1.40.0" "1.41.0" "1.42.0" "1.43.0" "1.44.0" "1.46.1")
set(Boost_USE_STATIC_LIBS TRUE)

find_package(Boost 1.46.1 COMPONENTS thread regex)   

if (Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
   link_directories(${Boost_LIBRARY_DIRS})
else()
   message(FATAL_ERROR "Boost not found!")
endif()

if (APPLE)
    set (BOOST_LIBRARY_DIRS ${ENV_TUNDRA_DEP_PATH}/lib)
    set (BOOST_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/include)
endif()

endmacro (configure_boost)

macro (configure_qt4)
    sagase_configure_package (QT4 
        NAMES Qt4 4.6.1
        COMPONENTS QtCore QtGui QtWebkit QtScript QtScriptTools QtXml QtNetwork QtUiTools QtDeclarative
        PREFIXES ${ENV_TUNDRA_DEP_PATH} ${ENV_QT_DIR})

    # FindQt4.cmake
    if (QT4_FOUND AND QT_USE_FILE)
    
        include (${QT_USE_FILE})
        
        set (QT4_INCLUDE_DIRS 
            ${QT_INCLUDE_DIR}
            ${QT_QTCORE_INCLUDE_DIR}
            ${QT_QTGUI_INCLUDE_DIR}
            ${QT_QTUITOOLS_INCLUDE_DIR}
            ${QT_QTNETWORK_INCLUDE_DIR}
            ${QT_QTXML_INCLUDE_DIR}
            ${QT_QTSCRIPT_INCLUDE_DIR}
            ${QT_DECLARATIVE_INCLUDE_DIR}
            ${QT_QTWEBKIT_INCLUDE_DIR})
            
#            ${QT_QTSCRIPTTOOLS_INCLUDE_DIR}
#            ${QT_PHONON_INCLUDE_DIR}

        
        set (QT4_LIBRARY_DIR  
            ${QT_LIBRARY_DIR})
        
        set (QT4_LIBRARIES 
            ${QT_LIBRARIES}
            ${QT_QTCORE_LIBRARY}
            ${QT_QTGUI_LIBRARY}
            ${QT_QTUITOOLS_LIBRARY}
            ${QT_QTNETWORK_LIBRARY}
            ${QT_QTXML_LIBRARY}
            ${QT_QTSCRIPT_LIBRARY}
            ${QT_DECLARATIVE_LIBRARY}
            ${QT_QTWEBKIT_LIBRARY})
            
#            ${QT_QTSCRIPTTOOLS_LIBRARY}
#            ${QT_PHONON_LIBRARY}
        
    endif ()
    
    sagase_configure_report (QT4)
endmacro (configure_qt4)

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

macro (configure_skyx)
    # Prioritize env variable SKYX_HOME to be searched first
    # to allow custom skyx builds agains a custom ogre (potentially from OGRE_HOME)
    sagase_configure_package (SKYX
      NAMES SkyX SKYX skyx
      COMPONENTS SkyX SKYX skyx
      PREFIXES ${ENV_SKYX_HOME} ${ENV_TUNDRA_DEP_PATH})

    if (NOT WIN32)
      set (SKYX_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/include/SkyX)
    endif ()
    
    sagase_configure_report (SKYX)
endmacro (configure_skyx)

macro (configure_hydrax)
    # Prioritize env variable HYDRAX_HOME to be searched first
    # to allow custom hydrax builds agains a custom ogre (potentially from OGRE_HOME)
    sagase_configure_package (HYDRAX
        NAMES Hydrax HYDRAX hydrax
        COMPONENTS Hydrax HYDRAX hydrax
        PREFIXES ${ENV_HYDRAX_HOME} ${ENV_TUNDRA_DEP_PATH})

    if (NOT WIN32)
       set (HYDRAX_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/include/Hydrax)
    endif ()

    sagase_configure_report (HYDRAX)
endmacro (configure_hydrax)

macro (configure_qtpropertybrowser)
    sagase_configure_package (QT_PROPERTY_BROWSER
        NAMES QtPropertyBrowser QtSolutions_PropertyBrowser-2.5
        COMPONENTS QtPropertyBrowser QtSolutions_PropertyBrowser-2.5
        PREFIXES ${ENV_TUNDRA_DEP_PATH})
    
    sagase_configure_report (QT_PROPERTY_BROWSER)
endmacro (configure_qtpropertybrowser)

macro (configure_openal)
    sagase_configure_package(OPENAL
        NAMES OpenAL openal
        COMPONENTS al OpenAL32
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/OpenAL ${ENV_TUNDRA_DEP_PATH}/OpenAL/libs/Win32)

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

macro (configure_sparkle)
    FIND_LIBRARY (SPARKLE_LIBRARY NAMES Sparkle)
    set (SPARKLE_INCLUDE_DIRS ${SPARKLE_LIBRARY}/Headers)
    set (SPARKLE_LIBRARIES ${SPARKLE_LIBRARY})
endmacro (configure_sparkle)

macro(use_package_knet)
    set(KNET_DIR $ENV{KNET_DIR_QT47})
    
    # If KNET_DIR_QT47 was not specified, use kNet from TUNDRA_DEP_PATH.
    if ("${KNET_DIR}" STREQUAL "")
        if (MSVC)
            set(KNET_DIR ${ENV_TUNDRA_DEP_PATH}/kNet)
        else()
            set(KNET_DIR ${ENV_TUNDRA_DEP_PATH})
        endif()
    endif()

    include_directories (${KNET_DIR}/include)
    link_directories (${KNET_DIR}/lib)

    message (STATUS "Using kNet from ${KNET_DIR}")
endmacro()

macro(link_package_knet)
    target_link_libraries(${TARGET_NAME} optimized kNet)
    target_link_libraries(${TARGET_NAME} debug kNet)
endmacro()

macro(use_package_bullet)
    if (WIN32)
        if ("${ENV_BULLET_DIR}" STREQUAL "")
            set(BULLET_DIR ${ENV_TUNDRA_DEP_PATH}/Bullet)
        else ()
            message (STATUS "-- Using from env variable BULLET_DIR")
            set(BULLET_DIR ${ENV_BULLET_DIR})
        endif ()
        include_directories(${BULLET_DIR}/include)
        link_directories(${BULLET_DIR}/lib)
    else() # Linux, note: mac will also come here..
        if ("${ENV_BULLET_DIR}" STREQUAL "")
            set(BULLET_DIR ${ENV_TUNDRA_DEP_PATH})
        else ()
            message (STATUS "-- Using from env variable BULLET_DIR")
            set(BULLET_DIR ${ENV_BULLET_DIR})
        endif ()
        include_directories(${BULLET_DIR}/include/bullet)
        link_directories(${BULLET_DIR}/lib)
    endif()
endmacro()

macro(link_package_bullet)
    target_link_libraries(${TARGET_NAME} optimized LinearMath optimized BulletDynamics optimized BulletCollision)
    if (WIN32)
        target_link_libraries(${TARGET_NAME} debug LinearMath_d debug BulletDynamics_d debug BulletCollision_d)
    endif()
endmacro()

macro(use_package_ogg)
    if (MSVC)
        include_directories(${ENV_TUNDRA_DEP_PATH}/libogg/include)
        link_directories(${ENV_TUNDRA_DEP_PATH}/libogg/lib)
    elseif (APPLE)
        include_directories(${ENV_TUNDRA_DEP_PATH}/include/ogg)
        link_directories(${ENV_TUNDRA_DEP_PATH}/lib)
    endif()
endmacro()

macro(link_package_ogg)
    if (MSVC)
        target_link_libraries(${TARGET_NAME} optimized libogg)
        target_link_libraries(${TARGET_NAME} debug liboggd)
    else()
        target_link_libraries(${TARGET_NAME} general ogg)
    endif()
endmacro()

macro(use_package_vorbis)
    if (MSVC)
        include_directories(${ENV_TUNDRA_DEP_PATH}/libvorbis/include)
        link_directories(${ENV_TUNDRA_DEP_PATH}/libvorbis/lib)
    elseif (APPLE)
        include_directories(${ENV_TUNDRA_DEP_PATH}/include/vorbis)
        link_directories(${ENV_TUNDRA_DEP_PATH}/lib)
    endif()
endmacro()

macro(link_package_vorbis)
    if (MSVC)
        target_link_libraries(${TARGET_NAME} optimized libvorbis optimized libvorbisfile)
        target_link_libraries(${TARGET_NAME} debug libvorbisd debug libvorbisfiled)
    else()
        target_link_libraries(${TARGET_NAME} general vorbis general vorbisfile)
    endif()
endmacro()

macro(use_package_theora)
    if (MSVC)
        include_directories(${ENV_TUNDRA_DEP_PATH}/libtheora/include)
        link_directories(${ENV_TUNDRA_DEP_PATH}/libtheora/lib)
    elseif (APPLE)
        include_directories(${ENV_TUNDRA_DEP_PATH}/include/theora)
        link_directories(${ENV_TUNDRA_DEP_PATH}/lib)
    endif()
endmacro()

macro(link_package_theora)
    if (MSVC)
        target_link_libraries(${TARGET_NAME} optimized libtheora)
        target_link_libraries(${TARGET_NAME} debug libtheorad)
    else()
        target_link_libraries(${TARGET_NAME} general theora)
    endif()
endmacro()


macro(use_package_assimp)
    if (WIN32)
        if ("${ENV_ASSIMP_DIR}" STREQUAL "")
           set(ASSIMP_DIR ${ENV_TUNDRA_DEP_PATH}/assimp)
        else ()
            message (STATUS "-- Using from env variable ASSIMP_DIR")
            set(ASSIMP_DIR ${ENV_ASSIMP_DIR})
        endif()
        include_directories(${ASSIMP_DIR}/include)
        link_directories(${ASSIMP_DIR}/lib/assimp_debug_Win32)
        link_directories(${ASSIMP_DIR}/lib/assimp_release_Win32)
    else() # Linux, note: mac will also come here..
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
