# Define target name and output directory
init_target (KristalliProtocolModule OUTPUT modules/core)

set(KRISTALLI_CORE_PATH "../Kristalli")

include_directories(${KRISTALLI_CORE_PATH}/include)
link_directories(${KRISTALLI_CORE_PATH}/build/win)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
#file (GLOB UI_FILES ui/*.ui)
#file (GLOB RESOURCE_FILES resource/*.qrc)
    
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
#QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
#QT4_WRAP_UI(UI_SRCS ${UI_FILES})
#QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DKRISTALLIPROTOCOL_MODULE_EXPORTS)
add_definitions (-D_WINSOCKAPI_)
add_definitions (-DNAALI_CLIENT)

use_package (BOOST)
#use_package (QT4)
use_package (POCO)
use_modules (RexCommon Core Foundation Interfaces)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES})
#build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

link_package (BOOST)
#link_package (QT4)
link_package (POCO)
link_modules (RexCommon Core Foundation Interfaces)

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

# Add a #define DEBUG_CPP_NAME "this compilation unit name" to each compilation unit to aid in memory leak checking.
if (MSVC)
    foreach(src_file ${CPP_FILES})
        set_source_files_properties(${src_file} PROPERTIES COMPILE_FLAGS "-DDEBUG_CPP_NAME=\"\\\"${basename}\"\\\"")
    endforeach(src_file ${CPP_FILES})
endif (MSVC)

target_link_libraries (${TARGET_NAME} Algorithm.lib Core.lib Network.lib Time.lib ws2_32.lib)

final_target ()
