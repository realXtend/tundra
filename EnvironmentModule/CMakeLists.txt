macro (MocFolder)
    file (GLOB MOCS_TO_SOURCE_GROUP *.cxx */*.cxx)
    source_group ("CMake Moc" FILES ${MOCS_TO_SOURCE_GROUP})
endmacro (MocFolder)

#############################################################################################

# Define target name and output directory
init_target (EnvironmentModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB UI_FILES ui/*.ui)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES Water.h Environment.h Sky.h Terrain.h EnvironmentEditor.h TerrainLabel.h PostProcessWidget.h EC_*.h TerrainWeightEditor.h)
file (GLOB RESOURCE_FILES resource/*.qrc)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DENVIRONMENT_MODULE_EXPORTS)
set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${UI_FILES} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

if (CAELUM)
	add_definitions (-DCAELUM)
endif (CAELUM)


if (HYDRAX)
	add_definitions (-DHYDRAX)
endif (HYDRAX)

use_package (BOOST)
use_package (POCO)
use_package (CURL)
use_package (XMLRPC)
use_package (OGRE)
use_package (QT4)

if (CAELUM)
    use_package (CAELUM)
endif (CAELUM)

use_modules (Core Foundation Interfaces SceneManager RexCommon OgreRenderingModule ProtocolUtilities ProtocolModuleTaiga ProtocolModuleOpenSim) 
build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${RESOURCE_SRCS} ${UI_SRCS})
link_modules (Core Foundation Interfaces RexCommon SceneManager OgreRenderingModule ProtocolUtilities ProtocolModuleTaiga ProtocolModuleOpenSim)

link_package (BOOST)
link_package (POCO)
link_package (CURL)
link_package (XMLRPC)
link_package (OGRE)
link_package (QT4)

if (CAELUM)
    link_package (CAELUM)
endif (CAELUM)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
