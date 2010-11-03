# Define target name
init_target (OpenAssetImport)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

use_package (BOOST)
use_package (POCO)
use_package (OGRE)
use_modules (Core Foundation Interfaces SceneManager)
use_package_assimp()

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES})

link_modules (Core Foundation Interfaces SceneManager)
link_ogre()
link_package (BOOST)
link_package (POCO)
link_package_assimp()

SetupCompileFlagsWithPCH()

final_target ()
