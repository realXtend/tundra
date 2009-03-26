# Define target name and output directory
set (TARGET_NAME DebugStats)
set (TARGET_DIR ${PROJECT_BINARY_DIR}/bin/modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../Core
	../Foundation
	../Interfaces
	../GtkmmUI
	../SceneModule
)

# Disable warning
if (MSVC)
   set_property (DIRECTORY PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS)
endif (MSVC)

# In Visual Studio, use unicode character set
if (MSVC)
	add_definitions (-DUNICODE -D_UNICODE)
endif (MSVC)

FIND_GTKMM()

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()
INCLUDE_GTKMM()

add_definitions (-DMODULE_EXPORTS)

# Define dynamic library
if (NOT MSVC)
	SET (LIBRARY_OUTPUT_PATH ${TARGET_DIR})
endif (NOT MSVC)

add_library (${TARGET_NAME} SHARED ${SOURCE_FILES})
set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)
set_target_properties (${TARGET_NAME} PROPERTIES PREFIX "")

if (MSVC)
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${TARGET_DIR})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TargetPath)\" ${TARGET_DIR})
	target_link_libraries (${TARGET_NAME} )
endif (MSVC)

target_link_libraries (${TARGET_NAME} Foundation SceneModule)

# External library linkage (needs target defined)
LINK_POCO()
LINK_GTKMM()

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)



