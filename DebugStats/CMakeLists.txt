# Define target name and output directory
init_target (DebugStats OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

use_package (BOOST)
use_package (POCO)
use_modules (RexCommon ReXLogic OpenSimProtocol OgreRenderingModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES})
use_framework_modules (Core Foundation Interfaces SceneManager)

link_modules (RexCommon RexLogicModule OpenSimProtocolModule OgreRenderingModule)
link_package (BOOST)
link_package (POCO)


# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

copy_target ()
