# Define target name and output directory
init_target (RexLogicModule OUTPUT modules/core)

GetEntityComponents()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES RexLogicModule.h EventHandlers/LoginHandler.h RexMovementInput.h
    EventHandlers/MainPanelHandler.h EntityComponent/EC_*.h Environment/Primitive.h Communications/*.h
    Communications/InWorldChat/*.h Camera/ObjectCameraController.h SceneInteract.h)

# SubFolders to project with filtering
AddSourceFolder (Avatar)
AddSourceFolder (EntityComponent)
AddSourceFolder (EventHandlers)
AddSourceFolder (Environment)
AddSourceFolder (Communications)
AddSourceFolder (Communications/InWorldChat)
AddSourceFolder (Camera)

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

add_definitions (-DREXLOGIC_MODULE_EXPORTS)

if (UISERVICE_TEST)
    add_definitions (-DUISERVICE_TEST)
endif ()

use_package (BOOST)
use_package (POCO)
use_package (OGRE)
use_package (QT4)
use_package (CURL)
use_package (OPENJPEG)
use_package (XMLRPC)

use_modules (RexLogicModule Core Foundation Interfaces SceneManager OgreRenderingModule AssetModule AvatarModule RexCommon
    HttpUtilities RpcUtilities ProtocolUtilities ProtocolModuleOpenSim ProtocolModuleTaiga EnvironmentModule)

if (NOT UISERVICE_TEST)
    use_modules (UiModule)
endif ()

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

LinkEntityComponent(EntityComponents/EC_HoveringText EC_HoveringText)
LinkEntityComponent(EntityComponents/EC_Clone EC_Clone)
LinkEntityComponent(EntityComponents/EC_OpenSimPresence EC_OpenSimPresence)
LinkEntityComponent(EntityComponents/EC_OpenSimPrim EC_OpenSimPrim)
LinkEntityComponent(EntityComponents/EC_ChatBubble EC_ChatBubble)
LinkEntityComponent(EntityComponents/EC_Ruler EC_Ruler)
LinkEntityComponent(EntityComponents/EC_SoundRuler EC_SoundRuler)
LinkEntityComponent(EntityComponents/EC_SoundListener EC_SoundListener)
LinkEntityComponent(EntityComponents/EC_HoveringWidget EC_HoveringWidget)
LinkEntityComponent(EntityComponents/EC_NetworkPosition EC_NetworkPosition)

LinkEntityComponent(EntityComponents/EC_Highlight EC_Highlight)
LinkEntityComponent(EntityComponents/EC_InputMapper EC_InputMapper)
LinkEntityComponent(EntityComponents/EC_Movable EC_Movable)
LinkEntityComponent(EntityComponents/EC_Mesh EC_Mesh)
LinkEntityComponent(EntityComponents/EC_Sound EC_Sound)
LinkEntityComponent(EntityComponents/EC_ParticleSystem EC_ParticleSystem)
LinkEntityComponent(EntityComponents/EC_3DCanvas EC_3DCanvas)
LinkEntityComponent(EntityComponents/EC_3DCanvasSource EC_3DCanvasSource)
LinkEntityComponent(EntityComponents/EC_Touchable EC_Touchable)
LinkEntityComponent(EntityComponents/EC_Light EC_Light)

link_modules (Core Foundation Interfaces SceneManager OgreRenderingModule AssetModule AvatarModule RexCommon
    HttpUtilities RpcUtilities ProtocolUtilities ProtocolModuleOpenSim ProtocolModuleTaiga EnvironmentModule)

if (NOT UISERVICE_TEST)
    link_modules (UiModule)
endif ()

link_package (OGRE)
link_package (BOOST)
link_package (POCO)
link_package (QT4)
link_package (CURL)
link_package (OPENJPEG)
link_package (XMLRPC)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
