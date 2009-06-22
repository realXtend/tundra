# Define target name and output directory
set (TARGET_NAME RexLogicModule)
set (TARGET_DIR ${PROJECT_BINARY_DIR}/bin/modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES RexLoginWindow.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../Core
	../Foundation
	../SceneManager
	../Interfaces
	../OgreRenderingModule
	../OpenSimProtocol
	../RexCommon
	../InputModuleOIS
	../QtModule
	../GtkmmUI
)

# Library directories
#link_directories (
	#../OpenSimProtocol/Debug
	#../OpenSimProtocol/Release
#)

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()
INCLUDE_OGRE()
INCLUDE_GTKMM()
INCLUDE_QT4()

include(${QT_USE_FILE})
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DMODULE_EXPORTS -DUSE_QT)

# Define dynamic library
if (NOT MSVC)
	SET (LIBRARY_OUTPUT_PATH ${TARGET_DIR})
endif (NOT MSVC)

add_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} )


set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)
set_target_properties (${TARGET_NAME} PROPERTIES PREFIX "")
set_target_properties (${TARGET_NAME} PROPERTIES LINK_INTERFACE_LIBRARIES "")

# External library linkage
LINK_BOOST()
LINK_POCO()
LINK_OGRE()
LINK_GTKMM()
LINK_QT4()

if (MSVC)
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${TARGET_DIR})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TargetPath)\" ${TARGET_DIR})
endif (MSVC)

# Define libraries to be linked into target
target_link_libraries (${TARGET_NAME} Foundation Interfaces OpenSimProtocolModule RexCommon OgreRenderingModule GtkmmUI)

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
