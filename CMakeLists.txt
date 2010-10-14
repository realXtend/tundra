# Set project name
project(naali-viewer)

# CMake version requirement
cmake_minimum_required(VERSION 2.4)

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
macro(AddSourceFolder folder_name)
    file (GLOB H_FILES_IN_FOLDER_${folder_name} ${folder_name}/*.h)
    file (GLOB CPP_FILES_IN_FOLDER_${folder_name} ${folder_name}/*.cpp)
    source_group ("Header Files\\${folder_name}" FILES ${H_FILES_IN_FOLDER_${folder_name}})
    source_group ("Source Files\\${folder_name}" FILES ${CPP_FILES_IN_FOLDER_${folder_name}})
    set (H_FILES ${H_FILES} ${H_FILES_IN_FOLDER_${folder_name}})
    set (CPP_FILES ${CPP_FILES} ${CPP_FILES_IN_FOLDER_${folder_name}})
endmacro()

# Moves all Qt moc-compiler generated files into their own folder in the Visual Studio project. Call this once after having added all source files in the build in your module,
# but before the call to 'set (SOURCE_FILES ...)' directive.
# This macro is optional and for "conveniency" only. If you omit this call, you will have all the generated moc files in the Visual Studio project root.
# TODO: Add support for more deep folder structures. Current
#       implementation only support folders directly under project folder
macro(MocFolder)
    file (GLOB MOCS_TO_SOURCE_GROUP *.cxx */*.cxx)
    source_group ("CMake Moc" FILES ${MOCS_TO_SOURCE_GROUP})
endmacro()

# Enables the use of Precompiled Headers in the project this macro is invoked in. Also adds the DEBUG_CPP_NAME to each .cpp file that specifies the name of that compilation unit. MSVC only.
macro(SetupCompileFlagsWithPCH)
    if (MSVC)
        # Label StableHeaders.cpp to create the PCH file and mark all other .cpp files to use that PCH file.
        # Add a #define DEBUG_CPP_NAME "this compilation unit name" to each compilation unit to aid in memory leak checking.
        foreach(src_file ${CPP_FILES})
            if (${src_file} MATCHES "StableHeaders.cpp$")
                set_source_files_properties(${src_file} PROPERTIES COMPILE_FLAGS "/YcStableHeaders.h")        
            else()
                get_filename_component(basename ${src_file} NAME)
                set_source_files_properties(${src_file} PROPERTIES COMPILE_FLAGS "/YuStableHeaders.h -DDEBUG_CPP_NAME=\"\\\"${basename}\"\\\"")
            endif()
        endforeach(src_file ${CPP_FILES})
    endif (MSVC)
endmacro()

# Sets up the compilation flags without PCH. For now just set the DEBUG_CPP_NAME to each compilation unit.
# TODO: The SetupCompileFlags and SetupCompileFlagsWithPCH macros should be merged, and the option to use PCH be passed in as a param. However,
# CMake string ops in PROPERTIES COMPILE_FLAGS gave some problems with this, so these are separate for now.
macro(SetupCompileFlags)
    if (MSVC)
        # Add a #define DEBUG_CPP_NAME "this compilation unit name" to each compilation unit to aid in memory leak checking.
        foreach(src_file ${CPP_FILES})
            if (${src_file} MATCHES "StableHeaders.cpp$")
            else()
                get_filename_component(basename ${src_file} NAME)
                set_source_files_properties(${src_file} PROPERTIES COMPILE_FLAGS "-DDEBUG_CPP_NAME=\"\\\"${basename}\"\\\"")
            endif()
        endforeach(src_file ${CPP_FILES})
    endif (MSVC)
endmacro()

# Adds a compilation step that copies the module entry & dependency XML file to the runtime /bin/modules/core directory.
# Before calling this, define the XML_FILES list in your module.
macro(CopyModuleXMLFile)
    foreach(XML_FILE ${XML_FILES})
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
    endforeach(XML_FILE)
endmacro()

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
    add_definitions (-Wall -Wextra -Wno-unknown-pragmas -Wno-unused -Wno-reorder -Wno-switch -Wno-write-strings)
endif()

# Set CMake custom module path & include them
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} CMakeModules)

# The following CMake modules are required for the project to build.
include (Sagase)
include (ModuleSetup)
include (ConfigurePackages)

# Disable unnecessary build types
set (CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo;Debug" CACHE STRING "Configurations" FORCE)

# In Visual Studio, use unicode character set
if (MSVC)
    add_definitions (-DUNICODE -D_UNICODE)
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

# If the custom optional modules configuration file does not yet
# exist, create it from the template file.
if (NOT EXISTS CMakeBuildConfig.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CMakeBuildConfigTemplate.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/CMakeBuildConfig.txt)
endif ()

# Read the set of optional modules from another file
# that is kept outside the source control. 
# To configure the set of optional modules to add to the build,
# edit CMakeBuildConfig.txt and rerun cmake.
include (CMakeBuildConfig.txt)

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
