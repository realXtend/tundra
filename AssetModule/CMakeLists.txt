# Define target name and output directory
init_target (AssetModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB H_MOC_FILES QtHttpAssetProvider.h QtHttpAssetTransfer.h AssetCache.h)
file (GLOB XML_FILES *.xml)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${H_MOC_FILES})

add_definitions (-DASSET_MODULE_EXPORTS)
set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)
use_package (BOOST)
use_package (POCO)
use_package (CURL)
use_package (XMLRPC)
use_package (QT4)
use_modules (Core Foundation Interfaces RexCommon ProtocolUtilities HttpUtilities ProtocolModuleOpenSim ProtocolModuleTaiga)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_package (BOOST)
link_package (POCO)
link_package (CURL)
link_package (XMLRPC)
link_package (QT4)
link_modules (Core Foundation Interfaces RexCommon ProtocolUtilities HttpUtilities ProtocolModuleOpenSim ProtocolModuleTaiga)

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
    add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

SetupCompileFlagsWithPCH()

final_target ()
