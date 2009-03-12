# Define target name and output directory
set (TARGET_NAME SceneModule)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../Core
	../Foundation
	../Interfaces
)
		
# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()

# Define static library
add_library (${TARGET_NAME} STATIC ${SOURCE_FILES})
set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)
set_target_properties (${TARGET_NAME} PROPERTIES PREFIX "")

# Define libraries to be linked into target
target_link_libraries (${TARGET_NAME} Foundation)

# External library linkage
LINK_BOOST()
LINK_POCO()

# Precompiled headers on MSVC
if (MSVC)
	foreach( src_file ${CPP_FILES} )
        	set_source_files_properties(
            		${src_file}
            		PROPERTIES
            		COMPILE_FLAGS "/YuStableHeaders.h"
        	)
	endforeach( src_file ${CPP_FILES} )

	set_source_files_properties(StableHeaders.cpp
        	PROPERTIES
        	COMPILE_FLAGS "/YcStableHeaders.h"
        )
endif (MSVC)
