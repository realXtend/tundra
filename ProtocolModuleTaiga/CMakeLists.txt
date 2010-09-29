# Define target name and output directory
init_target (ProtocolModuleTaiga OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES TaigaLoginThread.h TaigaWorldSession.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

add_definitions (-DTAIGAPROTO_MODULE_EXPORTS)

use_package (BOOST)
use_package (POCO)
use_package (XMLRPC)
use_package (QT4)
use_package (CURL)
use_modules (Core Foundation Interfaces RexCommon HttpUtilities RpcUtilities ProtocolUtilities)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} )

link_modules (Core Foundation Interfaces RexCommon HttpUtilities RpcUtilities ProtocolUtilities)
link_package (BOOST)
link_package (POCO)
link_package (XMLRPC)
link_package (QT4)
link_package (CURL)

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
    add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

SetupCompileFlagsWithPCH()

final_target ()
