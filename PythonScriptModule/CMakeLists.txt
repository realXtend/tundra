# Define target name and output directory
init_target (PythonScriptModule OUTPUT modules/core)

set (PYTHON_FORCE_RELEASE_VERSION 0) # set if you want to use release version of python when building debug version of viewer
#doesn't work anymore now that PythonQt was introduced - attempts to build viewer in debugmode with release mode pythonqt have failed so far

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES PythonScriptModule.h PythonQtScriptingConsole.h Vector3dfDecorator.h TransformDecorator.h QuaternionDecorator.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

if(PYTHON_FORCE_RELEASE_VERSION)
    add_definitions(-DPYTHON_FORCE_RELEASE_VERSION) 
endif(PYTHON_FORCE_RELEASE_VERSION)

use_package (PYTHON)
use_package (PYTHON_QT)

use_modules (Framework Scene OgreRenderingModule AvatarModule KristalliProtocolModule
    EntityComponents/EC_DynamicComponent EntityComponents/EC_Ruler EntityComponents/EC_Touchable
    EntityComponents/EC_3DCanvas EntityComponents/EC_3DCanvasSource EntityComponents/EC_Script Input Ui CommunicationsModule Audio)

if (ENABLE_TAIGA_SUPPORT)
   use_modules (RexCommon RexLogicModule ProtocolUtilities ProtocolModuleOpenSim ProtocolModuleTaiga 
      EntityComponents/EC_OpenSimPresence EntityComponents/EC_OpenSimPrim)
endif()

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_modules (Framework Scene OgreRenderingModule KristalliProtocolModule
    EC_Touchable EC_DynamicComponent EC_Ruler EC_3DCanvas EC_3DCanvasSource EC_Script Input Ui Audio)

if (ENABLE_TAIGA_SUPPORT)
   link_modules (RexCommon RexLogicModule 
       ProtocolUtilities EC_OpenSimPresence EC_OpenSimPrim)
endif()

link_package (PYTHON)
link_package (PYTHON_QT)
link_ogre()

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
