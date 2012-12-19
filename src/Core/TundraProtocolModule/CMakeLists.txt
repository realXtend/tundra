# Define target name and output directory
init_target (TundraProtocolModule OUTPUT plugins)

GetEntityComponents()

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (MOC_FILES TundraLogicModule.h SyncManager.h SyncState.h Server.h Client.h KristalliProtocolModule.h UserConnection.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

add_definitions(-DTUNDRAPROTOCOL_MODULE_EXPORTS)
add_definitions(-D_WINSOCKAPI_)

# Includes
use_package_bullet()
UseTundraCore()
use_core_modules(TundraCore Math OgreRenderingModule PhysicsModule)

# This is only needed because this module bootstraps EC_Sound. If EC_Sound is removed to someplace else, this can be removed.
#use_core_modules (Audio)

# This is only needed because this module bootstraps EC_InputMapper. If this is done by some other means, this dependency can be removed.
#use_core_modules (Input)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

# Linking
link_package(QT4)
link_package_knet()
link_ogre()
link_modules(TundraCore Math OgreRenderingModule PhysicsModule)
link_entity_components(EC_HoveringText EC_TransformGizmo EC_Highlight EC_LaserPointer
    EC_Sound EC_PlanarMirror EC_ProximityTrigger)

# This is only needed because this module bootstraps EC_Sound. If EC_Sound is removed to someplace else, this can be removed.
#link_modules (Audio)

# This is only needed because this module bootstraps EC_InputMapper. If this is done by some other means, this dependency can be removed.
#link_modules (Input)

SetupCompileFlagsWithPCH()

if (WIN32)
    target_link_libraries (${TARGET_NAME} ws2_32.lib)
endif()

final_target ()
