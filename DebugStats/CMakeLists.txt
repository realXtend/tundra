# Define target name and output directory
init_target (DebugStats OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES})
use_framework_modules (${TARGET_NAME} Core Foundation Interfaces SceneManager)
use_module_headers (${TARGET_NAME} RexCommon ReXLogic OpenSimProtocol OgreRenderingModule)
use_package (BOOST)
use_package (POCO)

target_link_libraries (${TARGET_NAME} RexCommon RexLogicModule OpenSimProtocolModule OgreRenderingModule)

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)
