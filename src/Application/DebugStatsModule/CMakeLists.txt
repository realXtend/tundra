# Define target name and output directory
init_target(DebugStatsModule OUTPUT plugins)

GetEntityComponents()

MocFolder()

# Define source files
file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB MOC_FILES DebugStats.h TimeProfilerWindow.h)
file(GLOB UI_FILES ui/*.ui)

set(SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI (UI_SRCS ${UI_FILES})

add_definitions(-DDEBUGSTATS_MODULE_EXPORTS)

use_package_bullet()
UseTundraCore()
use_core_modules(TundraCore Math OgreRenderingModule AssetModule EnvironmentModule PhysicsModule ECEditorModule)
use_entity_components(EC_Script)

build_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

link_package(QT4)
link_package_knet()
link_ogre()
link_modules(TundraCore OgreRenderingModule EnvironmentModule AssetModule PhysicsModule ECEditorModule)
link_entity_components(EC_Script)

SetupCompileFlagsWithPCH()

if (MSVC)
    target_link_libraries (${TARGET_NAME} pdh.lib ws2_32.lib)
endif (MSVC)

final_target()
