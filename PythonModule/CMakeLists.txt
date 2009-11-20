# Define target name and output directory
init_target (PythonScriptModule OUTPUT modules/core)

set (PYTHON_FORCE_RELEASE_VERSION 0) # set if you want to use release version of python when building debug version of viewer
#doesn't work anymore now that PythonQt was introduced - attempts to build viewer in debugmode with release mode pythonqt have failed so far

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES QtModuleWrapper.h Vector3Wrapper.h QuaternionWrapper.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})


if(PYTHON_FORCE_RELEASE_VERSION)
    add_definitions(-DPYTHON_FORCE_RELEASE_VERSION) 
endif(PYTHON_FORCE_RELEASE_VERSION)

use_package (BOOST)
use_package (POCO)
use_package (QT4)
use_package (PYTHON)
use_package (PYTHON_QT)
use_package (OIS)
use_package (OGRE)
use_package(PROPERTYEDITOR)
use_modules (RexCommon RexLogicModule ProtocolUtilities OgreRenderingModule InputModuleOIS QtModule ProtocolModuleOpenSim ProtocolModuleTaiga)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})
use_framework_modules (Core Foundation Interfaces SceneManager)

link_directories (${PROJECT_BINARY_DIR}/RexLogicModule/Debug ${PROJECT_BINARY_DIR}/RexLogicModule/Release)
link_modules (RexCommon RexLogicModule ProtocolUtilities QtModule) # OgreRenderingModule)
link_package (BOOST)
link_package (POCO)
link_package (QT4)
link_package (PYTHON)
link_package (PYTHON_QT)
link_package (OIS)
link_package (OGRE)
link_package(PROPERTYEDITOR)


# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
    add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

final_target ()
