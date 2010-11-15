# Define target name and output directory
init_target (SceneStructureModule OUTPUT modules/core)

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES SceneStructureModule.h SceneStructureWindow.h SceneTreeWidget.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_package_assimp()

use_modules (Core Foundation Interfaces Ui Input SceneManager ECEditorModule TundraLogicModule
    OgreRenderingModule AssetModule RexCommon ProtocolUtilities)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

LinkEntityComponent(EntityComponents/EC_DynamicComponent EC_DynamicComponent)
GetOpenAssetImport()

link_modules (Core Foundation Interfaces Ui Input SceneManager ECEditorModule TundraLogicModule
    OgreRenderingModule)

link_ogre()

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
