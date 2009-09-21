# Define target name and output directory
init_target (UnitTests OUTPUT ./)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

build_executable (${TARGET_NAME} ${SOURCE_FILES})
use_framework_modules (${TARGET_NAME} Core Foundation Interfaces SceneManager)
use_module_headers (${TARGET_NAME} TestModule SupportModules InputModuleOIS)
target_link_libraries (${TARGET_NAME} SupportModules)
