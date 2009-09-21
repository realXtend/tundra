# Define target name and output directory
init_target (RexLogicModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES RexLoginWindow.h RexWebLogin.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DUSE_QT)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} )
use_framework_modules (${TARGET_NAME} Core Foundation Interfaces SceneManager)
use_module_headers (${TARGET_NAME} OgreRenderingModule OpenSimProtocol RexCommon InputModuleOIS QtModule HttpUtilities)
use_package (BOOST)
use_package (POCO)
use_package (OGRE)
use_package (QT4)

INCLUDE_CURL()
INCLUDE_OPENJPEG()

target_link_libraries (${TARGET_NAME} OpenSimProtocolModule RexCommon OgreRenderingModule QtModule HttpUtilities)
LINK_CURL()
LINK_OPENJPEG()
link_ois ()
link_ogre ()

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
    add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

# Precompiled headers on MSVC
if (MSVC)
    foreach( src_file ${CPP_FILES} )
            set_source_files_properties( ${src_file}
                    PROPERTIES
                    COMPILE_FLAGS "/YuStableHeaders.h")
    endforeach( src_file ${CPP_FILES} )

    set_source_files_properties(StableHeaders.cpp
            PROPERTIES
            COMPILE_FLAGS "/YcStableHeaders.h")
endif (MSVC)
