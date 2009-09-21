# Define target name
init_target (RexCommon)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../Core
	../RexCommon
)

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()

# Define static library

add_library (${TARGET_NAME} STATIC ${SOURCE_FILES})

# Because this is static library, compile it with -fPIC in Linux.

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)

if (NOT MSVC)
  LINK_BOOST()
  LINK_POCO()
endif()