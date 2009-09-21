# Define target name and output directory
init_target (NetTestLogicModule OUTPUT modules/test)

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
	../OpenSimProtocol
	../ReXLogic
	../gtkmmUI
	../RexCommon
)

# Library directories
#link_directories (
	#../OpenSimProtocol/Debug
	#../OpenSimProtocol/Release
	#../ReXLogic/Debug
	#../ReXLogic/Release
#)

# Disable warning
if (MSVC)
	add_definitions (-D_CRT_SECURE_NO_WARNINGS)
endif (MSVC)



FIND_GTKMM()

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()
INCLUDE_GTKMM()

add_definitions (-DMODULE_EXPORTS)
#add_definitions (-NETTEST_MODULE_EXPORTS)

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
	target_link_libraries (${TARGET_NAME} ws2_32.lib iphlpapi.lib)
endif (MSVC)

# Define libraries to be linked into target
target_link_libraries (${TARGET_NAME} Core Interfaces Foundation OpenSimProtocolModule RexCommon RexLogicModule)

# External library linkage (needs target defined)
LINK_POCO()
LINK_GTKMM()

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)



