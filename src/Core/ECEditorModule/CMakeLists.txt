# Define target name and output directory
init_target(ECEditorModule OUTPUT plugins)

GetEntityComponents()

# Define source files
file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB XML_FILES *.xml)
file(GLOB MOC_FILES ECEditorModule.h ECEditorWindow.h ECAttributeEditor.h ECBrowser.h
    EcXmlEditorWidget.h MultiEditPropertyFactory.h MultiEditPropertyManager.h EntityPlacer.h
    ECComponentEditor.h LineEditPropertyFactory.h AddComponentDialog.h EntityActionDialog.h
    FunctionDialog.h TreeWidgetItemExpandMemory.h SceneStructureModule.h SceneStructureWindow.h
    SceneTreeWidget.h AddContentWindow.h AssetsWindow.h AssetTreeWidget.h RequestNewAssetDialog.h
    CloneAssetDialog.h EditorButtonFactory.h TransformEditor.h NewEntityDialog.h AddAttributeDialog.h
    KeyBindingsConfigWindow.h)

set(SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder()

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

set(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

add_definitions(-DECEDITOR_MODULE_EXPORTS)

use_package(QT_PROPERTY_BROWSER)
use_package_qtpropertybrowser()
# Audio is needed only for visualization purposes to access the attributes of EC_Sound.
UseTundraCore()
use_core_modules(TundraCore Math OgreRenderingModule TundraProtocolModule)

build_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_package(QT4)
link_package_knet()
link_ogre()
link_package(QT_PROPERTY_BROWSER)
link_package_qtpropertybrowser()
link_modules(TundraCore Math OgreRenderingModule TundraProtocolModule)

# EC_Sound is needed only for visualization purposes to access the attributes of EC_Sound.
link_entity_components(EC_TransformGizmo EC_Highlight EC_Sound)

#TODO Fix up QtPropertyBrowser file naming on Mac.
if (APPLE)
    target_link_libraries(${TARGET_NAME} QtSolutions_PropertyBrowser-head)
endif()

SetupCompileFlagsWithPCH()

final_target()

# Install windows deps
setup_install_windows_deps("QtSolutions_PropertyBrowser-head.dll" Release|RelWithDebInfo "QtSolutions_PropertyBrowser-headd.dll" Debug)
