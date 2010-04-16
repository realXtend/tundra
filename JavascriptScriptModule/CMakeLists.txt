# Define target name and output directory
init_target (JavascriptScriptModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

use_package (BOOST)
use_package (POCO)
use_package (QT4)
use_modules (Core Foundation Interfaces SceneManager RexCommon)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES})

link_modules (Core Foundation Interfaces SceneManager RexCommon)
link_package (BOOST)
link_package (POCO)
link_package (QT4)


# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

final_target ()
