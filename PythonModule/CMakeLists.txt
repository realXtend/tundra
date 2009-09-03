# Define target name and output directory
set (TARGET_NAME PythonScriptModule)
set (TARGET_DIR ${PROJECT_BINARY_DIR}/bin/modules/core)
set (PYTHON_FORCE_RELEASE_VERSION 0) # set if you want to use release version of python when building debug version of viewer
#doesn't work anymore now that PythonQt was introduced - attempts to build viewer in debugmode with release mode pythonqt have failed so far

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../Core
	../Foundation
	../SceneManager
	../Interfaces
    ../RexCommon
    ../ReXLogic
    ../OpenSimProtocol #needed 'cause ReXLogic uses it
    ../OgreRenderingModule #to be able to get placeable component data
    ../InputModuleOIS
    ../QtModule #for UICanvas creation (in PythonQt part of the api)
)

link_directories (${PROJECT_BINARY_DIR}/RexLogic/Debug ${PROJECT_BINARY_DIR}/RexLogic/Release)

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()
INCLUDE_PYTHON()
INCLUDE_PYTHONQT()
INCLUDE_QT4()
INCLUDE_OIS()
INCLUDE_OGRE()


add_definitions (-DMODULE_EXPORTS)
if(PYTHON_FORCE_RELEASE_VERSION)
    add_definitions(-DPYTHON_FORCE_RELEASE_VERSION) 
endif(PYTHON_FORCE_RELEASE_VERSION)

# Define dynamic library
if (NOT MSVC)
	SET (LIBRARY_OUTPUT_PATH ${TARGET_DIR})
endif (NOT MSVC)

add_library (${TARGET_NAME} SHARED ${SOURCE_FILES})
set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)
set_target_properties (${TARGET_NAME} PROPERTIES PREFIX "")
set_target_properties (${TARGET_NAME} PROPERTIES LINK_INTERFACE_LIBRARIES "")

if (MSVC)
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${TARGET_DIR})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TargetPath)\" ${TARGET_DIR})
endif (MSVC)

# Define libraries to be linked into target
target_link_libraries (${TARGET_NAME} Core Foundation Interfaces OpenSimProtocolModule RexCommon RexLogicModule) # OgreRenderingModule)

# External library linkage
LINK_BOOST()
LINK_POCO()
LINK_PYTHON()
#LINK_OGRE() #was direct camera access
LINK_PYTHONQT()
LINK_QT4()

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)
