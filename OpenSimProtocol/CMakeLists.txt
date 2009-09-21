# Define target name and output directory
init_target (OpenSimProtocolModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../Core
	../RexCommon
	../Foundation
	../Interfaces
	../NetTestLogicModule
	../HttpUtilities
)


# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()
INCLUDE_XMLRPCEPI()

add_definitions (-DOSPROTO_MODULE_EXPORTS)

# Define dynamic library
if (NOT MSVC)
	SET (LIBRARY_OUTPUT_PATH ${TARGET_DIR})
endif (NOT MSVC)

add_library (${TARGET_NAME} SHARED ${SOURCE_FILES})
set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)
set_target_properties (${TARGET_NAME} PROPERTIES PREFIX "")
set_target_properties (${TARGET_NAME} PROPERTIES LINK_INTERFACE_LIBRARIES "")

# Define libraries to be linked into target
target_link_libraries (${TARGET_NAME} Foundation RexCommon Interfaces Core HttpUtilities)

# External library linkage (needs target defined)
LINK_POCO()
LINK_XMLRPCEPI()

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)



