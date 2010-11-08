# Define target name and output directory
init_target (JavascriptModule OUTPUT modules/core)

GetEntityComponents()

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)
file (GLOB MOC_FILES JavascriptModule.h ScriptMetaTypeDefines.h JavascriptEngine.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})

use_modules (Core Foundation Interfaces SceneManager Input)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

LinkEntityComponent(EntityComponents/EC_Script EC_Script)
LinkEntityComponent(EntityComponents/EC_DynamicComponent EC_DynamicComponent)

link_modules (Core Foundation Interfaces EC_Script Input)
link_ogre()

SetupCompileFlags()
CopyModuleXMLFile()

final_target ()
