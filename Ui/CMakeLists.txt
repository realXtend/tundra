# Define target name and output directory
init_target (Ui)
#init_target (Ui OUTPUT ./)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES NaaliUi.h NaaliGraphicsView.h NaaliMainWindow.h UiServiceInterface.h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_package (BOOST)
use_package (POCO)
#use_package (OGRE)
use_package (QT4)
use_modules (Core Foundation RexCommon Interfaces)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})

add_definitions (-DUI_API_EXPORTS)
# If defined, we are building the Ui project as a DLL.
# add_definitions (-DUI_API_DLL)

link_package (BOOST)
link_package (POCO)
#link_ogre()
link_package (QT4)
link_modules (Core Foundation)

SetupCompileFlags()

final_target ()
