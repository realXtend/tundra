# Define target name
init_target (Interfaces)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

use_package (BOOST)
use_package (POCO)
use_modules (Core Foundation Interfaces SceneManager)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES})

link_package (BOOST)
link_package (POCO)
link_modules (Core Foundation Interfaces SceneManager)

# Have to define language, because there are no CPP files in the source directory
set_target_properties (${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

final_target ()
