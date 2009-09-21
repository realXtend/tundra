# Define target name and output directory
init_target (RexCBLogin)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES *.ui)
file (GLOB RESOURCE_FILES resource/*.qrc)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
#include_directories (
#	../Core
#	../Foundation
#	../SceneModule
#	../Interfaces
#	../RexCommon
#)

# External library includes
#INCLUDE_BOOST()
INCLUDE_QT4()

#Wrap 
include(${QT_USE_FILE})
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

# Define static  library
if (NOT MSVC)
	SET (LIBRARY_OUTPUT_PATH ${TARGET_DIR})
endif (NOT MSVC)

add_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

# Because this is static library, compile it with -fPIC in Linux.

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()


set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)
set_target_properties (${TARGET_NAME} PROPERTIES PREFIX "")
set_target_properties (${TARGET_NAME} PROPERTIES LINK_INTERFACE_LIBRARIES "")

# Define libraries to be linked into target
#target_link_libraries (${TARGET_NAME} Foundation Interfaces RexCommon )

# External library linkage
#LINK_BOOST()
LINK_QT4()

# Copy XML files from source directory to target directory
#foreach (XML_FILE ${XML_FILES})
#	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
#endforeach (XML_FILE)

