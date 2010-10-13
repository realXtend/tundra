# Define target name and output directory
init_target (SupportModules OUTPUT modules/core)

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)
file (GLOB MOC_FILES ConsoleProxyWidget.h UiConsoleManager.h CommandManager.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})

add_definitions (-DCONSOLE_MODULE_EXPORTS)

use_package (BOOST)
use_package (POCO)
use_package (OGRE)
use_package (QT4)
use_modules (Core Foundation Interfaces RexCommon Ui Input)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

link_modules (Core Foundation Interfaces RexCommon Ui)
link_ogre()
link_package (BOOST)
link_package (POCO)
link_package (QT4)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
