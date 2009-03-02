# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../external_libs/include
	../Core
	../framework
	../Interfaces
)

# Library directories
link_directories (
	../external_libs
)

# Define dynamic library
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
add_library (GeometrySystem SHARED ${SOURCE_FILES})


