# Define target name and output directory
init_target (ECEditorModule OUTPUT modules/core)

GetEntityComponents()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES ECEditorModule.h ECEditorWindow.h ECAttributeEditor.h
    ECBrowser.h EcXmlEditorWidget.h MultiEditPropertyFactory.h MultiEditPropertyManager.h EntityPlacer.h
    ECComponentEditor.h LineEditPropertyFactory.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

add_definitions (-DECEDITOR_MODULE_EXPORTS)

use_package (BOOST)
use_package (POCO)
use_package (QT4)
use_package (OGRE)
use_package (QT_PROPERTY_BROWSER)
use_modules (Core Foundation Interfaces SceneManager OgreRenderingModule RexCommon SupportModules ProtocolUtilities Input)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_package (BOOST)
link_package (POCO)
link_package (QT4)
link_ogre()
link_package (QT_PROPERTY_BROWSER)
link_modules (Core Foundation Interfaces SceneManager OgreRenderingModule RexCommon SupportModules Input)

LinkEntityComponent(EntityComponents/EC_DynamicComponent EC_DynamicComponent)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
