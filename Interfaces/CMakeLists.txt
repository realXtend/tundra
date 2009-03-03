# Define target name
set (TARGET_NAME Interfaces)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Define static library
add_library (${TARGET_NAME} STATIC ${SOURCE_FILES})

# Have to define language, because there are no CPP files in the source directory
set_target_properties (${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
