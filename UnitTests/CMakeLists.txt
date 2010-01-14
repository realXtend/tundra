# Define target name and output directory
init_target (UnitTests OUTPUT ./)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

use_modules (TestModule SupportModules)

build_executable (${TARGET_NAME} ${SOURCE_FILES})
use_framework_modules (Core Foundation Interfaces SceneManager)

link_modules (SupportModules)

final_target ()
