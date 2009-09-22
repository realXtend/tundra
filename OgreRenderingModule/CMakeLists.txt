# Define target name and output directory
init_target (OgreRenderingModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

if (HYDRAX)
	INCLUDE_HYDRAX()
endif (HYDRAX)

if (CAELUM)
	add_definitions (-DCAELUM)
endif (CAELUM)

if (HYDRAX)
	add_definitions (-DHYDRAX)
endif (HYDRAX)

add_definitions (-DOGRE_MODULE_EXPORTS) 

use_package (BOOST)
use_package (POCO)
use_package (OGRE)
use_package (OIS)
use_modules (AssetModule SupportModules InputModuleOIS RexCommon)

if (CAELUM)
    use_package (CAELUM)
endif (CAELUM)

if (HYDRAX)
	use_package (HYDRAX)
endif (HYDRAX)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES})
use_framework_modules (Core Foundation Interfaces SceneManager)

link_modules (Foundation Interfaces RexCommon)
link_package (BOOST)
link_package (POCO)
link_package (OGRE)
link_package (OIS)
link_package (OGRE)

if (CAELUM)
    link_package (CAELUM)
endif (CAELUM)

if (HYDRAX)
	link_package (HYDRAX)
endif (HYDRAX)

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

# Precompiled headers on MSVC
if (MSVC)
	foreach( src_file ${CPP_FILES} )
        	set_source_files_properties(${src_file}
            		PROPERTIES
            		COMPILE_FLAGS "/YuStableHeaders.h")
	endforeach( src_file ${CPP_FILES} )

	set_source_files_properties(StableHeaders.cpp
        	PROPERTIES
        	COMPILE_FLAGS "/YcStableHeaders.h")
endif ()

copy_target ()
