# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Define library
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
add_library (Interfaces STATIC ${SOURCE_FILES})

# Have to define language, because there are no CPP files in the source directory
set_target_properties (Interfaces PROPERTIES LINKER_LANGUAGE CXX)
