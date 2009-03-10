# CMake version requirement
cmake_minimum_required (VERSION 2.4)

# Set CMake library search policy
if (COMMAND cmake_policy)
	cmake_policy (SET CMP0003 NEW)
endif (COMMAND cmake_policy)

# Set CMake custom module path & include them
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} CMakeModules)
include(Boost)
include(PoCo)
include(Curl)
include(XmlRpcEpi)

# Disable unnecessary build types
set (CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Configurations" FORCE)

# Set project name
project (rexNG)

# In Visual Studio, use unicode character set
if (MSVC)
	add_definitions (-DUNICODE -D_UNICODE)
endif (MSVC)

# Find needed external libraries, abort on error
FIND_BOOST()
FIND_POCO()
FIND_CURL()
FIND_XMLRPCEPI()

# Default additional include & library directories
include_directories (
	${PROJECT_SOURCE_DIR}/external_libs/include
)

# Library directories
link_directories (
	${PROJECT_SOURCE_DIR}/external_libs
)

# Add subprojects
add_subdirectory (Core)
add_subdirectory (Foundation)
add_subdirectory (Interfaces)
add_subdirectory (GeometrySystem)
add_subdirectory (OgreRenderingModule)
add_subdirectory (rex)
add_subdirectory (rexlogic)
add_subdirectory (TestModule)
add_subdirectory (UnitTests)
add_subdirectory (OpenSimProtocol)
