# Define target name and output directory
init_target (TundraLogicModule OUTPUT modules/core)

#GetEntityComponents()

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
use_modules (Core Foundation Interfaces Asset Scene Console KristalliProtocolModule
    OgreRenderingModule PhysicsModule EntityComponents/EC_DynamicComponent)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

#LinkEntityComponent(EntityComponents/EC_DynamicComponent EC_DynamicComponent)

link_modules (Core Foundation Interfaces Asset Scene Console KristalliProtocolModule
    OgreRenderingModule PhysicsModule EC_DynamicComponent)
link_ogre()


SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

if (WIN32)
    target_link_libraries (${TARGET_NAME} ws2_32.lib)
endif()

final_target ()
