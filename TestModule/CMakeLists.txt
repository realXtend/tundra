# Define target name and output directory
set (TARGET_NAME TestModule)
set (TARGET_DIR ${PROJECT_BINARY_DIR}/bin/modules/test)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../external_libs/include
	../Core
	../framework
	../GeometrySystem
	../Interfaces
	../OgreRenderingSystem
)

# Library directories
link_directories (
	../external_libs
)

# Define dynamic library
if (NOT MSVC)
	SET (LIBRARY_OUTPUT_PATH ${TARGET_DIR})
endif (NOT MSVC)

add_library (${TARGET_NAME} SHARED ${SOURCE_FILES})
set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)

if (MSVC)
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different $(TargetPath) ${TARGET_DIR})
endif (MSVC)

# Define libraries to be linked into target
target_link_libraries (${TARGET_NAME} framework)

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)