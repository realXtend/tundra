# Define target name and output directory
init_target(Audio)
# Use the output that only if we are building Audio project as a shared library.
#init_target(Audio OUTPUT ./)

# Define source files
file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB MOC_FILES AudioAsset.h SoundChannel.h SoundSettings.h AudioAPI.h)

set(SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_modules(Framework Asset)

add_definitions(-DAUDIO_API_EXPORTS)
# Define AUDIO_API_DLL if we are building the Audio project as a shared library.
#add_definitions(-DAUDIO_API_DLL)

use_package(OPENAL)
use_package(OGG)
use_package(VORBIS)

build_library(${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})
# Use the SHARED option if we are building Audio project as a shared library.
#build_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_package(BOOST)
link_package(QT4)
link_package(OPENAL)
link_package(OGG)
link_package(VORBIS)
link_modules(Framework Asset)

SetupCompileFlags()

final_target()
