
init_target (BrowserUiPlugin OUTPUT plugins)

file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
QT4_WRAP_CPP(MOC_FILES BrowserUiPlugin.h CookieJar.h)

UseTundraCore()
use_core_modules(TundraCore Math)
use_app_modules(JavascriptModule)

build_library (${TARGET_NAME} SHARED ${CPP_FILES} ${H_FILES} ${MOC_FILES})

SetupCompileFlags()

link_modules(TundraCore JavascriptModule)

link_package(QT4)
if (APPLE)
    link_ogre()
endif ()

final_target ()

# Install browserui under jsmodules.
# Data gets copied from the src dir to the install destination.
# If you want to run this plugin from /bin while devving do a manual copy.
#setup_install_directory (jsmodules)
