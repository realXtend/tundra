# Define target name and output directory
init_target (viewer OUTPUT ./)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Preprocessor defines
if (MSVC)
   set_property (DIRECTORY PROPERTY COMPILE_DEFINITIONS_RELEASE _DMEMDUMP)
   set_property (DIRECTORY PROPERTY COMPILE_DEFINITIONS_DEBUG _DMEMORYLEAKS)
endif (MSVC)

use_package (BOOST)
use_package (POCO)
use_modules (SupportModules TestModule)

build_executable (${TARGET_NAME} ${SOURCE_FILES})
use_framework_modules (Core Foundation Interfaces)

link_package (BOOST)
link_package (POCO)

final_target ()

