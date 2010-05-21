# Define target name and output directory
init_target (PythonScriptModule OUTPUT modules/core)

set (PYTHON_FORCE_RELEASE_VERSION 0) # set if you want to use release version of python when building debug version of viewer
#doesn't work anymore now that PythonQt was introduced - attempts to build viewer in debugmode with release mode pythonqt have failed so far

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES Vector3Wrapper.h QuaternionWrapper.h QEC_Prim.h NaaliQtWrapper.h)
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
use_package (PROPERTYEDITOR)
use_modules (Core Foundation Interfaces SceneManager OgreRenderingModule RexCommon RexLogicModule
    ProtocolUtilities UiModule ProtocolModuleOpenSim ProtocolModuleTaiga EntityComponents/EC_Highlight
    EntityComponents/EC_OpenSimPresence EntityComponents/EC_OpenSimPrim EntityComponents/EC_3DCanvas EntityComponents/EC_3DCanvasSource EntityComponents/EC_DynamicComponent)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_directories (${PROJECT_BINARY_DIR}/RexLogicModule/Debug ${PROJECT_BINARY_DIR}/RexLogicModule/Release)
link_modules (Core Foundation Interfaces SceneManager OgreRenderingModule RexCommon RexLogicModule EC_Highlight
    ProtocolUtilities UiModule EC_OpenSimPresence EC_OpenSimPrim EC_3DCanvas EC_3DCanvasSource EC_DynamicComponent) # OgreRenderingModule)
link_package (BOOST)
link_package (POCO)
link_package (QT4)
link_package (PYTHON)
link_package (PYTHON_QT)
link_package (OGRE)
link_package (CURL)
link_package (XMLRPC)
link_package (PROPERTYEDITOR)

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
    add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

# MSVC -specific settings for preprocessor and PCH use
if (MSVC)
    # Label StableHeaders.cpp to create the PCH file and mark all other .cpp files to use that PCH file.
    # Add a #define DEBUG_CPP_NAME "this compilation unit name" to each compilation unit to aid in memory leak checking.
    foreach(src_file ${CPP_FILES})
            if (${src_file} MATCHES "StableHeaders.cpp$")
                set_source_files_properties(${src_file} PROPERTIES COMPILE_FLAGS "/YcStableHeaders.h")        
            else()
                get_filename_component(basename ${src_file} NAME)
                set_source_files_properties(${src_file} PROPERTIES COMPILE_FLAGS "/YuStableHeaders.h -DDEBUG_CPP_NAME=\"\\\"${basename}\"\\\"")
            endif()
    endforeach(src_file ${CPP_FILES})
endif (MSVC)

final_target ()
