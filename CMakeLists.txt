# CMake version requirement
cmake_minimum_required (VERSION 2.4)

# In Visual Studio, use unicode character set
add_definitions (-DUNICODE -D_UNICODE)

# Disable unnecessary build types
set (CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Configurations" FORCE)

# Set project name
project (rexNG)

# Add subprojects
add_subdirectory (Core)
add_subdirectory (framework)
add_subdirectory (Interfaces)
add_subdirectory (GeometrySystem)
add_subdirectory (OgreRenderingSystem)
add_subdirectory (rex)
add_subdirectory (rexlogic)
add_subdirectory (TestSystem)
add_subdirectory (UnitTests)
