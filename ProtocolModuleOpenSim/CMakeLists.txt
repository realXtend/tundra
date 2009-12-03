# Define target name and output directory
init_target (ProtocolModuleOpenSim OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES )
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DOSPROTO_MODULE_EXPORTS -DUSE_QT)

use_package (BOOST)
use_package (POCO)
use_package (XMLRPC)
use_package (QT4)
use_package (CURL)
use_modules (Core Foundation Interfaces SceneManager RexCommon HttpUtilities RpcUtilities ProtocolUtilities)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} )

link_modules (Core Foundation Interfaces SceneManager RexCommon HttpUtilities RpcUtilities ProtocolUtilities)
link_package (BOOST)
link_package (POCO)
link_package (XMLRPC)
link_package (QT4)
link_package (CURL)

# MSVC -specific settings for preprocessor and PCH use
if (MSVC)
    # Add a #define DEBUG_CPP_NAME "this compilation unit name" to each compilation unit to aid in memory leak checking.
    foreach(src_file ${CPP_FILES})
        get_filename_component(basename ${src_file} NAME)
        set_source_files_properties(${src_file} PROPERTIES COMPILE_FLAGS "-DDEBUG_CPP_NAME=\"\\\"${basename}\"\\\"")
    endforeach(src_file ${CPP_FILES})
endif (MSVC)

final_target ()
