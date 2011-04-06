# Define target name and output directory
init_target (DebugStatsModule OUTPUT modules/core)

GetEntityComponents()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES DebugStats.h TimeProfilerWindow.h ParticipantWindow.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DDEBUGSTATS_MODULE_EXPORTS) 

use_modules (Framework Scene OgreRenderingModule EnvironmentModule
    SupportModules AssetModule Input Ui)

if (ENABLE_TAIGA_SUPPORT)
   use_modules (RexCommon ProtocolUtilities ProtocolModuleOpenSim ProtocolModuleTaiga)
endif()

if (OGREASSETEDITOR)
use_modules(OgreAssetEditorModule)
endif()

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_ogre()

link_modules (Framework Scene OgreRenderingModule EnvironmentModule
    SupportModules AssetModule Input Ui)

if (ENABLE_TAIGA_SUPPORT)
   link_modules (RexCommon ProtocolUtilities ProtocolModuleOpenSim ProtocolModuleTaiga)
endif()

if (OGREASSETEDITOR)
link_modules(OgreAssetEditorModule)
endif()

LinkEntityComponent(EntityComponents/EC_OpenSimPrim EC_OpenSimPrim)
LinkEntityComponent(EntityComponents/EC_OpenSimPresence EC_OpenSimPresence)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

if (MSVC)
	target_link_libraries (${TARGET_NAME} pdh.lib)
endif (MSVC)

final_target ()
