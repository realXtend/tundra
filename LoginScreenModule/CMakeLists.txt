# Define target name and output directory
init_target (LoginScreenModule OUTPUT modules/core)

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)
file (GLOB MOC_FILES LoginScreenModule.h LoginWidget.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})

use_modules (Framework Input Ui)

# To enable Taiga login using this module, comment out above and uncomment the following.
# add_definitions(-DENABLE_TAIGA_LOGIN)
# use_modules (Framework Input Ui)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

link_modules (Framework Input)
link_ogre()

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
