# Define target name and output directory
init_target(Audio)
# Use the output that only if we are building Audio project as a shared library.
#init_target(Audio OUTPUT ./)

MocFolder ()

# Define source files
file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB MOC_FILES AudioAsset.h SoundChannel.h SoundSettings.h AudioAPI.h)

set(SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_core_modules(Framework Math Asset)

add_definitions(-DAUDIO_API_EXPORTS)
# Define AUDIO_API_DLL if we are building the Audio project as a shared library.
#add_definitions(-DAUDIO_API_DLL)

if (NOT ANDROID)
    use_package(OPENAL)
endif()
use_package_ogg()
use_package_vorbis()

build_library(${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})
# Use the SHARED option if we are building Audio project as a shared library.
#build_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_package(QT4)
if (NOT ANDROID)
    link_package(OPENAL)
    link_package_ogg()
    link_package_vorbis()
endif()
link_modules(Framework Math Asset)

SetupCompileFlags()

final_target()
