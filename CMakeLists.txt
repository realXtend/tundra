# CMake version requirement
cmake_minimum_required (VERSION 2.4)

# Set CMake library search policy
if (COMMAND cmake_policy)
	cmake_policy (SET CMP0003 NEW)
endif (COMMAND cmake_policy)

# Set CMake custom module path & include them
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} CMakeModules)
include (Boost)
include (PoCo)
include (Curl)
include (XmlRpcEpi)
include (Ogre)
include (Gtkmm)
include (Python)

# Disable unnecessary build types
set (CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Configurations" FORCE)

# Set common directory for installing non-system dependencies
set (REX_DEP_PATH $ENV{REX_DEP_PATH})

# Create config files from example config files 
if (NOT EXISTS ./bin/data/app_config.xml)
	execute_process (COMMAND cmake -E copy ./bin/data/app_config_example.xml ./bin/data/app_config.xml)
endif (NOT EXISTS ./bin/data/app_config.xml)

# Set project name
project (realXtend-viewer)

# In Visual Studio, use unicode character set
if (MSVC)
	add_definitions (-DUNICODE -D_UNICODE)
endif (MSVC)

# Find needed external libraries, abort on error
FIND_BOOST()
FIND_POCO()
FIND_CURL()
FIND_XMLRPCEPI()
FIND_OGRE()

# Default additional include & library directories
include_directories (${REX_DEP_PATH}/include)

# Library directories
link_directories (${REX_DEP_PATH}/lib)

# Add subprojects
add_subdirectory (Core)
add_subdirectory (Foundation)
add_subdirectory (Interfaces)
add_subdirectory (SceneModule)
add_subdirectory (OgreRenderingModule)
add_subdirectory (Application)
add_subdirectory (ReXLogic)
add_subdirectory (TestModule)
add_subdirectory (UnitTests)
add_subdirectory (OpenSimProtocol)
add_subdirectory (GtkmmUI)
#add_subdirectory (NetTestLogicModule)
#add_subdirectory (PythonModule)
add_subdirectory (SupportModules)
