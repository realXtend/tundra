# =============================================================================
# Configuration macros for global dependencies.
#
# All global dependency configurations should go here.
# All per-module dependency configurations should go in <Module>/CMakeLists.txt.

macro(configure_qt4)
    if (NOT ANDROID)
        sagase_configure_package(QT4
            NAMES Qt4 4.6.1
            COMPONENTS QtCore QtGui QtWebkit QtScript QtXml QtNetwork QtUiTools
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

macro(configure_qjson)
    if ("${QJSON_ROOT}" STREQUAL "")
        file (TO_CMAKE_PATH "$ENV{QJSON_ROOT}" QJSON_ROOT)
        set (QJSON_ROOT ${QJSON_ROOT} CACHE PATH "QJSON_ROOT dependency path" FORCE)
    endif ()

    if ("${QJSON_ROOT}" STREQUAL "")
        if (WIN32 OR APPLE)
            set (QJSON_ROOT ${ENV_TUNDRA_DEP_PATH}/qjson)
        else ()
            set (QJSON_ROOT ${ENV_TUNDRA_DEP_PATH})
        endif ()
    endif ()

    # Find QJson/Parser header and back up one folder for <QJson/Parser> style includes.
    # Windows uses the /build directory for headers, as the install step is a bit wonky.
    find_path (QJSON_INCLUDE_DIR parser.h HINTS ${QJSON_ROOT}/build/include ${QJSON_ROOT}/include PATH_SUFFIXES qjson)
    RemoveLastElementFromPath(${QJSON_INCLUDE_DIR} QJSON_INCLUDE_DIRS)

    if (NOT MSVC)
        find_library (QJSON_LIBRARIES NAMES qjson HINTS ${QJSON_ROOT}/lib)
    else ()
        # We could use /build directory for the lib too. But this would not allow us to change build modes on the fly.
        # It's recommended to run the build modes deps script before building with it, but its still nicer to
        # pass in a single link directory and have your build type subdirs below it, Visual Studio will do the right thing.
        find_path (QJSON_LIBRARY_DIR NAMES qjson.lib HINTS ${QJSON_ROOT}/lib PATH_SUFFIXES Release RelWithDebInfo Debug)
        RemoveLastElementFromPath(${QJSON_LIBRARY_DIR} QJSON_LIBRARY_DIRS)
        set(QJSON_LIBRARIES qjson.lib)
    endif ()
    sagase_configure_report(QJSON)
endmacro(configure_qjson)

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
    if (WIN32)
        target_link_libraries(${TARGET_NAME} ws2_32.lib)
    endif ()
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
        target_link_libraries(${TARGET_NAME} optimized ${ENV_TUNDRA_DEP_PATH}/ogg/win32/${VS2008_OR_VS2010}/$(PlatformName)/Release/libogg_static.lib)
        target_link_libraries(${TARGET_NAME} debug ${ENV_TUNDRA_DEP_PATH}/ogg/win32/${VS2008_OR_VS2010}/$(PlatformName)/Debug/libogg_static.lib)
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
        target_link_libraries(${TARGET_NAME} optimized ${ENV_TUNDRA_DEP_PATH}/vorbis/win32/${VS2008_OR_VS2010}/$(PlatformName)/Release/libvorbis_static.lib)
        target_link_libraries(${TARGET_NAME} optimized ${ENV_TUNDRA_DEP_PATH}/vorbis/win32/${VS2008_OR_VS2010}/$(PlatformName)/Release/libvorbisfile_static.lib)
        target_link_libraries(${TARGET_NAME} debug ${ENV_TUNDRA_DEP_PATH}/vorbis/win32/${VS2008_OR_VS2010}/$(PlatformName)/Debug/libvorbis_static.lib)
        target_link_libraries(${TARGET_NAME} debug ${ENV_TUNDRA_DEP_PATH}/vorbis/win32/${VS2008_OR_VS2010}/$(PlatformName)/Debug/libvorbisfile_static.lib)
    else()
        target_link_libraries(${TARGET_NAME} general vorbis general vorbisfile)
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
