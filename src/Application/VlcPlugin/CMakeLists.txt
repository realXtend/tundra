
macro(configure_vlc)
    sagase_configure_package(VLC
        NAMES vlc
        COMPONENTS libvlc libvlccore
        PREFIXES ${ENV_TUNDRA_DEP_PATH})
    sagase_configure_report (VLC)
endmacro()

configure_vlc()

############################################################################

init_target (VlcPlugin OUTPUT plugins)

file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB UI_FILES ui/*.ui)
file (GLOB RESOURCE_FILES ui/*.qrc)
set  (MOC_H_FILES VlcPlugin.h VlcMediaPlayer.h VlcVideoWidget.h PlayerStatus.h EC_MediaPlayer.h)

QT4_WRAP_CPP(MOC_FILES ${MOC_H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

use_core_modules (Framework Asset Ui OgreRenderingModule)
use_app_modules (SceneWidgetComponents)
use_package (VLC)

build_library (${TARGET_NAME} SHARED ${CPP_FILES} ${H_FILES} ${MOC_FILES} ${UI_SRCS} ${RESOURCE_SRCS})

SetupCompileFlags()

link_modules (Framework Asset Ui OgreRenderingModule SceneWidgetComponents)
link_package (VLC)

final_target ()

MocFolder ()
UiFolder ()