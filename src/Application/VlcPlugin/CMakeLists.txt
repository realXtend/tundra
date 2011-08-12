
macro(configure_vlc)
    sagase_configure_package(VLC
        NAMES vlc
        COMPONENTS libvlc libvlccore
        PREFIXES ${ENV_TUNDRA_DEP_PATH})
    sagase_configure_report (VLC)
endmacro()

# Do some checks, VC10 is out of the question as building from 
# sources is impossible on with VC tools (said by the vlc project).
# And they only ship VC9 binaries, and even then you need to create the .lib files by hand
#
# Linux and mac are doable but code changes are propably needed.

if (UNIX OR APPLE)
    set (SHOULD_RUN 1)
else (WIN32 AND VC90)
    set (SHOULD_RUN 1)
else ()
    set (SHOULD_RUN 0)
endif ()

if (SHOULD_RUN)

    configure_vlc()

    init_target (VlcPlugin OUTPUT plugins)

    MocFolder ()
    UiFolder ()

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

    # Install vlcplugins to install prefix for windows
    if (WIN32)
        setup_install_directory (${TUNDRA_BIN}/vlcplugins)
        setup_install_windows_deps ("libvlc.dll;libvlccore.dll" Release|RelWithDebInfo|Debug)
    endif ()

else ()
    message (WARNING "VlcPlugin can be built on linux, mac and windows with Visual Studio 2008. ")
    message (WARNING "Seems you are not meeting these requirements! See notes from VlcPlugin/CMakeLists.txt")
endif ()
