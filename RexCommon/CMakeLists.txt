# Define target name
init_target (RexCommon)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

use_package (BOOST)
use_package (POCO)
use_modules (RexCommon)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES})
use_framework_modules (Core  )

link_package (BOOST)
link_package (POCO)           


