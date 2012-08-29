macro(use_package_hydrax)
    # TODO inclusion chains for using Hydrax from deps for other platforms.
    
    # Hydrax lookup rules:
    # 1. If the environment variable HYDRAX_HOME is set, use that directory and cache it.
    # 2. Otherwise, use the deps directory path.

    # TODO Linux/mac still uses sagase. Try to get rid of it and unify the below.
    if (MSVC)
        message("** Configuring Hydrax")
        if ("${HYDRAX_HOME}" STREQUAL "")
            if (NOT "$ENV{HYDRAX_HOME}" STREQUAL "")
                file (TO_CMAKE_PATH "$ENV{HYDRAX_HOME}" HYDRAX_HOME)
                # Cache HYDRAX_HOME for runs that dont define $ENV{HYDRAX_HOME}.
                set (HYDRAX_HOME ${HYDRAX_HOME} CACHE PATH "HYDRAX_HOME dependency path" FORCE)
            else ()
                message(STATUS "CMake cache or environment variable HYDRAX_HOME not set, assuming location to TUNDRA_DEP_PATH.")
                set(HYDRAX_HOME ${ENV_TUNDRA_DEP_PATH}/Hydrax)
            endif ()
        endif()
        message(STATUS "Using HYDRAX_HOME = ${HYDRAX_HOME}")
    endif ()
    
    if (MSVC) 
        include_directories(${HYDRAX_HOME}/include)
        link_directories(${HYDRAX_HOME}/lib)
    else()
        sagase_configure_package (HYDRAX
            NAMES Hydrax HYDRAX hydrax
            COMPONENTS Hydrax HYDRAX hydrax
            PREFIXES ${ENV_HYDRAX_HOME} ${ENV_TUNDRA_DEP_PATH})
        set (HYDRAX_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/include/Hydrax)
        include_directories(${HYDRAX_INCLUDE_DIRS})
        sagase_configure_report (HYDRAX)
    endif()
endmacro()

macro(link_package_hydrax)
    if (MSVC OR APPLE) 
        target_link_libraries(${TARGET_NAME} optimized Hydrax debug Hydraxd)
    else ()
        # Linux: assume sagase found the libs. See use_package_hydrax todos.
        target_link_libraries(${TARGET_NAME} ${HYDRAX_LIBRARIES})
    endif()
endmacro()


macro(use_package_skyx)
    # SkyX look up rules:
    # 1. Use cmake cached SKYX_DIR.
    # 2. Use env variable SKYX_DIR and cache it.
    # 3. Assume SkyX from deps path.

    message("** Configuring SkyX")
    if ("${SKYX_DIR}" STREQUAL "")
        file (TO_CMAKE_PATH "$ENV{SKYX_DIR}" SKYX_DIR)
        # Cache SKYX_DIR for runs that dont define $ENV{SKYX_DIR}.
        set (SKYX_DIR ${SKYX_DIR} CACHE PATH "SKYX_DIR dependency path" FORCE)
    endif ()
    if ("${SKYX_DIR}" STREQUAL "")
        if (MSVC)
            set(SKYX_DIR ${ENV_TUNDRA_DEP_PATH}/SkyX)
        else()
            set(SKYX_DIR ${ENV_TUNDRA_DEP_PATH})
        endif()
    endif()
    message (STATUS "Using SKYX_DIR = ${SKYX_DIR}")

    if (MSVC) 
        include_directories(${SKYX_DIR}/include) # For prebuilt VS2008/VS2010 deps.
        include_directories(${SKYX_DIR}/Include) # For full-built source deps.
        link_directories(${SKYX_DIR}/lib)
    else() # Linux and mac
        include_directories(${SKYX_DIR}/include/SkyX)
        link_directories(${SKYX_DIR}/lib)
    endif()
endmacro()

macro(link_package_skyx)
    target_link_libraries(${TARGET_NAME} optimized SkyX)
    if (MSVC) 
        target_link_libraries(${TARGET_NAME} debug SkyX_d)
    endif()
endmacro()

# Only do something if one of the components are enabled
if (ENABLE_SKYX OR ENABLE_HYDRAX)

# Define target name
init_target (SkyXHydrax OUTPUT plugins)

MocFolder ()

# Define source files
if (ENABLE_SKYX)
    file (GLOB CPP_FILES dllmain.cpp EC_SkyX.cpp)
    file (GLOB H_FILES EC_SkyX.h)
endif()
if (ENABLE_HYDRAX)
    file (GLOB CPP_FILES dllmain.cpp EC_Hydrax.cpp)
    file (GLOB H_FILES EC_Hydrax.h)
endif()
if (ENABLE_SKYX AND ENABLE_HYDRAX)
    file (GLOB CPP_FILES dllmain.cpp EC_SkyX.cpp EC_Hydrax.cpp)
    file (GLOB H_FILES EC_SkyX.h EC_Hydrax.h)
endif()

file (GLOB MOC_FILES ${H_FILES})
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

if (ENABLE_SKYX)
    add_definitions(-DSKYX_ENABLED)
#    use_package(SKYX) # Other OSes #TODO:REMOVE THIS!
    use_package_skyx() # Windows 
endif()
if (ENABLE_HYDRAX)
    add_definitions(-DHYDRAX_ENABLED)
    use_package(HYDRAX)  # Other OSes #TODO:REMOVE THIS!
    use_package_hydrax() # Windows
endif()

use_core_modules(Framework Math Scene OgreRenderingModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_modules(Framework Math Scene OgreRenderingModule)

link_ogre()

if (ENABLE_SKYX)
#    link_package (SKYX)  # Other OSes #TODO:REMOVE THIS!
    link_package_skyx() # Windows
endif()
if (ENABLE_HYDRAX)
    link_package(HYDRAX)  # Other OSes #TODO:REMOVE THIS!
    link_package_hydrax() # Windows
endif()

SetupCompileFlags()

final_target ()

# Install windows deps
if (ENABLE_SKYX)
    setup_install_windows_deps ("SkyX.dll" Release|RelWithDebInfo "SkyX_d.dll" Debug)
endif ()
if (ENABLE_HYDRAX)
    setup_install_windows_deps ("Hydrax.dll" Release|RelWithDebInfo "Hydraxd.dll" Debug)
endif ()

endif ()
