# Define target name
init_target (RexCommon)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
use_module_headers (${TARGET_NAME} RexCommon)

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()

# Define static library

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES})
use_framework_modules (${TARGET_NAME} Core)

if (NOT MSVC)
  LINK_BOOST()
  LINK_POCO()
endif()
