configure_openal ()
configure_ogg ()
configure_vorbis ()

# Define target name and output directory
init_target(Audio OUTPUT ./)

# Define source files
file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB MOC_FILES AudioAsset.h SoundChannel.h SoundSettings.h Audio.h)

set(SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_modules(Core Asset)

add_definitions(-DAUDIO_API_EXPORTS)
# If defined, we are building the Audio project as a DLL.
add_definitions(-DAUDIO_API_DLL)

use_package(OPENAL)
use_package(OGG)
use_package(VORBIS)

#build_library(${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})
build_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_package(BOOST)
link_package(POCO)
link_package(QT4)
link_package(OPENAL)
link_package(OGG)
link_package(VORBIS)
link_modules(Core Asset)

SetupCompileFlags()

final_target()
