# Define target name and output directory
init_target (SceneManager)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES Entity.h SceneManager.h EC_Name.h EntityAction.h EC_Name.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_package (BOOST)
use_package (POCO)
use_package (QT4)
use_modules (Core Foundation Interfaces)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})

link_package (BOOST)
link_package (POCO)
link_package (QT4)
link_modules (Core Foundation Interfaces)

SetupCompileFlagsWithPCH()


final_target ()
