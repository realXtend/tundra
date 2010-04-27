# Set project name
project (naali-viewer)

# CMake version requirement
cmake_minimum_required (VERSION 2.4)

# Set CMake library search policy
if (COMMAND cmake_policy)
    cmake_policy (SET CMP0003 NEW)
    cmake_policy (SET CMP0005 NEW)
endif (COMMAND cmake_policy)

# Set CMake custom module path & include them
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} CMakeModules)

# The following CMake modules are required for the project to build.
include (Sagase)
include (ModuleSetup)
include (ConfigurePackages)

# Disable unnecessary build types
set (CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo;Debug" CACHE STRING "Configurations" FORCE)

# Use Caelum, but not Hydrax for now.
SET (CAELUM 1)
SET (HYDRAX 0)

# Possibility to disable console output and make viewer a true Windows application (Windows only)
SET (WINDOWS_APP 0)

# Set login scene type: 1 = dynamic switchable ether + normal login, 0 = static classical login
SET (DYNAMIC_LOGIN_SCENE 1)

# In Visual Studio, use unicode character set
if (MSVC)
    add_definitions (-DUNICODE -D_UNICODE)
endif (MSVC)

# Generate profiling data.
if (MSVC)
   add_definitions (-DPROFILING)
endif (MSVC)

# Enable memory leak checking in all core modules.
if (MSVC)
    add_definitions (-DMEMORY_LEAK_CHECK)
endif (MSVC)

# On all platforms, we enable PCH files by default.
add_definitions (-DPCH_ENABLED)
SET (PCH_ENABLED 1)

# Set normalized path for common environment variables
file (TO_CMAKE_PATH "$ENV{NAALI_DEP_PATH}" ENV_NAALI_DEP_PATH)
file (TO_CMAKE_PATH "$ENV{QT_DIR}" ENV_QT_DIR)
file (TO_CMAKE_PATH "$ENV{OGRE_HOME}" ENV_OGRE_HOME)

# Find needed external libraries, abort on error
configure_boost ()
configure_poco ()
configure_qt4 ()
configure_python ()
configure_python_qt ()
configure_ogre ()
configure_xmlrpc ()
configure_curl ()
configure_openjpeg ()
configure_propertyeditor ()
configure_qtpropertybrowser ()

if (CAELUM)
    configure_caelum ()
endif (CAELUM)

if (HYDRAX)
    configure_hydrax ()
endif (HYDRAX)

# Add all the subprojects in the rex viewer.

# The following are the core modules that are 
# required for the project to build. None of
# these can't be omitted.
add_subdirectory (Core)
add_subdirectory (Foundation)
add_subdirectory (Interfaces)
add_subdirectory (RexCommon)
add_subdirectory (SceneManager)
add_subdirectory (OgreRenderingModule)
add_subdirectory (Application)
add_subdirectory (RexLogicModule)
add_subdirectory (SupportModules)
add_subdirectory (AssetModule)
add_subdirectory (UiModule)
add_subdirectory (HttpUtilities)
add_subdirectory (RpcUtilities)
add_subdirectory (ProtocolUtilities)
add_subdirectory (EnvironmentModule)

# The following are not really core modules,
# but you can't do anything practical without
# having them so, they're effectively required.
add_subdirectory (WorldInputModule)
add_subdirectory (TextureDecoderModule)
add_subdirectory (ProtocolModuleOpenSim)
add_subdirectory (ProtocolModuleTaiga)

# The following are optional, but contain 
# commonly used functionality.
add_subdirectory (InventoryModule)
add_subdirectory (OgreAssetEditorModule)

# Entity components. Note: at the current system if you disable one of these you must
# comment out the declaration at the responsible module's Load() function.
add_subdirectory (EntityComponents/EC_Highlight)        # declared by RexLogicModule
add_subdirectory (EntityComponents/EC_HoveringText)     # declared by RexLogicModule
add_subdirectory (EntityComponents/EC_Clone)            # declared by RexLogicModule
add_subdirectory (EntityComponents/EC_Light)            # declared by RexLogicModule
add_subdirectory (EntityComponents/EC_OpenSimPresence)  # declared by RexLogicModule
add_subdirectory (EntityComponents/EC_OpenSimPrim)  # declared by RexLogicModule

# If the custom optional modules configuration file does not yet
# exist, create it from a template.
if (NOT EXISTS CMakeOptionalModules.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CMakeOptionalModulesTemplate.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/CMakeOptionalModules.txt)
endif ()

# Read the set of optional modules from another file
# that is kept outside the source control. 
# To configure the set of optional modules to add to the build,
# edit CMakeOptionalModules.txt and rerun cmake.
include (CMakeOptionalModules.txt)

# Collect translation files.
file (GLOB TRANSLATIONS_FILES bin/data/translations/*.ts)

# Collect ui-files which are stored into data/
file (GLOB ui_src bin/data/ui/*.ui)

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${ui_src})
update_translation_files(TRANSLATIONS_FILES)
update_qm_files(TRANSLATIONS_FILES)
MESSAGE("Language translations generated")