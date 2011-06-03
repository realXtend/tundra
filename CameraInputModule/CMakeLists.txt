# Define target name and output directory
init_target (CameraInputModule OUTPUT modules/core)

add_definitions (-DCAMERAINPUT_MODULE_EXPORTS)

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (XML_FILES CameraInputModule.xml)
set (MOC_FILES CameraInputModule.h CameraInput.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})
set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_modules (Core Foundation Interfaces Asset Scene)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

link_modules (Core Foundation Interfaces Asset Scene)
link_ogre()
link_package_opencv()

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
