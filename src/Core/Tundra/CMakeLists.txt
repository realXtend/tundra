# Define target name and output directory
if (NOT ANDROID)
    init_target (Tundra OUTPUT ./)
else ()
    file(MAKE_DIRECTORY ${TUNDRA_BIN}/../android/libs)
    file(MAKE_DIRECTORY ${TUNDRA_BIN}/../android/libs/${ANDROID_ABI})
    init_target (Tundra OUTPUT ../android/libs/${ANDROID_ABI})
endif ()

# Define source files
file (GLOB CPP_FILES main.cpp)
file (GLOB H_FILES "") # Dont do *.h it will pickup windows spesific resource.h

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})
set (RESOURCE_FILES "")

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

if (MSVC)
    # Windows resource files for icon.
    if (BUILDING_INSTALLER)
        set(RESOURCE_FILES ${RESOURCE_FILES} resource.h Tundra.rc)
    endif()

    # Preprocessor defines on MSVC
    set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS_RELEASE _DMEMDUMP)
    set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS_RELWITHDEBINFO _DMEMDUMP)
endif()

SetupCompileFlags()

UseTundraCore()
use_core_modules(TundraCore Math)

# On Android build the main program as a shared object
if (ANDROID)
   # Nasty CMake hack: the linker directories needed by the static modules have to be redeclared here. \todo Better solution
   use_package_bullet()
   build_library(${TARGET_NAME} SHARED ${SOURCE_FILES})
elseif (MSVC)
   build_executable(${TARGET_NAME} WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})
else()
   build_executable(${TARGET_NAME} ${SOURCE_FILES} ${RESOURCE_FILES})
endif()

if (WIN32 AND CMAKE_CL_64)
    link_package_knet() # For kNet's BottomMemoryAllocator
endif()

link_modules(TundraCore)
# On Android all modules are linked as static.
if (ANDROID)
    link_modules(TundraProtocolModule OgreRenderingModule PhysicsModule EnvironmentModule AssetModule JavascriptModule AvatarModule DebugStatsModule)
endif()

final_target ()

# Install files
setup_install_files (${TUNDRA_BIN}/plugins/TundraInstallationDirectory.txt "plugins")   # C++ plugins
setup_install_files (${TUNDRA_BIN}/../README.md)      # Readme
setup_install_files (${TUNDRA_BIN}/../LICENSE)        # Licence
setup_install_files (${TUNDRA_BIN}/../WhatsNew.txt)   # Whats new
setup_install_files (${TUNDRA_BIN}/plugins.xml)       # Default startup config
setup_install_files (${TUNDRA_BIN}/viewer.xml)        # Viewer startup config
setup_install_files_find (${TUNDRA_BIN}/dox "*.txt" Release|Debug|RelWithDebInfo "dox") # Doc files used in installer

# Install certain things only in non BUILD_SDK_ONLY mode
if (NOT BUILD_SDK_ONLY)
    setup_install_directory (${TUNDRA_BIN}/data)        # Data: ui, assets, etc
    setup_install_directory (${TUNDRA_BIN}/scenes)      # Example application scenes
    setup_remove_file ("data/assets/dummy.txt")         # Remove git dummy file from data/assets
endif ()

# Install deps for windows
setup_install_windows_deps_regexp ("" qt/bin/ "[/][Q][t][A-Z].*[^d][4][.]dll$" Release|RelWithDebInfo qt/bin/ "[/][Q][t][A-Z].*[d][4][.]dll$" Debug)
setup_install_windows_deps_regexp (qtplugins qt/plugins/ "[/]plugins[/].*[/][q].*[^d][4][.]dll$" Release|RelWithDebInfo qt/plugins/ "[/]plugins[/].*[/][q].*[d][4][.]dll$" Debug)
setup_install_windows_deps ("libeay32.dll;ssleay32.dll" Release|RelWithDebInfo|Debug) # OpenSSL DLLs for SSL support in QNetwork
