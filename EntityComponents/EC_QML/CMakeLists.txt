# Define target name and output directory
init_target (EC_QML)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES EC_QML.h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})
#set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_modules (Core Foundation Interfaces Ui Input Scene Asset OgreRenderingModule TundraLogicModule RexCommon)
use_package_qtdeclarative()

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS} )

LinkEntityComponent(EntityComponents/EC_3DCanvas EC_3DCanvas)
LinkEntityComponent(EntityComponents/EC_Mesh EC_Mesh)

link_ogre()
link_package_qtdeclarative()
link_modules (Core Foundation Interfaces Ui Input Scene Asset OgreRenderingModule TundraLogicModule RexCommon)

SetupCompileFlagsWithPCH()

final_target ()
