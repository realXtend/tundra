# Define target name and output directory
init_target (UnitTests OUTPUT ./)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

use_package (BOOST)
use_package (POCO)

use_modules (Core Foundation Interfaces SceneManager TestModule SupportModules RexCommon)

build_executable (${TARGET_NAME} ${SOURCE_FILES})

link_modules (Core Foundation Interfaces SceneManager TestModule SupportModules RexCommon)

link_package (BOOST)
link_package (POCO)

final_target ()
