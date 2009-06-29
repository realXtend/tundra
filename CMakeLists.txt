# Set project name
project (realXtend-viewer)

# CMake version requirement
cmake_minimum_required (VERSION 2.4)

# Set CMake library search policy
if (COMMAND cmake_policy)
	cmake_policy (SET CMP0003 NEW)
endif (COMMAND cmake_policy)

# Set CMake custom module path & include them
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} CMakeModules)

# The following CMake modules are required for the project
# to build.
include (Boost)
include (PoCo)
include (Curl)
include (XmlRpcEpi)
include (Ogre)
include (OIS)
include (OpenJpeg)
include (Qt)

# The following CMake modules are optional.
include (Gtkmm)
include (Python)
#include (OpenAL)
include (Caelum)
#include (Hydrax)
#include (TinyJson)

# Disable unnecessary build types
set (CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Configurations" FORCE)

# Set common directory for installing non-system dependencies
set (REX_DEP_PATH $ENV{REX_DEP_PATH})

# Use Caelum, but not Hydrax for now.
SET (CAELUM 1)
SET (HYDRAX 0)

# Possibility to disable console output and make viewer a true Windows application (Windows only)
SET (WINDOWS_APP 0)
if (MSVC)
    if (WINDOWS_APP)
        add_definitions (-DWINDOWS_APP)
    endif (WINDOWS_APP)
endif (MSVC)

# In Visual Studio, use unicode character set
if (MSVC)
	add_definitions (-DUNICODE -D_UNICODE)
endif (MSVC)

# Generate profiling data. Currently disabled because of threadsafety issues.
if (MSVC)
	add_definitions (-DPROFILING)
endif (MSVC)

# Find needed external libraries, abort on error
FIND_BOOST()
FIND_POCO() #TODO: didn't detect on fedora
FIND_CURL()
FIND_XMLRPCEPI() #TODO: didn't detect on fedora
FIND_OGRE()
FIND_OIS()
FIND_OPENJPEG()
FIND_GTKMM()
#FIND_OPENAL()
FIND_QT4()
FIND_PYTHON()

if (CAELUM)
	FIND_CAELUM()
endif (CAELUM)

if (HYDRAX)
	FIND_HYDRAX()
endif (HYDRAX)

# Default additional include & library directories
include_directories (${REX_DEP_PATH}/include)

# Library directories
link_directories (${REX_DEP_PATH}/lib)

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
add_subdirectory (ReXLogic)
add_subdirectory (SupportModules)
add_subdirectory (AssetModule)
add_subdirectory (QtModule)

# The following are not really core modules,
# but you can't do anything practical without
# having them so, they're effectively required.
add_subdirectory (InputModuleOIS)
add_subdirectory (TextureDecoderModule)
add_subdirectory (OpenSimProtocol)

# The following modules are optional. You may
# leave out any of these, and the project should
# still build and work. If it doesn't, please
# report it as a bug.

#add_subdirectory (TestModule)
#add_subdirectory (UnitTests) #TODO: didn't compile on fedora
#add_subdirectory (GtkmmUI)
#add_subdirectory (DebugStats)
#add_subdirectory (NetTestLogicModule)
add_subdirectory (PythonModule)
add_subdirectory (CommunicationModule)
#add_subdirectory (CommunicationUIModule)
#add_subdirectory (OpenALAudioModule)
add_subdirectory (RexCBLogin)