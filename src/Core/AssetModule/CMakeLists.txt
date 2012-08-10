# Define target name and output directory
init_target(AssetModule OUTPUT plugins)

MocFolder()

# Define source files
file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB H_MOC_FILES AssetCache.h LocalAssetStorage.h LocalAssetProvider.h
    HttpAssetProvider.h HttpAssetStorage.h HttpAssetTransfer.h AssetModule.h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

QT4_WRAP_CPP(MOC_SRCS ${H_MOC_FILES})

add_definitions (-DASSET_MODULE_EXPORTS)
set(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

UseTundraCore()
use_core_modules(TundraCore Math TundraProtocolModule)

build_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_modules(TundraCore Math TundraProtocolModule)

link_package_knet()
if (APPLE)
    link_ogre()
endif ()

SetupCompileFlagsWithPCH()

if (WIN32)
    target_link_libraries (${TARGET_NAME} ws2_32.lib)
endif()

final_target()
