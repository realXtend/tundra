# Define target name and output directory
init_target (PythonScriptModule OUTPUT plugins)

set (PYTHON_FORCE_RELEASE_VERSION 0) # set if you want to use release version of python when building debug version of viewer
#doesn't work anymore now that PythonQt was introduced - attempts to build viewer in debugmode with release mode pythonqt have failed so far

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES PythonScriptModule.h PythonQtScriptingConsole.h Vector3dfDecorator.h TransformDecorator.h QuaternionDecorator.h AssetReferenceDecorator.h)
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

# note: KristalliProtocolModule, CommunicationsModule are removed from tundra2 branch, need to modify this and code to get it compiled!
use_core_modules (Framework Scene OgreRenderingModule KristalliProtocolModule Input Ui Audio Console)
use_app_modules (AvatarModule CommunicationsModule)

# ENABLE_TAIGA_SUPPORT has been deprecated afaik, this should be removed!
if (ENABLE_TAIGA_SUPPORT)
   use_modules (RexCommon RexLogicModule ProtocolUtilities ProtocolModuleOpenSim ProtocolModuleTaiga 
                EntityComponents/EC_OpenSimPresence EntityComponents/EC_OpenSimPrim)
endif()

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_modules (Framework Scene OgreRenderingModule KristalliProtocolModule Input Ui Audio Console)

# note: Some of these components may not be in the default build of tundra2 branch, need to modify what this module needs at bare minimum.
# also EC_DynamicComponent is now under the Scene project, no need to add it to this list.
link_entity_components (EC_Ruler EC_Touchable EC_3DCanvas EC_3DCanvasSource EC_Script)

# ENABLE_TAIGA_SUPPORT has been deprecated afaik, this should be removed!
if (ENABLE_TAIGA_SUPPORT)
   link_modules (RexCommon RexLogicModule ProtocolUtilities EC_OpenSimPresence EC_OpenSimPrim)
endif()

link_package (PYTHON)
link_package (PYTHON_QT)
link_ogre()

SetupCompileFlagsWithPCH()

final_target ()

# Install files
setup_install_directory (${TUNDRA_BIN}/pymodules)

# Install windows deps
# Note: TEST THE BELOW REGEXP WHEN THIS MODULE BUILDS!
setup_install_windows_deps_regexp ("[Pp]ython_+.*[^d][.]dll$" Release|RelWithDebInfo "[Pp]ython_+.*[_][d][.]dll$" Debug)
