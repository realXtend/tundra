# Define target name and output directory
init_target(ScenePersistenceModule OUTPUT plugins)

GetEntityComponents()

# Define source files
file(GLOB CPP_FILES *.cpp *.c)
file(GLOB H_FILES *.h)
file(GLOB XML_FILES *.xml)
file(GLOB MOC_FILES ScenePersistenceModule.h)
set(SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions(-DSCENEPERSISTENCE_MODULE_EXPORTS) 

use_modules(Core Foundation Interfaces SceneManager RexCommon)

build_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_ogre()

link_modules(Core Foundation Interfaces SceneManager RexCommon)

SetupCompileFlags()


final_target()
