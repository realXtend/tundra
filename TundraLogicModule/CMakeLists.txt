# Define target name and output directory
init_target (TundraLogicModule OUTPUT plugins)

GetEntityComponents()

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (XML_FILES TundraLogicModule.xml)
set (MOC_FILES TundraLogicModule.h SyncManager.h Server.h Client.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

add_definitions (-DTUNDRALOGIC_MODULE_EXPORTS)

use_package_bullet()
use_modules (Framework Asset Scene AssetModule KristalliProtocolModule Console
    OgreRenderingModule PhysicsModule EntityComponents/EC_DynamicComponent)

# This is only needed because this module bootstraps EC_Sound. If EC_Sound is removed to someplace else, this can be removed.
use_modules(Audio)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

LinkEntityComponent(EntityComponents/EC_HoveringText EC_HoveringText)
LinkEntityComponent(EntityComponents/EC_Clone EC_Clone)
LinkEntityComponent(EntityComponents/EC_ChatBubble EC_ChatBubble)
LinkEntityComponent(EntityComponents/EC_Ruler EC_Ruler)
LinkEntityComponent(EntityComponents/EC_Gizmo EC_Gizmo)
LinkEntityComponent(EntityComponents/EC_Selected EC_Selected)

LinkEntityComponent(EntityComponents/EC_Highlight EC_Highlight)
LinkEntityComponent(EntityComponents/EC_InputMapper EC_InputMapper)
LinkEntityComponent(EntityComponents/EC_Sound EC_Sound)
LinkEntityComponent(EntityComponents/EC_ParticleSystem EC_ParticleSystem)
LinkEntityComponent(EntityComponents/EC_3DCanvas EC_3DCanvas)
LinkEntityComponent(EntityComponents/EC_3DCanvasSource EC_3DCanvasSource)
LinkEntityComponent(EntityComponents/EC_Touchable EC_Touchable)
LinkEntityComponent(EntityComponents/EC_VideoSource EC_VideoSource)
LinkEntityComponent(EntityComponents/EC_PlanarMirror EC_PlanarMirror)
LinkEntityComponent(EntityComponents/EC_ProximityTrigger EC_ProximityTrigger)

link_modules (Framework Asset Scene AssetModule KristalliProtocolModule
    OgreRenderingModule PhysicsModule EC_DynamicComponent Console)

# This is only needed because this module bootstraps EC_Sound. If EC_Sound is removed to someplace else, this can be removed.
link_modules(Audio)

link_ogre()


SetupCompileFlagsWithPCH()


if (WIN32)
    target_link_libraries (${TARGET_NAME} ws2_32.lib)
endif()

final_target ()
