macro(configure_crunch)
    # Hydrax lookup rules:
    # 1. If the environment variable CRUNCH_ROOT is set, use that directory and cache it.
    # 2. Otherwise, use the deps directory path.

    message("** Configuring Crunch")
    if ("${CRUNCH_ROOT}" STREQUAL "")
        if (NOT "$ENV{CRUNCH_ROOT}" STREQUAL "")
            file (TO_CMAKE_PATH "$ENV{CRUNCH_ROOT}" CRUNCH_ROOT)
            # Cache CRUNCH_ROOT for runs that dont define $ENV{CRUNCH_ROOT}.
            set (CRUNCH_ROOT ${CRUNCH_ROOT} CACHE PATH "CRUNCH_ROOT dependency path" FORCE)
        else ()
            message(STATUS "CMake cache or environment variable CRUNCH_ROOT not set, assuming location to TUNDRA_DEP_PATH.")
            set(CRUNCH_ROOT ${ENV_TUNDRA_DEP_PATH})
        endif ()
    endif()
    
    find_path(CRUNCH_INCLUDE_DIR crn_decomp.h 
        HINTS ${CRUNCH_ROOT}/include            # win
              ${CRUNCH_ROOT}/include/crunch)    # linux/osx
    
    if (MSVC)
        find_library(CRUNCH_RELEASE_LIBRARY crnlib_DLL_VC9
            HINTS ${CRUNCH_ROOT}/lib/VC9/Release_DLL/Win32)
        find_library(CRUNCH_DEBUG_LIBRARY crnlibD_DLL_VC9
            HINTS ${CRUNCH_ROOT}/lib/VC9/Debug_DLL/Win32)
    else ()
        find_library(CRUNCH_RELEASE_LIBRARY libcrnlib
            HINTS ${CRUNCH_ROOT}/lib)
    endif()
    
    message (STATUS "-- Include Directories:")
    message (STATUS "       " ${CRUNCH_INCLUDE_DIR})
    message (STATUS "-- Libraries:")
    message (STATUS "       " ${CRUNCH_RELEASE_LIBRARY})
    if (MSVC)
        message (STATUS "       " ${CRUNCH_DEBUG_LIBRARY})
    endif()
	message ("")
endmacro()

macro(use_package_crunch)
    include_directories(${CRUNCH_INCLUDE_DIR})
endmacro()

macro(link_package_crunch)
    if (MSVC) 
        target_link_libraries(${TARGET_NAME} optimized ${CRUNCH_RELEASE_LIBRARY} debug ${CRUNCH_DEBUG_LIBRARY})
    else ()
        target_link_libraries(${TARGET_NAME} ${CRUNCH_RELEASE_LIBRARY})
    endif()
endmacro()

configure_crunch ()

#################################################################################

# Define target name and output directory
init_target(OgreRenderingModule OUTPUT plugins)

# Define source files
file(GLOB LIBSQUISH_CPP_FILES libsquish/*.cpp)
file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB UI_FILES *.ui)
file(GLOB XML_FILES *.xml)
file(GLOB MOC_FILES RenderWindow.h EC_*.h Renderer.h TextureAsset.h OgreMeshAsset.h OgreParticleAsset.h
    OgreSkeletonAsset.h OgreMaterialAsset.h OgreRenderingModule.h OgreWorld.h UiPlane.h)
if (WIN32)
    set(SOURCE_FILES ${LIBSQUISH_CPP_FILES} ${CPP_FILES} ${H_FILES})
else()
    set(SOURCE_FILES ${CPP_FILES} ${H_FILES})
endif()

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder()
#UiFolder()

# Build parameters required by Qt - define files for moc prepass, process resource and ui files.
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
#QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

set(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

include_directories (libsquish)

add_definitions(-DOGRE_MODULE_EXPORTS)

use_package_assimp()
use_package_crunch()
use_core_modules(Framework Math Scene Ui Asset Console)

build_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

GetOpenAssetImport()

link_ogre()
link_package_crunch()
link_modules(Framework Scene Ui Asset Console)

SetupCompileFlagsWithPCH()

final_target()

# Install files
setup_install_directory(${TUNDRA_BIN}/media)

setup_install_files_find(${TUNDRA_BIN} "resources.cfg")
if (WIN32)
    setup_install_files_find(${TUNDRA_BIN} "plugins.cfg" Release|RelWithDebInfo)
    setup_install_files_find(${TUNDRA_BIN} "pluginsd.cfg" Debug)
elseif(UNIX)
    setup_install_files_find(${TUNDRA_BIN} "plugins-unix.cfg")
elseif(APPLE)
    setup_install_files_find(${TUNDRA_BIN} "plugins-mac.cfg")
endif()

# Install windows deps
setup_install_windows_deps("cg.dll" Release|RelWithDebInfo|Debug)
setup_install_windows_deps("OgreMain.dll" Release|RelWithDebInfo "OgreMain_d.dll" Debug)
if (IS_DIRECTORY ${ENV_TUNDRA_DEP_PATH}/ogre-safe-nocrashes/SDK/bin) # Full deps
    if (IS_DIRECTORY ${ENV_TUNDRA_DEP_PATH}/ogre-safe-nocrashes/SDK/bin/Debug)
        setup_install_windows_deps_regexp("" ogre-safe-nocrashes/SDK/bin/Debug/ "Plugin_+.*[_][d][.]dll$" Debug)
        setup_install_windows_deps_regexp("" ogre-safe-nocrashes/SDK/bin/Debug/ "RenderSystem_+.*[_][d][.]dll$" Debug)
    endif ()
    if (IS_DIRECTORY ${ENV_TUNDRA_DEP_PATH}/ogre-safe-nocrashes/SDK/bin/Release)
        setup_install_windows_deps_regexp("" ogre-safe-nocrashes/SDK/bin/Release/ "Plugin_+.*[^d][.]dll$" Release)
        setup_install_windows_deps_regexp("" ogre-safe-nocrashes/SDK/bin/Release/ "RenderSystem_+.*[^d][.]dll$" Release)
    endif ()
    if (IS_DIRECTORY ${ENV_TUNDRA_DEP_PATH}/ogre-safe-nocrashes/SDK/bin/RelWithDebInfo)
        setup_install_windows_deps_regexp("" ogre-safe-nocrashes/SDK/bin/RelWithDebInfo/ "Plugin_+.*[^d][.]dll$" RelWithDebInfo)
        setup_install_windows_deps_regexp("" ogre-safe-nocrashes/SDK/bin/RelWithDebInfo/ "RenderSystem_+.*[^d][.]dll$" RelWithDebInfo)
    endif ()
elseif (IS_DIRECTORY ${ENV_TUNDRA_DEP_PATH}/ogre-safe-nocrashes/bin) # Prebuilt from full deps
    setup_install_windows_deps_regexp("" ogre-safe-nocrashes/bin/ "Plugin_+.*[^d][.]dll$" Release|RelWithDebInfo ogre-safe-nocrashes/bin/ "Plugin_+.*[_][d][.]dll$" Debug)
    setup_install_windows_deps_regexp("" ogre-safe-nocrashes/bin/ "RenderSystem_+.*[^d][.]dll$" Release|RelWithDebInfo ogre-safe-nocrashes/bin/ "RenderSystem_+.*[_][d][.]dll$" Debug)
endif ()
