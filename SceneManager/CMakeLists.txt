# Define target name and output directory
init_target (SceneManager)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()

# Define static library

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES})
use_framework_modules (${TARGET_NAME} Core Foundation Interfaces SceneManager)

# Precompiled headers on MSVC
if (MSVC)
	foreach (src_file ${CPP_FILES})
        	set_source_files_properties (${src_file} PROPERTIES COMPILE_FLAGS "/YuStableHeaders.h")
	endforeach (src_file ${CPP_FILES})

	set_source_files_properties (StableHeaders.cpp PROPERTIES COMPILE_FLAGS "/YcStableHeaders.h")
endif (MSVC)

