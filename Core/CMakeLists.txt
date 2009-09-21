# Define target name
init_target (Core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES})
use_package (BOOST)
use_package (POCO)

# Precompiled headers on MSVC
if (MSVC)
	foreach (src_file ${CPP_FILES})
        	set_source_files_properties (${src_file} PROPERTIES COMPILE_FLAGS "/YuCoreStableHeaders.h")
	endforeach (src_file ${CPP_FILES})

	set_source_files_properties (CoreStableHeaders.cpp PROPERTIES COMPILE_FLAGS "/YcCoreStableHeaders.h")
endif (MSVC)
