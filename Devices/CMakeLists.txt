# Define target name and output directory
init_target (Devices)

# Use this if building Devices as a SHARED lib.
#init_target (Input OUTPUT ./)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES DevicesAPI.h IDevice.h IPositionalDevice.h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_modules (Core Foundation RexCommon Interfaces)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})

link_package (BOOST)
link_package (POCO)
link_package (QT4)

link_modules (Core)

SetupCompileFlags()

final_target ()
