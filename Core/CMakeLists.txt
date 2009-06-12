# Define target name
set (TARGET_NAME Core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../Core
)

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()

# Define static library

add_library (${TARGET_NAME} STATIC ${SOURCE_FILES})

# Because this is static library, compile it with -fPIC in Linux.

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()


set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)

# Precompiled headers on MSVC
if (MSVC)
	foreach (src_file ${CPP_FILES})
        	set_source_files_properties (${src_file} PROPERTIES COMPILE_FLAGS "/YuCoreStableHeaders.h")
	endforeach (src_file ${CPP_FILES})

	set_source_files_properties (CoreStableHeaders.cpp PROPERTIES COMPILE_FLAGS "/YcCoreStableHeaders.h")
endif (MSVC)

# External library linkage (needs target defined)

if (NOT MSVC)
  LINK_BOOST()
  LINK_POCO()
endif()