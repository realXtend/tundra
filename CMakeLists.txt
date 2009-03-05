# CMake version requirement
cmake_minimum_required (VERSION 2.4)

# Set CMake library search policy
if (COMMAND cmake_policy)
	cmake_policy (SET CMP0003 NEW)
endif (COMMAND cmake_policy)

# Disable unnecessary build types
set (CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Configurations" FORCE)

# Set project name
project (rexNG)

# In Visual Studio, use unicode character set
if (MSVC)
	add_definitions (-DUNICODE -D_UNICODE)
endif (MSVC)

# Search for external libraries
# Boost
if (NOT MSVC)
	# Static linking needed for UnitTests main
	set (Boost_USE_STATIC_LIBS ON)
	find_package (Boost COMPONENTS date_time filesystem system thread unit_test_framework)
endif (NOT MSVC)

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
