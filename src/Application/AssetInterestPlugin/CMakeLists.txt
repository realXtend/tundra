# Define target name and output directory
init_target (AssetInterestPlugin OUTPUT plugins)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB UI_FILES ui/*.ui)
file (GLOB XML_FILES *.xml)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

MocFolder ()
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI (UI_SRCS ${UI_FILES})

# Includes
UseTundraCore()
use_core_modules(TundraCore Math OgreRenderingModule TundraProtocolModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

# Linking
link_ogre()
link_package_knet()
link_modules(TundraCore Math OgreRenderingModule TundraProtocolModule)

if (WIN32)
    target_link_libraries (${TARGET_NAME} ws2_32.lib)
endif()

SetupCompileFlags()

final_target ()
