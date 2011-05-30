# Define target name and output directory
init_target (ARModule OUTPUT modules/core)

add_definitions (-DAR_MODULE_EXPORTS)

# Define source files 
MocFolder ()
UiFolder ()

file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)
file (GLOB RESOURCE_FILES ui/*.qrc)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})
set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

use_modules (Core Foundation Interfaces Scene Asset Ui Input Devices OgreRenderingModule CameraInputModule TundraLogicModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

link_package (BOOST)
link_package (POCO)
link_package (QT4)

link_ogre()
link_package_opencv()

link_modules (Core Foundation Interfaces Scene Asset Ui Input Devices OgreRenderingModule CameraInputModule TundraLogicModule)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
