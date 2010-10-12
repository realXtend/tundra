# Define target name and output directory
init_target (OgreRenderingModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB UI_FILES *.ui)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES NaaliRenderWindow.h RendererSettings.h EC_*.h Renderer.h CAVESettingsWidget.h CAVEManager.h CAVEViewSettings.h CAVEViewSettingsAdvanced.h StereoController.h StereoWidget.h ExternalRenderWindow.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()
#UiFolder ()

# Build parameters required by Qt - define files for moc prepass, process resource and ui files.
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
#QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

if (HYDRAX)
    INCLUDE_HYDRAX()
endif (HYDRAX)

if (CAELUM)
    add_definitions (-DCAELUM)
endif (CAELUM)

if (HYDRAX)
    add_definitions (-DHYDRAX)
endif (HYDRAX)

add_definitions (-DOGRE_MODULE_EXPORTS)

#add_definitions (-DUSE_NVIDIA_PERFHUD)

use_package (BOOST)
use_package (POCO)
use_package (OGRE)
use_package (QT4)
use_modules (Core Foundation Interfaces SceneManager AssetModule RexCommon ProtocolUtilities Input Ui)

if (CAELUM)
    use_package (CAELUM)
endif (CAELUM)

if (HYDRAX)
    use_package (HYDRAX)
endif (HYDRAX)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

link_package (BOOST)
link_package (POCO)
link_package (OGRE)
link_package (QT4)
link_modules (Core Foundation Interfaces SceneManager RexCommon Ui)

if (CAELUM)
    link_package (CAELUM)
endif (CAELUM)

if (HYDRAX)
    link_package (HYDRAX)
endif (HYDRAX)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
