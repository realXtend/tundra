# Define target name and output directory
init_target (UnitTests OUTPUT ./)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
use_module_headers (${TARGET_NAME} TestModule SupportModules InputModuleOIS)

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()
INCLUDE_OIS()
INCLUDE_OGRE()

add_executable (${TARGET_NAME} ${SOURCE_FILES})
set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)

use_framework_modules (${TARGET_NAME} Core Foundation Interfaces SceneManager)
target_link_libraries (${TARGET_NAME} SupportModules)


if (MSVC)
   target_link_libraries (${TARGET_NAME} optimized dbghelp.lib)
endif (MSVC)


# External library linkage
LINK_POCO()
LINK_BOOST()
LINK_OGRE()
