# Android: build cpu-features static library which Ogre depends on
# Todo: do in a cleaner way. This library is in turn referred by FindOgre.cmake
if (ANDROID)
    add_library (cpu-features STATIC ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)
endif ()

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

# Build parameters required by Qt - define files for moc prepass, process resource and ui files.
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
#QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

set(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

include_directories (libcrunch)
include_directories (libsquish)

add_definitions(-DOGRE_MODULE_EXPORTS)

use_package_assimp()
UseTundraCore()
use_core_modules(TundraCore Math)

build_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

link_package(QT4)
link_ogre()
link_modules(TundraCore Math)

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
setup_install_windows_deps("OgreOverlay.dll" Release|RelWithDebInfo "OgreOverlay_d.dll" Debug)

if (TUNDRA_NO_BOOST)
    setup_install_windows_deps("tbb.dll" Release|RelWithDebInfo "tbb_debug.dll" Debug)
endif ()

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
