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

message("\n** Configuring Boost")
message(STATUS "BOOST_ROOT = " ${BOOST_ROOT})

set(Boost_FIND_REQUIRED TRUE)
set(Boost_FIND_QUIETLY TRUE)
set(Boost_DEBUG FALSE)
set(Boost_USE_MULTITHREADED TRUE)
set(Boost_DETAILED_FAILURE_MSG FALSE)
set(Boost_ADDITIONAL_VERSIONS "1.39.0" "1.40.0" "1.41.0" "1.42.0" "1.43.0" "1.44.0" "1.46.1")

find_package(Boost 1.39.0 COMPONENTS thread regex)

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
	if (NOT MSVC)
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
    endif()
endmacro (configure_skyx)

macro (configure_hydrax)
	if (NOT MSVC AND NOT APPLE)
		# Prioritize env variable HYDRAX_HOME to be searched first
		# to allow custom hydrax builds agains a custom ogre (potentially from OGRE_HOME)
		sagase_configure_package (HYDRAX
			NAMES Hydrax HYDRAX hydrax
			COMPONENTS Hydrax HYDRAX hydrax
			PREFIXES ${ENV_HYDRAX_HOME} ${ENV_TUNDRA_DEP_PATH})

		set (HYDRAX_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/include/Hydrax)

		sagase_configure_report (HYDRAX)
	endif()
endmacro (configure_hydrax)

macro(use_package_hydrax)
    if (MSVC) # TODO inclusion chains for using Hydrax from deps for other platforms.
	
    # Hydrax lookup rules:
    # 1. If the environment variable HYDRAX_HOME is set, use that directory.
    # 2. Otherwise, use the deps directory path.

	if (NOT "$ENV{HYDRAX_HOME}" STREQUAL "")
		set(HYDRAX_HOME $ENV{HYDRAX_HOME})
	else()
		set(HYDRAX_HOME ${ENV_TUNDRA_DEP_PATH}/Hydrax)
	endif()

    include_directories(${HYDRAX_HOME}/include)
    link_directories(${HYDRAX_HOME}/lib)
	else()
		include_directories(${ENV_TUNDRA_DEP_PATH}/include/Hydrax)
	endif()
endmacro()

macro(link_package_hydrax)
	if (MSVC OR APPLE) # TODO linkage settings for using Hydrax from deps for other platforms.
		target_link_libraries(${TARGET_NAME} optimized Hydrax debug Hydraxd)
	endif()
endmacro()

macro(use_package_skyx)
    if (MSVC) # TODO inclusion chains for using SkyX from deps for other platforms.
	
    # SkyX lookup rules:
    # 1. If the environment variable SKYX_HOME is set, use that directory.
    # 2. Otherwise, use the deps directory path.

	if (NOT "$ENV{SKYX_HOME}" STREQUAL "")
		set(SKYX_HOME $ENV{SKYX_HOME})
	else()
		set(SKYX_HOME ${ENV_TUNDRA_DEP_PATH}/SkyX)
	endif()

    include_directories(${SKYX_HOME}/include) # For prebuilt VS2008/VS2010 deps.
    include_directories(${SKYX_HOME}/SkyX/include) # For full-built deps.
    link_directories(${SKYX_HOME}/lib)
	endif()
endmacro()

macro(link_package_skyx)
	if (MSVC) # TODO linkage settings for using Hydrax from deps for other platforms.
		target_link_libraries(${TARGET_NAME} optimized SkyX debug SkyX_d)
	endif()
endmacro()

macro (configure_qtpropertybrowser)
    if (NOT MSVC)
      sagase_configure_package (QT_PROPERTY_BROWSER
          NAMES QtPropertyBrowser QtSolutions_PropertyBrowser-2.5
          COMPONENTS QtPropertyBrowser QtSolutions_PropertyBrowser-2.5
          PREFIXES ${ENV_TUNDRA_DEP_PATH})

      sagase_configure_report (QT_PROPERTY_BROWSER)
    endif()
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
    set(KNET_DIR $ENV{KNET_DIR})
    
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
        include_directories(${BULLET_DIR}/include) # For prebuilt VS2008/VS2010 deps.
        include_directories(${BULLET_DIR}/src) # For full-built source deps.
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
    if (IS_DIRECTORY ${ENV_BULLET_DIR}/msvc/2008) # full prebuilt deps
        if (WIN32)
            target_link_libraries(${TARGET_NAME} debug ${ENV_BULLET_DIR}/msvc/2008/lib/debug/LinearMath.lib)
            target_link_libraries(${TARGET_NAME} debug ${ENV_BULLET_DIR}/msvc/2008/lib/debug/BulletDynamics.lib)
            target_link_libraries(${TARGET_NAME} debug ${ENV_BULLET_DIR}/msvc/2008/lib/debug/BulletCollision.lib)
            target_link_libraries(${TARGET_NAME} optimized ${ENV_BULLET_DIR}/msvc/2008/lib/release/LinearMath.lib)
            target_link_libraries(${TARGET_NAME} optimized ${ENV_BULLET_DIR}/msvc/2008/lib/release/BulletDynamics.lib)
            target_link_libraries(${TARGET_NAME} optimized ${ENV_BULLET_DIR}/msvc/2008/lib/release/BulletCollision.lib)
        endif()
    elseif (IS_DIRECTORY ${BULLET_DIR}/lib/Release) # prebuilt deps package
        if (WIN32)
            target_link_libraries(${TARGET_NAME} debug ${BULLET_DIR}/lib/Debug/LinearMath.lib)
            target_link_libraries(${TARGET_NAME} debug ${BULLET_DIR}/lib/Debug/BulletDynamics.lib)
            target_link_libraries(${TARGET_NAME} debug ${BULLET_DIR}/lib/Debug/BulletCollision.lib)
            target_link_libraries(${TARGET_NAME} optimized ${BULLET_DIR}/lib/Release/LinearMath.lib)
            target_link_libraries(${TARGET_NAME} optimized ${BULLET_DIR}/lib/Release/BulletDynamics.lib)
            target_link_libraries(${TARGET_NAME} optimized ${BULLET_DIR}/lib/Release/BulletCollision.lib)
        endif()
    else()
        target_link_libraries(${TARGET_NAME} optimized LinearMath optimized BulletDynamics optimized BulletCollision)
        if (WIN32)
            target_link_libraries(${TARGET_NAME} debug LinearMath_d debug BulletDynamics_d debug BulletCollision_d)
        endif()
    endif()
endmacro()

macro(use_package_ogg)
    if (MSVC)
        include_directories(${ENV_TUNDRA_DEP_PATH}/libogg/include) # For prebuilt VS2008/VS2010 deps.
        link_directories(${ENV_TUNDRA_DEP_PATH}/libogg/lib) # For prebuilt VS2008/VS2010 deps.

        include_directories($ENV{TUNDRA_DEP_PATH}/ogg/include) # For full-built deps.
    elseif (APPLE)
        include_directories(${ENV_TUNDRA_DEP_PATH}/include/ogg)
        link_directories(${ENV_TUNDRA_DEP_PATH}/lib)
    endif()
endmacro()

macro(link_package_ogg)
    if (MSVC)
        if (IS_DIRECTORY $ENV{TUNDRA_DEP_PATH}/ogg/win32/VS2008/Win32) # Using full-built deps.
            target_link_libraries(${TARGET_NAME} optimized $ENV{TUNDRA_DEP_PATH}/ogg/win32/VS2008/Win32/Release/libogg_static.lib)
            target_link_libraries(${TARGET_NAME} debug $ENV{TUNDRA_DEP_PATH}/ogg/win32/VS2008/Win32/Debug/libogg_static.lib)
        elseif (IS_DIRECTORY $ENV{TUNDRA_DEP_PATH}/ogg/lib/Release) # Using pre-built deps mirrored from full-built deps.
            target_link_libraries(${TARGET_NAME} optimized $ENV{TUNDRA_DEP_PATH}/ogg/lib/Release/libogg_static.lib)
            target_link_libraries(${TARGET_NAME} debug $ENV{TUNDRA_DEP_PATH}/ogg/lib/Debug/libogg_static.lib)
        else() # Using pre-built VS2008/VS2010 deps.
            target_link_libraries(${TARGET_NAME} optimized libogg)
            target_link_libraries(${TARGET_NAME} debug liboggd)
        endif()
    else()
        target_link_libraries(${TARGET_NAME} general ogg)
    endif()
endmacro()

macro(use_package_vorbis)
    if (MSVC)
        include_directories(${ENV_TUNDRA_DEP_PATH}/libvorbis/include) # For prebuilt VS2008/VS2010 deps.
        link_directories(${ENV_TUNDRA_DEP_PATH}/libvorbis/lib) # For prebuilt VS2008/VS2010 deps.

        include_directories(${ENV_TUNDRA_DEP_PATH}/vorbis/include) # For full-built deps.
    elseif (APPLE)
        include_directories(${ENV_TUNDRA_DEP_PATH}/include/vorbis)
        link_directories(${ENV_TUNDRA_DEP_PATH}/lib)
    endif()
endmacro()

macro(link_package_vorbis)
    if (MSVC)
        if (IS_DIRECTORY $ENV{TUNDRA_DEP_PATH}/vorbis/win32/VS2008/Win32) # Using full-built deps.
            target_link_libraries(${TARGET_NAME} optimized $ENV{TUNDRA_DEP_PATH}/vorbis/win32/VS2008/Win32/Release/libvorbis_static.lib)
            target_link_libraries(${TARGET_NAME} optimized $ENV{TUNDRA_DEP_PATH}/vorbis/win32/VS2008/Win32/Release/libvorbisfile_static.lib)
            target_link_libraries(${TARGET_NAME} debug $ENV{TUNDRA_DEP_PATH}/vorbis/win32/VS2008/Win32/Debug/libvorbis_static.lib)
            target_link_libraries(${TARGET_NAME} debug $ENV{TUNDRA_DEP_PATH}/vorbis/win32/VS2008/Win32/Debug/libvorbisfile_static.lib)
        elseif (IS_DIRECTORY $ENV{TUNDRA_DEP_PATH}/vorbis/lib/Release) # Using pre-built deps mirrored from full-built deps.
            target_link_libraries(${TARGET_NAME} optimized $ENV{TUNDRA_DEP_PATH}/vorbis/lib/Release/libvorbis_static.lib)
            target_link_libraries(${TARGET_NAME} optimized $ENV{TUNDRA_DEP_PATH}/vorbis/lib/Release/libvorbisfile_static.lib)
            target_link_libraries(${TARGET_NAME} debug $ENV{TUNDRA_DEP_PATH}/vorbis/lib/Debug/libvorbis_static.lib)
            target_link_libraries(${TARGET_NAME} debug $ENV{TUNDRA_DEP_PATH}/vorbis/lib/Debug/libvorbisfile_static.lib)  
        else() # Using pre-built VS2008/VS2010 deps.
            target_link_libraries(${TARGET_NAME} optimized libvorbis optimized libvorbisfile)
            target_link_libraries(${TARGET_NAME} debug libvorbisd debug libvorbisfiled)
        endif()
    else()
        target_link_libraries(${TARGET_NAME} general vorbis general vorbisfile)
    endif()
endmacro()

macro(use_package_theora)
    if (MSVC)
        include_directories(${ENV_TUNDRA_DEP_PATH}/libtheora/include) # For prebuilt VS2008/VS2010 deps.
        link_directories(${ENV_TUNDRA_DEP_PATH}/libtheora/lib) # For prebuilt VS2008/VS2010 deps.
        
        include_directories(${ENV_TUNDRA_DEP_PATH}/theora/include) # For full-built deps.        
    elseif (APPLE)
        include_directories(${ENV_TUNDRA_DEP_PATH}/include/theora)
        link_directories(${ENV_TUNDRA_DEP_PATH}/lib)
    endif()
endmacro()

macro(link_package_theora)
    if (MSVC)
        if (IS_DIRECTORY $ENV{TUNDRA_DEP_PATH}/theora/win32/VS2008/Win32) # Using full-built deps.
            target_link_libraries(${TARGET_NAME} optimized $ENV{TUNDRA_DEP_PATH}/theora/win32/VS2008/Win32/Release_SSE2/libtheora_static.lib)
            target_link_libraries(${TARGET_NAME} debug $ENV{TUNDRA_DEP_PATH}/theora/win32/VS2008/Win32/Debug/libtheora_static.lib)
        elseif (IS_DIRECTORY $ENV{TUNDRA_DEP_PATH}/theora/lib/Release_SSE2) # Using pre-built deps mirrored from full-built deps.
            target_link_libraries(${TARGET_NAME} optimized $ENV{TUNDRA_DEP_PATH}/theora/lib/Release_SSE2/libtheora_static.lib)
            target_link_libraries(${TARGET_NAME} debug $ENV{TUNDRA_DEP_PATH}/theora/lib/Debug/libtheora_static.lib)
        else() # Using pre-built VS2008/VS2010 deps.
            target_link_libraries(${TARGET_NAME} optimized libtheora)
            target_link_libraries(${TARGET_NAME} debug libtheorad)
        endif()
    else()
        target_link_libraries(${TARGET_NAME} general theora)
    endif()
endmacro()

macro(use_package_qtpropertybrowser)
    if (MSVC)
        include_directories(${ENV_TUNDRA_DEP_PATH}/qt-solutions/qtpropertybrowser/src) # For full-built deps.
        include_directories(${ENV_TUNDRA_DEP_PATH}/qtpropertybrowser/include) # For prebuilt deps mirrored from full-built deps.
        include_directories(${ENV_TUNDRA_DEP_PATH}/QtPropertyBrowser/includes) # For prebuilt deps vs2008.
        link_directories(${ENV_TUNDRA_DEP_PATH}/qt-solutions/qtpropertybrowser/lib) # For full-built deps.
        link_directories(${ENV_TUNDRA_DEP_PATH}/QtPropertyBrowser/lib) # For prebuilt deps vs2008.
    endif()
endmacro()

macro(link_package_qtpropertybrowser)
    if (MSVC)
        target_link_libraries(${TARGET_NAME} debug QtSolutions_PropertyBrowser-headd.lib)
        target_link_libraries(${TARGET_NAME} optimized QtSolutions_PropertyBrowser-head.lib)
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
