# Define target name
init_target (RexCommon)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

use_package (BOOST)
use_package (POCO)
use_package (QT4)
use_modules (Core)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES})

link_package (BOOST)
link_package (POCO)
link_package (QT4)
link_modules (Core)

SetupCompileFlags()

final_target ()
