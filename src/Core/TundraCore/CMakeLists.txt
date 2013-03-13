# Define target name and output directory
if (TUNDRACORE_SHARED)
    init_target(TundraCore OUTPUT ./)
else()
    init_target(TundraCore)
endif()

MocFolder()

# Define source files
file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB MOC_FILES
    Asset/AssetAPI.h Asset/IAsset.h Asset/IAssetTransfer.h Asset/IAssetUploadTransfer.h
    Asset/IAssetStorage.h Asset/AssetRefListener.h Asset/BinaryAsset.h Asset/AssetCache.h
    Asset/IAssetBundle.h Asset/IAssetBundleTypeFactory.h
    Audio/AudioAPI.h Audio/AudioAsset.h Audio/SoundChannel.h Audio/SoundSettings.h
    Console/ConsoleAPI.h Console/ConsoleWidget.h Console/ShellInputThread.h
    Framework/Framework.h Framework/Application.h Framework/FrameAPI.h Framework/ConsoleAPI.h
    Framework/DebugAPI.h Framework/ConfigAPI.h Framework/IRenderer.h Framework/IModule.h
    Framework/PluginAPI.h Framework/VersionInfo.h Framework/Profiler.h
    Input/InputAPI.h Input/InputContext.h Input/KeyEvent.h Input/KeyEventSignal.h Input/MouseEvent.h
    Input/GestureEvent.h Input/EC_InputMapper.h
    Scene/SceneAPI.h Scene/Scene.h Scene/Entity.h Scene/IComponent.h Scene/EntityAction.h
    Scene/EC_Name.h Scene/EC_DynamicComponent.h Scene/AttributeChangeType.h Scene/ChangeRequest.h
    Ui/UiAPI.h Ui/UiGraphicsView.h Ui/UiMainWindow.h Ui/UiProxyWidget.h Ui/QtUiAsset.h Ui/RedirectedPaintWidget.h
)

AddSourceFolder(Asset)
AddSourceFolder(Audio)
AddSourceFolder(Console)
AddSourceFolder(Framework)
AddSourceFolder(Input)
AddSourceFolder(Scene)
AddSourceFolder(Ui)

set(SOURCE_FILES ${CPP_FILES} ${H_FILES})
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

UseTundraCore() # Include all subfolders within the project
use_core_modules(Math)

add_definitions(-DTUNDRACORE_EXPORTS)

if (TUNDRACORE_SHARED)
    build_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})
else()
    build_library(${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})
endif()

if (NOT TUNDRA_NO_BOOST)
    # Link Boost to Framework. All plugins link to framework, so Boost dependency will automatically propagate from here to each library.
    if (UNIX OR APPLE) # Visual Studio uses library auto-linking, so this is only necessary for the other platforms.
       target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})
    endif()
endif()

link_package(QT4)
link_package_knet()     # Scene
if (NOT ANDROID)
    link_package(OPENAL)    # Audio
    link_package_ogg()      # Audio
    link_package_vorbis()   # Audio
endif()
link_modules(Math)

# Add log library on Android
if (ANDROID)
    target_link_libraries (${TARGET_NAME} log)
endif()

SetupCompileFlagsWithPCH()

final_target()

# Install Audio deps for Windows
# todo: Verify if wrap_oal.dll is really needed for vc9/10. vs 2010 seems to run fine without the dll.
if (VS90)
    setup_install_windows_deps("wrap_oal.dll;OpenAL32.dll" Release|RelWithDebInfo|Debug)
elseif(VC100)
    setup_install_windows_deps("OpenAL32.dll" Release|RelWithDebInfo|Debug)
endif()
