# Define target name
init_target (OpenAssetImport)

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES ${H_FILES})
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_core_modules (Framework Asset)
use_package_assimp()

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})

link_modules (Framework Asset)
link_ogre()
link_package_assimp()
link_package(QT4)

SetupCompileFlagsWithPCH()

final_target ()
