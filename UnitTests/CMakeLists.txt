# Define target name and output directory
init_target (UnitTests OUTPUT ./)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../Core
	../Foundation
	../Interfaces
	../SceneManager
	../TestModule
	../SupportModules
	../InputModuleOIS
)

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()
INCLUDE_OIS()
INCLUDE_OGRE()

# Define executable
if (NOT MSVC)
	SET (EXECUTABLE_OUTPUT_PATH ${TARGET_DIR})
endif (NOT MSVC)

add_executable (${TARGET_NAME} ${SOURCE_FILES})
set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)

# Define libraries to be linked into target
target_link_libraries (${TARGET_NAME} Core Foundation 
Interfaces SupportModules)
if (MSVC)
   target_link_libraries (${TARGET_NAME} optimized dbghelp.lib)
endif (MSVC)


# External library linkage
LINK_POCO()
LINK_BOOST()
LINK_OGRE()
