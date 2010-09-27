# Set project name
project (naali-viewer)

# CMake version requirement
cmake_minimum_required (VERSION 2.4)

# The AddEntityComponent macro adds the static EC lib from the given sourceFolder directory to the build.
# The componentName must be the name of the EC that will be added. E.g. AddEntityComponent(EntityComponents/EC_Highlight EC_Highlight)
# Use this macro when adding ECs to the build, since this macro tracks the set of ECs that are added, so that the individual Naali
# modules get #defines in C++ code for each EC that is present. This allows conditionally excluding ECs from the build.
macro(AddEntityComponent sourceFolder componentName)
   add_subdirectory (${sourceFolder})
   set (${componentName}_ENABLED 1)
   set (USED_ENTITYCOMPONENTS ${USED_ENTITYCOMPONENTS} ${componentName})
endmacro()

# Generates in the current project a #define for each EC that has been added to the build. Use this macro in your module CMakeLists.txt
# to receive information in C++ about which ECs have been added to the project. This allows supporting conditional compilation of the ECs in your module.
macro(GetEntityComponents)
   foreach(componentName ${USED_ENTITYCOMPONENTS})
      add_definitions(-D${componentName}_ENABLED)
   endforeach()
endmacro()

# Links the current project to the given EC, if that EC has been added to the build. Otherwise omits the EC.
macro(LinkEntityComponent sourceFolder componentName)
   if (${componentName}_ENABLED)
      use_modules(${sourceFolder})
      link_modules(${componentName})
      add_definitions(-D${componentName}_ENABLED)
   endif()
endmacro()

# Adds the given folder_name into the source files of the current project. Use this macro when your module contains .cpp and .h files in several subdirectories.
macro (AddSourceFolder folder_name)
    file (GLOB H_FILES_IN_FOLDER_${folder_name} ${folder_name}/*.h)
    file (GLOB CPP_FILES_IN_FOLDER_${folder_name} ${folder_name}/*.cpp)
    source_group ("Header Files\\${folder_name}" FILES ${H_FILES_IN_FOLDER_${folder_name}})
    source_group ("Source Files\\${folder_name}" FILES ${CPP_FILES_IN_FOLDER_${folder_name}})
    set (H_FILES ${H_FILES} ${H_FILES_IN_FOLDER_${folder_name}})
    set (CPP_FILES ${CPP_FILES} ${CPP_FILES_IN_FOLDER_${folder_name}})
endmacro (AddSourceFolder)

# Moves all Qt moc-compiler generated files into their own folder in the Visual Studio project. Call this once after having added all source files in the build in your module,
# but before the call to 'set (SOURCE_FILES ...)' directive.
# This macro is optional and for "conveniency" only. If you omit this call, you will have all the generated moc files in the Visual Studio project root.
# TODO: Add support for more deep folder structures. Current
#       implementation only support folders directly under project folder
macro (MocFolder)
    file (GLOB MOCS_TO_SOURCE_GROUP *.cxx */*.cxx)
    source_group ("CMake Moc" FILES ${MOCS_TO_SOURCE_GROUP})
endmacro (MocFolder)

# Moves all Qt ui copmiler-generated .h files into their own folder in the Visual Studio project. Call this once after having added all source files in the build in your module.
# This macro is optional and for "conveniency" only. If you omit this call, you will have all the generated moc files in the Visual Studio project root.
macro (UiFolder)
    file (GLOB GENERATED_UI_H ui_*.h)
    source_group ("Generated UI" FILES ${GENERATED_UI_H})
endmacro (UiFolder)

# Set CMake library search policy
if (COMMAND cmake_policy)
    cmake_policy (SET CMP0003 NEW)
    cmake_policy (SET CMP0005 NEW)
endif (COMMAND cmake_policy)

# Turn on warnings
if (MSVC)
    add_definitions (/W2)
else ()
    add_definitions (-W)
endif()

# Set CMake custom module path & include them
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} CMakeModules)

# Set Kristalli path
set(KRISTALLI_CORE_PATH ${PROJECT_BINARY_DIR}/Kristalli)

# The following CMake modules are required for the project to build.
include (Sagase)
include (ModuleSetup)
include (ConfigurePackages)

# Disable unnecessary build types
set (CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo;Debug" CACHE STRING "Configurations" FORCE)

if (MSVC)
  # Use Caelum, but not Hydrax for now.
  SET (CAELUM 1)
  SET (HYDRAX 0)
else ()
  SET (CAELUM 0)
  SET (HYDRAX 0)
endif()


# Possibility to disable console output and make viewer a true Windows application (Windows only)
SET (WINDOWS_APP 0)

# Set login scene type: 1 = dynamic switchable ether + normal login, 0 = static classical login
SET (DYNAMIC_LOGIN_SCENE 1)

# In Visual Studio, use unicode character set
if (MSVC)
    add_definitions (-DUNICODE -D_UNICODE)
endif (MSVC)

# Generate profiling data.
add_definitions (-DPROFILING)

# Enable memory leak checking in all core modules.
if (MSVC)
    add_definitions (-DMEMORY_LEAK_CHECK)
endif (MSVC)

# On all platforms, we enable PCH files by default.
add_definitions (-DPCH_ENABLED)
SET (PCH_ENABLED 1)

# Set normalized path for common environment variables
file (TO_CMAKE_PATH "$ENV{NAALI_DEP_PATH}" ENV_NAALI_DEP_PATH)
file (TO_CMAKE_PATH "$ENV{QTDIR}" ENV_QT_DIR)
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
configure_qtpropertybrowser ()

if (CAELUM)
    configure_caelum ()
endif (CAELUM)

if (HYDRAX)
    configure_hydrax ()
endif (HYDRAX)

# UiServiceModule is an experimental light-weight UI module 
# which can be used as a substitute for UiModule.
# If you want to test UiServiceModule, uncomment the following line.
# The define also enables LoginScreenModule which is an experimental module 
# for abstracting the login screen / main menu from the main application:
# currently this functionality resides in RexLogicModule and UiModule.
SET (UISERVICE_TEST 1)

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

# The AddEntityComponent macros must appear before any of the add_subdirectory() directives for modules that use the ECs, or otherwise
# they will not be able to conditionally compile the EC.

# The following EC's are declared by RexLogicModule
AddEntityComponent(EntityComponents/EC_OpenSimPrim EC_OpenSimPrim)         # Required by ProtocolUtilities and RexLogicModule.
AddEntityComponent(EntityComponents/EC_OpenSimPresence EC_OpenSimPresence)

AddEntityComponent(EntityComponents/EC_Highlight EC_Highlight)
AddEntityComponent(EntityComponents/EC_InputMapper EC_InputMapper)
AddEntityComponent(EntityComponents/EC_Movable EC_Movable)
AddEntityComponent(EntityComponents/EC_Mesh EC_Mesh)
AddEntityComponent(EntityComponents/EC_Sound EC_Sound)
AddEntityComponent(EntityComponents/EC_ParticleSystem EC_ParticleSystem)
AddEntityComponent(EntityComponents/EC_3DCanvas EC_3DCanvas)
AddEntityComponent(EntityComponents/EC_3DCanvasSource EC_3DCanvasSource)
AddEntityComponent(EntityComponents/EC_HoveringText EC_HoveringText)
AddEntityComponent(EntityComponents/EC_Clone EC_Clone)
AddEntityComponent(EntityComponents/EC_Light EC_Light)
AddEntityComponent(EntityComponents/EC_Touchable EC_Touchable)
AddEntityComponent(EntityComponents/EC_Ruler EC_Ruler)
AddEntityComponent(EntityComponents/EC_SoundRuler EC_SoundRuler)
AddEntityComponent(EntityComponents/EC_SoundListener EC_SoundListener)

# The following EC's are declared by AvatarModule
AddEntityComponent(EntityComponents/EC_NetworkPosition EC_NetworkPosition) # Required.
AddEntityComponent(EntityComponents/EC_HoveringWidget EC_HoveringWidget)

# Declared by PythonScriptModule. 
AddEntityComponent(EntityComponents/EC_DynamicComponent EC_DynamicComponent)      # Used by ECEditorModule.
AddEntityComponent(EntityComponents/EC_Script EC_Script)

add_subdirectory (RexLogicModule)
add_subdirectory (SupportModules)
add_subdirectory (AvatarModule)
add_subdirectory (AssetModule)
add_subdirectory (HttpUtilities)
add_subdirectory (RpcUtilities)
add_subdirectory (ProtocolUtilities)
add_subdirectory (EnvironmentModule)
if (NOT UISERVICE_TEST)
    add_subdirectory (UiModule)
endif ()

# Tundra protocol, logic & executables
add_subdirectory (KristalliProtocolModule)
add_subdirectory (TundraLogicModule)
add_subdirectory (Viewer)
add_subdirectory (Server)

# The following are not really core modules,
# but you can't do anything practical without
# having them so, they're effectively required.
add_subdirectory (QtInputModule)
add_subdirectory (TextureDecoderModule)
add_subdirectory (ProtocolModuleOpenSim)
add_subdirectory (ProtocolModuleTaiga)

if (UISERVICE_TEST)
    add_subdirectory (UiServiceModule)
    add_subdirectory (LoginScreenModule)
endif ()

# The following are optional, but contain 
# commonly used functionality.
add_subdirectory (InventoryModule)
add_subdirectory (OgreAssetEditorModule)
add_subdirectory (CommunicationsModule)

# Entity components. Note: at the current system if you disable one of these you must
# comment out the declaration at the responsible module's Load() function.

# Add EC's declared by other modules here:
# ...

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

if ( UPDATE_LANGUAGE_TRANSLATIONS )
   MESSAGE("Update .ts files with new translation data.")
   update_translation_files(TRANSLATIONS_FILES)
endif()

update_qm_files(TRANSLATIONS_FILES)
MESSAGE("Language translations generated")
