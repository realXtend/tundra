# Define target name and output directory
init_target (LegacyAvatarModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

use_package (BOOST)
use_package (POCO)
use_modules (Core Foundation Interfaces SceneManager )

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${UI_FILES} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

link_modules (Core Foundation Interfaces SceneManager )
link_package (BOOST)
link_package (POCO)

SetupCompileFlags()
CopyModuleXMLFile()

final_target ()



