# Define target name and output directory
init_target (CAVEStereoModule OUTPUT plugins)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)

# SubFolders to project with filtering

MocFolder ()
UiFolder ()

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DCAVESTEREO_MODULE_EXPORTS)

use_modules (UiModule Core Foundation Interfaces RexCommon OgreRenderingModule )

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

link_modules (Core Foundation Interfaces RexCommon OgreRenderingModule)
link_ogre()

SetupCompileFlagsWithPCH()


final_target ()

