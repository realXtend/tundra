# Define target name and output directory
init_target (CommunicationModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES *.ui)
file (GLOB RESOURCE_FILES resource/*.qrc)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})


# Include directories
include_directories (
	../Core
	../Foundation
	../Interfaces
	../SceneManager
)

#FIND_QT4() 
FIND_TELEPATHY_QT4()
FIND_GSTREAMER()
FIND_TELEPATHY_FARSIGHT2()

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()
INCLUDE_QT4()
INCLUDE_TELEPATHY_QT4()
INCLUDE_GSTREAMER()
INCLUDE_TELEPATHY_FARSIGHT2()

#Wrap 
include(${QT_USE_FILE})
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})



add_definitions (-DCOMMS_MODULE_EXPORTS)

# Define dynamic library
if (NOT MSVC)
	SET (LIBRARY_OUTPUT_PATH ${TARGET_DIR})
endif (NOT MSVC)

add_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})
set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)
set_target_properties (${TARGET_NAME} PROPERTIES PREFIX "")
set_target_properties (${TARGET_NAME} PROPERTIES LINK_INTERFACE_LIBRARIES "")

target_link_libraries (${TARGET_NAME} Interfaces Foundation)

# External library linkage (needs target defined)
LINK_POCO()
LINK_QT4()
LINK_TELEPATHY_QT4()
LINK_GSTREAMER()
LINK_TELEPATHY_FARSIGHT2()

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

