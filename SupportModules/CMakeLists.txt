# Define target name and output directory
set (TARGET_NAME SupportModules)
set (TARGET_DIR ${PROJECT_BINARY_DIR}/bin/modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../Core
	../Foundation
	../Interfaces
	../OgreRenderingModule
	../InputModuleOIS
)
		
# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()
INCLUDE_OGRE()
INCLUDE_OIS()
INCLUDE_GTKMM()

add_definitions (-DCONSOLE_MODULE_EXPORTS -DMODULE_EXPORTS)

# Define dynamic library
if (NOT MSVC)
	SET (LIBRARY_OUTPUT_PATH ${TARGET_DIR})
endif (NOT MSVC)

add_library (${TARGET_NAME} SHARED ${SOURCE_FILES})
set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)
set_target_properties (${TARGET_NAME} PROPERTIES PREFIX "")

# Define libraries to be linked into target
target_link_libraries (${TARGET_NAME} Core Foundation SceneModule)

# External library linkage
LINK_BOOST()
LINK_POCO()
LINK_OGRE()

if (MSVC)
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${TARGET_DIR})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TargetPath)\" ${TARGET_DIR})
endif (MSVC)

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

# Precompiled headers on MSVC
if (MSVC)
	foreach( src_file ${CPP_FILES} )
        	set_source_files_properties(
            		${src_file}
            		PROPERTIES
            		COMPILE_FLAGS "/YuStableHeaders.h"
        	)
	endforeach( src_file ${CPP_FILES} )

	set_source_files_properties(StableHeaders.cpp
        	PROPERTIES
        	COMPILE_FLAGS "/YcStableHeaders.h"
        )
endif (MSVC)
