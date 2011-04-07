# Define target name and output directory
init_target (KristalliProtocolModule OUTPUT plugins)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)

    
set (MOC_FILES UserConnection.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

add_definitions (-DKRISTALLIPROTOCOL_MODULE_EXPORTS)
add_definitions (-D_WINSOCKAPI_)
add_definitions (-DNAALI_CLIENT)

use_modules (Framework Scene)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_modules (Framework Scene)

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

if (WIN32)
target_link_libraries (${TARGET_NAME} ws2_32.lib)
endif()

final_target ()
