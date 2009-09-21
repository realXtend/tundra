# Define target name and output directory
init_target (DebugStats OUTPUT modules/core)

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
	../SceneManager
	../RexCommon
	../ReXLogic
	../OpenSimProtocol
	../OgreRenderingModule
)

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()
INCLUDE_GTKMM()

add_library (${TARGET_NAME} SHARED ${SOURCE_FILES})
set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)
set_target_properties (${TARGET_NAME} PROPERTIES PREFIX "")

target_link_libraries (${TARGET_NAME} Foundation RexCommon RexLogicModule OpenSimProtocolModule OgreRenderingModule Interfaces)

# External library linkage (needs target defined)
LINK_POCO()
LINK_GTKMM()

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)



