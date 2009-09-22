# Define target name and output directory
init_target (OpenSimProtocolModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

add_definitions (-DOSPROTO_MODULE_EXPORTS)

use_package (BOOST)
use_package (POCO)
use_package (XMLRPC)
use_modules (NetTestLogicModule HttpUtilities)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES})
use_framework_modules (Core Foundation Interfaces SceneManager RexCommon)

link_modules (RexCommon HttpUtilities)
link_package (BOOST)
link_package (POCO)
link_package (XMLRPC)


# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

final_target ()



