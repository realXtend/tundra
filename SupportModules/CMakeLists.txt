# Define target name and output directory
init_target (SupportModules OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

add_definitions (-DCONSOLE_MODULE_EXPORTS)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES})
use_framework_modules (${TARGET_NAME} Core Foundation Interfaces SceneManager)
use_module_headers (${TARGET_NAME} OgreRenderingModule InputModuleOIS)
use_package (BOOST)
use_package (POCO)
use_package (OIS)
use_package (OGRE)

target_link_libraries (${TARGET_NAME} OgreRenderingModule)
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
