# Define target name and output directory
init_target (PythonScriptModule OUTPUT modules/core)

set (PYTHON_FORCE_RELEASE_VERSION 0) # set if you want to use release version of python when building debug version of viewer
#doesn't work anymore now that PythonQt was introduced - attempts to build viewer in debugmode with release mode pythonqt have failed so far

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

if(PYTHON_FORCE_RELEASE_VERSION)
    add_definitions(-DPYTHON_FORCE_RELEASE_VERSION) 
endif(PYTHON_FORCE_RELEASE_VERSION)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES})
use_framework_modules (${TARGET_NAME} Core Foundation Interfaces SceneManager)
use_module_headers (${TARGET_NAME} RexCommon ReXLogic OpenSimProtocol OgreRenderingModule InputModuleOIS QtModule)
use_package (BOOST)
use_package (POCO)
use_package (QT4)
use_package (OIS)
use_package (OGRE)

INCLUDE_PYTHON()
INCLUDE_PYTHONQT()
LINK_PYTHON()
LINK_PYTHONQT()

link_directories (${PROJECT_BINARY_DIR}/RexLogic/Debug ${PROJECT_BINARY_DIR}/RexLogic/Release)
target_link_libraries (${TARGET_NAME} OpenSimProtocolModule RexCommon RexLogicModule) # OgreRenderingModule)
link_ois ()
link_ogre ()

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)
