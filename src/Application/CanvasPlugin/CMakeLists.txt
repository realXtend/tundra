# Define the name of this plugin.
init_target(CanvasPlugin OUTPUT plugins)

# Define the source files for this plugin.
file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES EC_GraphicsViewCanvas.h)

# Make Qt run the MOC (Meta-object compiler) on all header files to produce its .cxx files where necessary.
file(GLOB MOC_FILES ${H_FILES})
set(SOURCE_FILES ${CPP_FILES} ${H_FILES})
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

# Enable Qt translation support for our plugin.
set(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

# List the cmake targets we depend on here. (adds include directories to the project)
UseTundraCore()
use_core_modules(TundraCore Math OgreRenderingModule)

# Tell cmake to generate a build output as a shared library.
build_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

# List the the cmake targets we need to link against here. (adds library link options to the project)
link_package(QT4)
link_ogre()
link_modules(TundraCore Math OgreRenderingModule)

# Pull Tundra-related compilation flags into this project (currently enables only DEBUG_CPP_NAME define, used for memory leak tracking).
SetupCompileFlags()

# Post-build step: copy output to /bin/plugins.
final_target()
