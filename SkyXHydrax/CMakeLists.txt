# Define target name
init_target (SkyXHydrax OUTPUT plugins)

# Define source files
if (SKYX)
    file (GLOB CPP_FILES dllmain.cpp EC_SkyX.cpp)
    file (GLOB H_FILES EC_SkyX.h)
endif(SKYX)
if (HYDRAX)
    file (GLOB CPP_FILES dllmain.cpp EC_Hydrax.cpp)
    file (GLOB H_FILES EC_Hydrax.h)
endif(HYDRAX)
if (SKYX AND HYDRAX)
    file (GLOB CPP_FILES dllmain.cpp EC_SkyX.h EC_Hydrax.cpp)
    file (GLOB H_FILES EC_SkyX.h EC_Hydrax.h)
endif(SKYX AND HYDRAX)

file (GLOB MOC_FILES ${H_FILES})
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

if (SKYX)
    add_definitions(-DSKYX_ENABLED)
    use_package (SKYX)
endif(SKYX)
if (HYDRAX)
    add_definitions(-DHYDRAX_ENABLED)
    use_package (HYDRAX)
endif(HYDRAX)

use_modules(Framework Scene OgreRenderingModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_modules(Framework Scene OgreRenderingModule)

link_ogre()

if (SKYX)
    link_package(SKYX)
endif(SKYX)
if (HYDRAX)
    link_package (HYDRAX)
endif(HYDRAX)

SetupCompileFlags()

final_target ()
