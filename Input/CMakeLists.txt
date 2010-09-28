# Define target name and output directory
init_target (Input)

# Use this if building Input as a SHARED lib.
#init_target (Input OUTPUT ./)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES Input.h InputContext.h KeyEvent.h KeyEventSignal.h MouseEvent.h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_package (BOOST)
use_package (POCO)
#use_package (OGRE)
use_package (QT4)
use_modules (Core Foundation RexCommon Interfaces Ui)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})

add_definitions (-DINPUT_API_EXPORTS)

link_package (BOOST)
link_package (POCO)
#link_package (OGRE)
link_package (QT4)
link_modules (Core Ui)

# MSVC -specific settings for preprocessor use
# Add a #define DEBUG_CPP_NAME "this compilation unit name" to each compilation unit to aid in memory leak checking.
if (MSVC)
    foreach(src_file ${CPP_FILES})
        get_filename_component(basename ${src_file} NAME)
        set_source_files_properties(${src_file} PROPERTIES COMPILE_FLAGS "-DDEBUG_CPP_NAME=\"\\\"${basename}\"\\\"")
    endforeach(src_file ${CPP_FILES})
endif (MSVC)

final_target ()
