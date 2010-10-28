# Define target name and output directory
init_target (TextureDecoderModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB H_MOC_FILES TextureCache.h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${H_MOC_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

add_definitions (-DTEXTURE_DECODER_MODULE_EXPORTS)

use_package (BOOST)
use_package (POCO)
use_package (QT4)
use_package (OPENJPEG)
use_modules (Core Foundation Interfaces RexCommon AssetModule Asset)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_modules (Core Foundation Interfaces RexCommon Asset)
link_package (BOOST)
link_package (POCO)
link_package (QT4)
link_package (OPENJPEG)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()