# Define target name and output directory
init_target (SceneWidgetComponents OUTPUT plugins)

MocFolder ()

# Define source files
file (GLOB CPP_FILES dllmain.cpp EC_WebView.cpp EC_WidgetCanvas.cpp) # omitting EC_VideoView.cpp for now, add when ported
file (GLOB H_FILES EC_WebView.h EC_WidgetCanvas.h SceneWidgetComponentsApi.h) # omitting EC_VideoView.h for now, add when ported
file (GLOB MOC_FILES ${H_FILES})

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

add_definitions (-DSCENEWIDGET_MODULE_EXPORTS)

use_core_modules (Framework Ui Input Scene Asset OgreRenderingModule TundraProtocolModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_ogre()
link_modules (Framework Ui Input Scene Asset OgreRenderingModule TundraProtocolModule)

SetupCompileFlags()

final_target ()
