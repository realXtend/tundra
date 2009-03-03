# Define target name
set (TARGET_NAME Core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../external_libs/include
)

# Library directories
link_directories (
	../external_libs
)

# Define static library
add_library (${TARGET_NAME} STATIC ${SOURCE_FILES})
