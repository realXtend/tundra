# Define target name and output directory
init_target (Scene)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES Entity.h Scene.h EC_Name.h EntityAction.h EC_Name.h EC_DynamicComponent.h IComponent.h AttributeChangeType.h SceneInteract.h SceneAPI.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_modules (Framework Asset Input)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})

link_modules (Framework Asset Input)

link_package (BOOST)
link_package (QT4)
link_package_knet()

SetupCompileFlagsWithPCH()

final_target ()
