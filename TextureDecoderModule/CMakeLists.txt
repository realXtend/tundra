# Define target name and output directory
init_target (TextureDecoderModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
use_module_headers (${TARGET_NAME} RexCommon AssetModule)

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()
INCLUDE_OPENJPEG()

add_definitions (-DTEXTURE_DECODER_MODULE_EXPORTS)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES})
use_framework_modules (${TARGET_NAME} Core Foundation Interfaces)
target_link_libraries (${TARGET_NAME} RexCommon)

# External library linkage
LINK_BOOST()
LINK_POCO()
LINK_OPENJPEG()

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
