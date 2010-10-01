# Define target name and output directory
init_target (QtInputModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_H_FILES QtInputService.h QtInputModule.h KeyBindingsConfigWindow.h)
file (GLOB XML_FILES *.xml)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${MOC_H_FILES})

add_definitions (-DQTINPUT_MODULE_EXPORTS)

use_package (BOOST)
use_package (POCO)
use_package (OGRE)
use_package (QT4)
use_modules (Core Foundation Interfaces RexCommon Ui)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_package (BOOST)
link_package (POCO)
link_package (OGRE)
link_package (QT4)
link_modules (Core Foundation Interfaces Ui)

SetupCompileFlags()
CopyModuleXMLFile()

final_target ()
