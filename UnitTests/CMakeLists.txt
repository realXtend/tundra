# Define target name and output directory
set (TARGET_NAME UnitTests)
set (TARGET_DIR ${PROJECT_BINARY_DIR}/bin)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../Core
	../Foundation
	../Interfaces
	../SceneModule
	../TestModule
	../SupportModules
	../InputModuleOIS
)

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()
INCLUDE_OIS()
# This is stupid, but seems to be needed for MSVC because linking to SupportModules pulls in OgreMain
INCLUDE_OGRE()

add_definitions (-DMODULE_EXPORTS)

# Define executable
if (NOT MSVC)
	SET (EXECUTABLE_OUTPUT_PATH ${TARGET_DIR})
endif (NOT MSVC)

add_executable (${TARGET_NAME} ${SOURCE_FILES})
set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)

if (MSVC)
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TargetPath)\" ${TARGET_DIR})
endif (MSVC)

# Define libraries to be linked into target
target_link_libraries (${TARGET_NAME} Core Foundation SceneModule 
Interfaces SupportModules)
if (MSVC)
   target_link_libraries (${TARGET_NAME} optimized dbghelp.lib)
endif (MSVC)


# External library linkage
LINK_POCO()
LINK_BOOST()
