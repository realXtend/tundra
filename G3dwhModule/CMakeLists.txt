# Define target name and output directory
init_target (G3dwhModule OUTPUT modules/core)

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB C_FILES)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)
file (GLOB MOC_FILES G3dwhModule.h G3dwhDialog.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES} ${C_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})

use_modules (Core Foundation Interfaces Input SceneStructureModule)
use_package_quazip()
build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

link_modules (Core Foundation Interfaces Input SceneStructureModule)

# Use link macros defined in CMakeModules/ConfigurePackages.cmake here
#link_package_qtmobility()
#link_ogre()
link_package_quazip()

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
