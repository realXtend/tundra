# Define target name and output directory
init_target (Input)

# Use this if building Input as a SHARED lib.
#init_target (Input OUTPUT ./)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES InputAPI.h InputContext.h KeyEvent.h KeyEventSignal.h MouseEvent.h GestureEvent.h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

if (ENABLE_TAIGA_SUPPORT)
   use_modules (RexCommon)
endif()

use_modules (Framework Ui)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})

link_package (BOOST)
#link_package (POCO)
link_package (QT4)

link_modules (Framework Ui)

SetupCompileFlags()

final_target ()
