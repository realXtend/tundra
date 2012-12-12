# Define target name and output directory
init_target (PhysicsModule OUTPUT plugins)

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (XML_FILES PhysicsModule.xml)
set (MOC_FILES PhysicsModule.h PhysicsWorld.h EC_RigidBody.h EC_VolumeTrigger.h EC_PhysicsMotor.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

add_definitions (-DPHYSICS_MODULE_EXPORTS)

use_package_bullet()
UseTundraCore()
use_core_modules(TundraCore Math OgreRenderingModule EnvironmentModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

link_package(QT4)
link_ogre()
link_package_bullet()
link_modules(TundraCore Math OgreRenderingModule EnvironmentModule)

SetupCompileFlagsWithPCH()

if (WIN32)
    target_link_libraries (${TARGET_NAME} ws2_32.lib)
endif()

# Xcode hack: For a very unknown reason, link_package_bullet() does not cause Xcode to add Bullet libs to PhysicsModule linked libs list.
# So, add it manually. TODO: Figure this out and remove the following lines. link_package_bullet() should do the job for Xcode as well.
if (XCODE_VERSION)
    target_link_libraries(${TARGET_NAME} LinearMath BulletDynamics BulletCollision)
endif()

final_target ()
