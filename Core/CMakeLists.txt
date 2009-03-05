# Define target name
set (TARGET_NAME Core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../Core
	../external_libs/include
)

# Library directories
link_directories (
	../external_libs
)

# Define static library
add_library (${TARGET_NAME} STATIC ${SOURCE_FILES})
set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)

# Precompiled headers on MSVC
if (MSVC)
	foreach( src_file ${CPP_FILES} )
        	set_source_files_properties(
            		${src_file}
            		PROPERTIES
            		COMPILE_FLAGS "/YuCoreStableHeaders.h"
        	)
	endforeach( src_file ${CPP_FILES} )

	set_source_files_properties(CoreStableHeaders.cpp
        	PROPERTIES
        	COMPILE_FLAGS "/YcCoreStableHeaders.h"
        )
endif (MSVC)
