# Define target name and output directory
init_target (PythonScriptModule OUTPUT modules/core)

set (PYTHON_FORCE_RELEASE_VERSION 0) # set if you want to use release version of python when building debug version of viewer
#doesn't work anymore now that PythonQt was introduced - attempts to build viewer in debugmode with release mode pythonqt have failed so far

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES PythonScriptModule.h PythonQtScriptingConsole.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

if(PYTHON_FORCE_RELEASE_VERSION)
    add_definitions(-DPYTHON_FORCE_RELEASE_VERSION) 
endif(PYTHON_FORCE_RELEASE_VERSION)

use_package (BOOST)
use_package (POCO)
use_package (QT4)
use_package (PYTHON)
use_package (PYTHON_QT)
use_package (OGRE)
use_package (CURL)
use_package (XMLRPC)

use_modules (Core Foundation Interfaces SceneManager OgreRenderingModule RexCommon RexLogicModule AvatarModule
    ProtocolUtilities ProtocolModuleOpenSim ProtocolModuleTaiga EntityComponents/EC_Highlight EntityComponents/EC_Touchable
    EntityComponents/EC_OpenSimPresence EntityComponents/EC_OpenSimPrim EntityComponents/EC_3DCanvas
    EntityComponents/EC_3DCanvasSource EntityComponents/EC_DynamicComponent EntityComponents/EC_SoundRuler
    EntityComponents/EC_Script EntityComponents/EC_NetworkPosition Input Ui)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_directories (${PROJECT_BINARY_DIR}/RexLogicModule/Debug ${PROJECT_BINARY_DIR}/RexLogicModule/Release)

link_modules (Core Foundation Interfaces SceneManager OgreRenderingModule RexCommon RexLogicModule AvatarModule EC_Highlight EC_Touchable
    ProtocolUtilities EC_OpenSimPresence EC_OpenSimPrim EC_3DCanvas EC_3DCanvasSource EC_DynamicComponent EC_SoundRuler
    EC_Script EC_NetworkPosition Input Ui)

link_package (BOOST)
link_package (POCO)
link_package (QT4)
link_package (PYTHON)
link_package (PYTHON_QT)
link_package (OGRE)
link_package (CURL)
link_package (XMLRPC)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
