# CMake version requirement
cmake_minimum_required (VERSION 2.4)

# Disable unnecessary build types
set (CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Configurations" FORCE)

# Set project name
project (rexNG)

# In Visual Studio, use unicode character set
if (MSVC)
	add_definitions (-DUNICODE -D_UNICODE)
endif (MSVC)

# Add subprojects
add_subdirectory (Core)
add_subdirectory (framework)
add_subdirectory (Interfaces)
add_subdirectory (GeometrySystem)
add_subdirectory (OgreRenderingModule)
add_subdirectory (rex)
add_subdirectory (rexlogic)
add_subdirectory (TestModule)
add_subdirectory (UnitTests)
add_subdirectory (OpenSimProtocol)
