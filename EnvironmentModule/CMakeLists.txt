# Define target name and output directory
init_target (EnvironmentModule OUTPUT plugins)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB UI_FILES ui/*.ui)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES EnvironmentModule.h TerrainLabel.h EC_*.h TerrainWeightEditor.h)
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

if (CAELUM)
    use_package (CAELUM)
endif (CAELUM)

use_modules (Framework Scene OgreRenderingModule Asset Console)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${RESOURCE_SRCS} ${UI_SRCS})
link_modules (Framework Scene OgreRenderingModule Asset Console)

link_ogre()

if (CAELUM)
    link_package (CAELUM)
endif (CAELUM)

SetupCompileFlagsWithPCH()

final_target ()
