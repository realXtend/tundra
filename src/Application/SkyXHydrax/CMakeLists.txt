# Define target name
init_target (SkyXHydrax OUTPUT plugins)

MocFolder ()

# Define source files
if (ENABLE_SKYX)
    file (GLOB CPP_FILES dllmain.cpp EC_SkyX.cpp)
    file (GLOB H_FILES EC_SkyX.h)
endif()
if (ENABLE_HYDRAX)
    file (GLOB CPP_FILES dllmain.cpp EC_Hydrax.cpp)
    file (GLOB H_FILES EC_Hydrax.h)
endif()
if (ENABLE_SKYX AND ENABLE_HYDRAX)
    file (GLOB CPP_FILES dllmain.cpp EC_SkyX.cpp EC_Hydrax.cpp)
    file (GLOB H_FILES EC_SkyX.h EC_Hydrax.h)
endif()

file (GLOB MOC_FILES ${H_FILES})
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

if (ENABLE_SKYX)
    add_definitions(-DSKYX_ENABLED)
    use_package (SKYX)
endif()
if (ENABLE_HYDRAX)
    add_definitions(-DHYDRAX_ENABLED)
    use_package (HYDRAX)
endif()

use_core_modules(Framework Scene OgreRenderingModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_modules(Framework Scene OgreRenderingModule)

link_ogre()

if (ENABLE_SKYX)
    link_package (SKYX)
endif()
if (ENABLE_HYDRAX)
    link_package (HYDRAX)
endif()

SetupCompileFlags()

final_target ()
