# Set project name
project(tundra)

# CMake version requirement, >= 2.5.5 to ensure that ProtoBuf is found ok.
cmake_minimum_required(VERSION 2.6.0)

# Policy CMP0005 - Preprocessor definition values are now escaped automatically.
if (COMMAND cmake_policy)
    cmake_policy(SET CMP0005 NEW)
endif()

# If the following are undefined, the default values in TundraVersionInfo.h are used.
add_definitions(-DTUNDRA_ORGANIZATION_NAME="Meshmoon")
add_definitions(-DTUNDRA_APPLICATION_NAME="Rocket")
add_definitions(-DTUNDRA_VERSION_STRING="2.5.1.2")
add_definitions(-DTUNDRA_VERSION_MAJOR=2)
add_definitions(-DTUNDRA_VERSION_MINOR=5)
add_definitions(-DTUNDRA_VERSION_MAJOR_PATCH=1)
add_definitions(-DTUNDRA_VERSION_MINOR_PATCH=2)

# TUNDRA_NO_BOOST defines whether we want to replace usage of Boost with the C++ (TR1) standard equivalents.
# Setting TUNDRA_NO_BOOST to TRUE, i.e. disabling usage of Boost in the Tundra codebase, requires compiler
# with up-to-date C++ TR1 and preferably the C99 standard cstdint header. Also if the used compiler has C++11 
# make_shared function available, it is used.
# If TUNDRA_NO_BOOST is set to TRUE, kNet must also be built without Boost dependecy. In order to make the
# Tundra build fully free of Boost, you must also build Ogre and AssImp, which depend on Boost by default,
# with Boost disabled. See the dependencies scripts for instructions how to control this.
# On Linux TUNDRA_NO_BOOST can not yet be used
if (WIN32 OR APPLE)
    option(TUNDRA_NO_BOOST "Specifies whether Tundra is build without Boost." TRUE)
else ()
    option(TUNDRA_NO_BOOST "Specifies whether Tundra is build without Boost." FALSE)
endif ()

option(TUNDRA_CPP11_ENABLED "Enables C++11 features on Tundra" FALSE)

if (APPLE)
    set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
    set(CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET "10.6")
endif()

if (APPLE AND TUNDRA_CPP11_ENABLED) # todo: Test if this works on Linux too
# Compiler-specific C++11 activation.
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
        if (NOT (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8))
            message(FATAL_ERROR "${PROJECT_NAME} C++11 features require GCC 4.8 or greater.")
        else ()
            message(STATUS "C++11 features enabled for GCC")
        endif ()
    elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE CLANG_VERSION)
        if (NOT (CLANG_VERSION VERSION_GREATER 4.1 OR CLANG_VERSION VERSION_EQUAL 4.1))
            message(FATAL_ERROR "${PROJECT_NAME} C++11 features require clang 4.1 or greater.")
        else ()
            message(STATUS "C++11 features enabled for Clang")
        endif ()
    else ()
        message(FATAL_ERROR "Your C++ compiler does not support C++11!")
    endif ()

    if ("${CMAKE_GENERATOR}" MATCHES "Xcode")
        set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x")
        set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
    endif()
elseif (MSVC AND TUNDRA_CPP11_ENABLED)
    # MSVC has partial C++11 support beginning in VC10
    if (MSVC90)
        message(FATAL_ERROR "VC9 does not support C++11 features!")
    endif()
endif()

macro(PrintUsedEnvVariables)
    # Print out the resolved variables. Note that this does not print the env variables
    # in most cases as they should be resolved to our cmake variables and get cached.
    # This way this printed list wont lie if ran without env variables and we are still
    # succesfully finding all the deps from our cacher (or in bad cases assumed) dirs.
    message("\n=========== Used Environment Variables ===========\n")
    message(STATUS "TUNDRA_DEP_PATH  = " ${ENV_TUNDRA_DEP_PATH})
    message(STATUS "QTDIR            = " ${ENV_QT_DIR})
    if (NOT TUNDRA_NO_BOOST)
        message (STATUS "BOOST_ROOT       = " ${BOOST_ROOT})
    endif()
    message(STATUS "OGRE_HOME        = " ${OGRE_DIR}) # OGRE_DIR is our internal OGRE_HOME
    message(STATUS "SKYX_HOME        = " ${SKYX_HOME})
    message(STATUS "HYDRAX_HOME      = " ${HYDRAX_HOME})
    message(STATUS "KNET_DIR         = " ${KNET_DIR})
    message(STATUS "BULLET_DIR       = " ${BULLET_DIR})
endmacro()

macro(PrintUsedBuildConfig)
    message("\n=========== Used Build Configuration =============\n")
    message(STATUS "TUNDRA_NO_BOOST            = " ${TUNDRA_NO_BOOST})
    message(STATUS "TUNDRA_CPP11_ENABLED       = " ${TUNDRA_CPP11_ENABLED})
    message(STATUS "TUNDRACORE_SHARED          = " ${TUNDRACORE_SHARED})
    message(STATUS "BUILD_SDK_ONLY             = " ${BUILD_SDK_ONLY})
    message(STATUS "INSTALL_BINARIES_ONLY      = " ${INSTALL_BINARIES_ONLY})
    message(STATUS "INSTALL_EXAMPLE_SCENES     = " ${INSTALL_EXAMPLE_SCENES})    
    message(STATUS "ENABLE_BUILD_OPTIMIZATIONS = " ${ENABLE_BUILD_OPTIMIZATIONS})
    message(STATUS "ENABLE_HYDRAX              = " ${ENABLE_HYDRAX})
    message(STATUS "ENABLE_SKYX                = " ${ENABLE_SKYX})
    message(STATUS "ENABLE_OPEN_ASSET_IMPORT   = " ${ENABLE_OPEN_ASSET_IMPORT})
    message(STATUS "ENABLE_PROFILING           = " ${ENABLE_PROFILING})
    message(STATUS "ENABLE_JS_PROFILING        = " ${ENABLE_JS_PROFILING})
    message(STATUS "ENABLE_MEMORY_LEAK_CHECKS  = " ${ENABLE_MEMORY_LEAK_CHECKS})
    message("")
    message(STATUS "Install prefix = " ${CMAKE_INSTALL_PREFIX})
    message("")
endmacro()

macro(GetLastElementFromPath dirPath outputParam)
    string (FIND ${dirPath} "/" lastPathSepIndex REVERSE)
    # This path does not have any subdirs in it
    if (${lastPathSepIndex} STREQUAL "-1")
        set (${outputParam} ${dirPath})
    # Path has subdirs, pick the last
    else ()
        math(EXPR lastPathSepIndex "${lastPathSepIndex} + 1")
        string(SUBSTRING ${dirPath} ${lastPathSepIndex} -1 ${outputParam})
    endif ()
endmacro ()

macro(RemoveLastElementFromPath dirPath outputParam)
    string (FIND ${dirPath} "/" lastPathSepIndex REVERSE)
    # This path does not have any subdirs in it
    if (${lastPathSepIndex} STREQUAL "-1")
        set (${outputParam} ${dirPath})
    # Path has subdirs, pick the path in front of it
    else ()
        string(SUBSTRING ${dirPath} 0 ${lastPathSepIndex} ${outputParam})
    endif ()
endmacro ()

# The AddEntityComponent macro adds the static EC lib from the given sourceFolder directory to the build.
# The componentName == A subfolder in src/EntityComponents/ or a relative path to the EC directory where the CMakeLists.txt is located.
# Use this macro when adding ECs to the build, since this macro tracks the set of ECs that are added, so that the individual
# modules get #defines in C++ code for each EC that is present. This allows conditionally excluding ECs from the build.
# Note that for eg. AddEntityComponent(3rdparty/myecs/EC_Thingie) the define will be EC_Thingie_ENABLED and you would expect.
# Examples:     AddEntityComponent(EC_Sound)
#               AddEntityComponent(3rdparty/myecs/EC_Thingie)
macro(AddEntityComponent componentName)
    set (PROJECT_TYPE "EntityComponents")
    # Chekc if the component is in our default entity components location
    # note: from cmake docs: IS_DIRECTORY Behavior is well-defined only for full paths.
    if (IS_DIRECTORY ${PROJECT_BINARY_DIR}/src/EntityComponents/${componentName})
        add_subdirectory (src/EntityComponents/${componentName})
        set (${componentName}_ENABLED 1)
        set (USED_ENTITYCOMPONENTS ${USED_ENTITYCOMPONENTS} ${componentName})
    elseif (IS_DIRECTORY ${PROJECT_BINARY_DIR}/${componentName})
        add_subdirectory (${componentName})
        GetLastElementFromPath(${componentName} compNameFromRelPath)
        set(${compNameFromRelPath}_ENABLED 1)
        set(USED_ENTITYCOMPONENTS ${USED_ENTITYCOMPONENTS} ${compNameFromRelPath})
    else ()
        message(FATAL_ERROR "Could not resolve AddEntityComponent() call with path " ${componentName} ". Are you sure the component is there?")
    endif ()
    set (PROJECT_TYPE "")
endmacro()

# AddProject takes in one or two parameters: 
# - One param  : argv0 == the relative directory (to the root CMakeLists.txt) where you project is and that contains the CMakeLists.txt for this project.
# - Two params : argv0 == 'Core'|'Application'. argv1 folder name of the project in the src/argv0/
# More useful for building the Tundra project when including platform provided projects.
# Note: One param projects or two param "Application" type plugins will always be omitted if BUILD_SDK_ONLY is true!
# Note: Due to the cmake add_subdirectory restrictions, the directory cannot be outside the Tundra source tree, hopefully we can go around this one day.
# Examples:     AddProject(Core Input)
#               AddProject(Application JavascriptModule)
#               AddProject(mysubdir/MyPlugin)
function(AddProject)
    set (PROJECT_TYPE ${ARGV0})
    if (NOT ARGV1)
        if (BUILD_SDK_ONLY)
            message (STATUS "** Omitting Application level project " ${ARGV0} " because BUILD_SDK_ONLY is set to TRUE")
        else ()
            add_subdirectory(${ARGV0})
        endif ()
    else ()
        if (NOT ${ARGV0} STREQUAL "Core" AND NOT ${ARGV0} STREQUAL "Application")
            message (FATAL_ERROR "You are trying to use AddProject macro with invalid project type parameter: " ${ARGV0} " with project name " ${ARGV1})
        endif()
        if (BUILD_SDK_ONLY AND ${ARGV0} STREQUAL "Application")
            message (STATUS "** Omitting Application level project " ${ARGV1} " because BUILD_SDK_ONLY is set to TRUE")
        else ()
            add_subdirectory(src/${ARGV0}/${ARGV1})
        endif ()
    endif ()
    set (PROJECT_TYPE "")
endfunction()

# Generates in the current project a #define for each EC that has been added to the build. Use this macro in your module CMakeLists.txt
# to receive information in C++ about which ECs have been added to the project. This allows supporting conditional compilation of the ECs in your module.
macro(GetEntityComponents)
   foreach(componentName ${USED_ENTITYCOMPONENTS})
      add_definitions(-D${componentName}_ENABLED)
   endforeach()
endmacro()

# Adds the given folder_name into the source files of the current project. Use this macro when your module contains .cpp and .h files in several subdirectories.
macro(AddSourceFolder folder_name)
    file(GLOB H_FILES_IN_FOLDER_${folder_name} ${folder_name}/*.h ${folder_name}/*.inl)
    file(GLOB CPP_FILES_IN_FOLDER_${folder_name} ${folder_name}/*.cpp)
    source_group("Header Files\\${folder_name}" FILES ${H_FILES_IN_FOLDER_${folder_name}})
    source_group("Source Files\\${folder_name}" FILES ${CPP_FILES_IN_FOLDER_${folder_name}})
    set(H_FILES ${H_FILES} ${H_FILES_IN_FOLDER_${folder_name}})
    set(CPP_FILES ${CPP_FILES} ${CPP_FILES_IN_FOLDER_${folder_name}})
endmacro()

# Moves all Qt moc-compiler generated files into their own folder in the Visual Studio project. Call this once after having added all source files in the build in your module,
# but before the call to 'set (SOURCE_FILES ...)' directive.
# This macro is optional and for "conveniency" only. If you omit this call, you will have all the generated moc files in the Visual Studio project root.
# TODO: Add support for more deep folder structures. Current
#       implementation only support folders directly under project folder
macro(MocFolder)
    file(GLOB MOCS_TO_SOURCE_GROUP *.cxx */*.cxx)
    source_group("CMake Moc" FILES ${MOCS_TO_SOURCE_GROUP})
endmacro()

# Moves all Qt ui copmiler-generated .h files into their own folder in the Visual Studio project. Call this once after having added all source files in the build in your module.
# This macro is optional and for "conveniency" only. If you omit this call, you will have all the generated moc files in the Visual Studio project root.
macro(UiFolder)
    file(GLOB GENERATED_UI_H ui_*.h)
    source_group("Generated UI" FILES ${GENERATED_UI_H})
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
        endforeach()
    endif()
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
        endforeach()
    endif()
endmacro()

# Set CMake library search policy
if (COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
    cmake_policy(SET CMP0005 NEW)
endif()

if (APPLE)
    EXEC_PROGRAM(/usr/bin/sw_vers ARGS -productVersion OUTPUT_VARIABLE OSX_MINOR_VERSION_RAW)
    STRING(REGEX REPLACE "10\\.([0-9]).*" "\\1" MACOSX_VERSION "${OSX_MINOR_VERSION_RAW}")
    if(${MACOSX_VERSION} LESS 6)
        message(FATAL_ERROR "Unsupported version of OS X: ${OSX_MINOR_VERSION_RAW}")
        return()
    elseif (${MACOSX_VERSION} LESS 7)
        set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
    endif()
endif()

# Detect that a supported Visual Version is used and set up helper variables VC_VER and VS_PLATFORM
if (MSVC)
    if (CMAKE_CL_64)
        set(VS_PLATFORM "x64")
    else()
        SET(VS_PLATFORM "Win32")
    endif()
    if (MSVC90)
        set(VC_VER "vc9")
    elseif(MSVC10)
        set(VC_VER "vc10")
    else()
        message(STATUS "-- Warning: Unsupported Visual Studio version detected. Currently Visual Studio 2008 and 2010 are supported".)
    endif()
endif()

# Set compiler warning levels.
# On MSVC, use warning level 4 but disable the following warnings:
# C4127 conditional expression is constant
# C4512 'class' : assignment operator could not be generated 
# TODO Ideally we wouldn't like to disable these, but if we don't, we get warning spam from Qt files.
if (MSVC)
    add_definitions (/W4 /wd4127 /wd4512)
else ()
    add_definitions (-Wall -Wextra -Wno-unknown-pragmas -Wno-unused -Wno-reorder -Wno-switch -Wno-write-strings)
endif()

# Set CMake custom module path & include them
# The following CMake modules are required for the project to build.
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} cmake)
include (Sagase)
include (ModuleSetup)
include (ConfigurePackages)
include (TundraInstall)
include (FindOgre)
include (FindDirectX)

# Disable unnecessary build types
set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo;Debug" CACHE STRING "Configurations" FORCE)

if (MSVC)
    # In Visual Studio, use unicode character set
    add_definitions(-DUNICODE -D_UNICODE)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_definitions(-DUNIX) # Currently, kNet uses this flag for Unix OSes. (required also for client-side headers). TODO: Clean this up so it's not required.
endif()

# On all platforms, we enable PCH files by default.
add_definitions (-DPCH_ENABLED)
SET(PCH_ENABLED 1)

# This setting affects only windows. Possibility to opt out of linking agains DirecX spesific libs.
# Enabled more efficient texture blitting to Ogre using DirectX. This makes RenderSystem_Direct3D9 mandatory.
set (ENABLE_DIRECTX 1)

# Set global hardcoded install prefix. User cannot change this at the moment, until we figure how we want to use this!
# Call the cleanup step that cleans the install prefix before every installations. This is important as module setups might change between builds.
set (CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/build)
set (TUNDRA_BIN ${PROJECT_BINARY_DIR}/bin)
setup_clean_install_step()

# Set normalized path for common environment variables
# If you want to use custom built dependencies along side with TUNDRA_DEP_PATH
# you can define the following env variables: 
# OGRE_HOME, SKYX_HOME, HYDRAX_HOME, KNET_DIR_QT47, BULLET_DIR, ASSIMP_DIR
file (TO_CMAKE_PATH "$ENV{TUNDRA_DEP_PATH}" ENV_TUNDRA_DEP_PATH)
file (TO_CMAKE_PATH "$ENV{QTDIR}" ENV_QT_DIR)
file (TO_CMAKE_PATH "$ENV{BOOST_ROOT}" ENV_BOOST_ROOT)
file (TO_CMAKE_PATH "$ENV{OGRE_HOME}" ENV_OGRE_HOME)
file (TO_CMAKE_PATH "$ENV{SKYX_HOME}" ENV_SKYX_HOME)
file (TO_CMAKE_PATH "$ENV{HYDRAX_HOME}" ENV_HYDRAX_HOME)
file (TO_CMAKE_PATH "$ENV{KNET_DIR}" ENV_KNET_DIR)
file (TO_CMAKE_PATH "$ENV{BULLET_DIR}" ENV_BULLET_DIR)
file (TO_CMAKE_PATH "$ENV{ASSIMP_DIR}" ENV_ASSIMP_DIR)

# Utilize cmakes cache for the environment variables. This is needed if eg. TUNDRA_DEP_PATH
# was set on the first run of cmake, but once eg. Visual Studio auto runs cmake (after changes to any CMakeLists.txt)
# it is no longer set. This will result us in losing all the acquired dependency information that was not found with
# cmake macros find_packade, find_library etc. that automatically cache. This is more flexible for many use cases to remember
# if we once got a valid TUNDRA_DEP_PATH. If env variable TUNDRA_DEP_PATH is present, the cache is never read but instead updated.
if (NOT ENV_TUNDRA_DEP_PATH)
    set (ENV_TUNDRA_DEP_PATH ${CACHED_ENV_TUNDRA_DEP_PATH})
else ()
    set (CACHED_ENV_TUNDRA_DEP_PATH ${ENV_TUNDRA_DEP_PATH} CACHE PATH "Tundra build dependency path" FORCE)
endif ()
if (NOT ENV_QT_DIR)
    set (ENV_QT_DIR ${CACHED_ENV_QT_DIR})
else ()
    set (CACHED_ENV_QT_DIR ${ENV_QT_DIR} CACHE PATH "Tundra build Qt path" FORCE)
endif ()
if (NOT ENV_OGRE_HOME)
    set (ENV_OGRE_HOME ${CACHED_ENV_OGRE_HOME})
else ()
    set (CACHED_ENV_OGRE_HOME ${ENV_OGRE_HOME} CACHE PATH "Tundra build Ogre path" FORCE)
endif ()

# If we still havent resolved TUNDRA_DEP_PATH report a fatal error.
# ENV_QT_DIR and ENV_OGRE_HOME are not fatal if missing.
if (NOT ENV_TUNDRA_DEP_PATH)
    message(FATAL_ERROR "TUNDRA_DEP_PATH could not be resolved from a environment variable or from cmake cache!")
endif ()

message ("\n\n=========== Configuring Dependencies ===========\n")

# Find needed external libraries, abort on error
if (NOT TUNDRA_NO_BOOST)
    configure_boost()
endif()
configure_qt4()
if (NOT APPLE) # Python and PythonQt are not available currently on Mac.
    if ("$ENV{TUNDRA_PYTHON_ENABLED}" STREQUAL "TRUE") 
        configure_python()
        configure_python_qt()
    endif()
endif()
configure_ogre()
if (NOT ANDROID)
    configure_qtpropertybrowser()
    configure_openal ()
endif()
use_package_ogg()
use_package_vorbis()

# This will find kNet and set the global include and library directories
use_package_knet() 

message ("\n** Adding global include and link directories")
use_package(OGRE)
use_package(QT4)
if (NOT ANDROID)
    use_package(OPENAL)
endif()

# OSX / Xcode needs Assimp as a global link directory, as it cannot find libassimp otherwise for eg. modules linking to OgreRenderingModule
if (APPLE)
    use_package_assimp()
endif()

# Convenience macro for including all TundraCore subfolders.
macro(UseTundraCore)
    include_directories(${PROJECT_BINARY_DIR}/src/Core/TundraCore/)
    include_directories(${PROJECT_BINARY_DIR}/src/Core/TundraCore/Asset)
    include_directories(${PROJECT_BINARY_DIR}/src/Core/TundraCore/Audio)
    include_directories(${PROJECT_BINARY_DIR}/src/Core/TundraCore/Console)
    include_directories(${PROJECT_BINARY_DIR}/src/Core/TundraCore/Framework)
    include_directories(${PROJECT_BINARY_DIR}/src/Core/TundraCore/Input)
    include_directories(${PROJECT_BINARY_DIR}/src/Core/TundraCore/Scene)
    include_directories(${PROJECT_BINARY_DIR}/src/Core/TundraCore/Ui)
endmacro()

# If the custom optional modules configuration file does not yet
# exist, create it from the template file.
if (NOT EXISTS CMakeBuildConfig.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CMakeBuildConfigTemplate.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/CMakeBuildConfig.txt)
endif ()

# The following defines are required to compile.
# These enable implicit conversions of common math types between different libraries and our internal math library.
add_definitions(-DMATH_QT_INTEROP)
add_definitions(-DMATH_ENABLE_STL_SUPPORT)
add_definitions(-DMATH_TUNDRA_INTEROP)
# Both MathGeoLib and kNet has same macros for logging, so disable them from the beforementioned.
add_definitions(-DLOGGING_SUPPORT_DISABLED)

if (ANDROID)
    add_definitions(-DQT_STL)
endif()

# Read the set of optional modules from another file
# that is kept outside the source control. 
# To configure the set of optional modules to add to the build,
# edit CMakeBuildConfig.txt and rerun cmake.
include(CMakeBuildConfig.txt)

# Collect translation files.
file(GLOB TRANSLATIONS_FILES bin/data/translations/*.ts)

# Collect ui-files which are stored into data/
file(GLOB ui_src bin/data/ui/*.ui)

set(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${ui_src})

if (UPDATE_LANGUAGE_TRANSLATIONS)
   message (STATUS "Update .ts files with new translation data.")
   update_translation_files(TRANSLATIONS_FILES)
endif()

update_qm_files(TRANSLATIONS_FILES)
message (STATUS "Language translations generated")

PrintUsedEnvVariables ()
PrintUsedBuildConfig ()
