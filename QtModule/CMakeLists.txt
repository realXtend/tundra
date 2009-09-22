# Define target name and output directory
init_target (QtModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)
file (GLOB RESOURCE_FILES resource/*.qrc)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DQT_MODULE_EXPORTS)

use_package (BOOST)
use_package (OGRE)
use_package (OIS) # ///\todo Remove.
use_package (QT4)
use_package (POCO)
use_modules (RexCommon OgreRenderingModule InputModuleOIS)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})
use_framework_modules (Core Foundation Interfaces SceneManager)

link_modules (RexCommon OgreRenderingModule InputModuleOIS)
link_package (OIS)
link_package (OGRE)
link_package (BOOST)
link_package (OGRE)
link_package (OIS) # ///\todo Remove.
link_package (QT4)
link_package (POCO)


# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

