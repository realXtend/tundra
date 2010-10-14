# Define target name and output directory
init_target (UiExternalModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)

# Qt4 Moc files to "CMake Moc" subgroup
# and ui_*.h generated .h files to "Generated UI" subgroup
MocFolder ()
UiFolder ()

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DUIEXTERNAL_MODULE_EXPORTS)

use_package (BOOST)
use_package (QT4)
use_package (POCO)
use_package (CURL)
use_package (XMLRPC)
use_modules (UiExternalModule UiModule Core Foundation Interfaces  RexCommon ProtocolUtilities SupportModules SceneManager)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

link_modules (Core Foundation Interfaces RexCommon ProtocolUtilities SupportModules SceneManager)
link_package (BOOST)
link_package (QT4)
link_package (POCO)
link_package (XMLRPC)
link_package (CURL)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()

