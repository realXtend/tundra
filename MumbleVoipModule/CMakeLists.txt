configure_celt ()
configure_protobuf ()
configure_openssl ()

# Define target name and output directory
init_target (MumbleVoipModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h )
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)
file (GLOB RESOURCE_FILES resource/*.qrc)

# Qt4 Moc files to subgroup "CMake Moc"
# and ui_*.h generated .h files to "Generated UI" subgroup
MocFolder ()
UiFolder ()

set (SOURCE_FILES ${SOURCE_FILES} ${CPP_FILES} ${H_FILES})

# Qt4 Wrap 
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DMUMBLE_VOIP_MODULE_EXPORTS)

use_package_mumbleclient()
use_package (CELT)
use_package (PROTOBUF)
use_package (OPENSSL)

use_modules (Core Foundation Interfaces RexCommon ProtocolUtilities OgreRenderingModule Scene Console
    EntityComponents/EC_OpenSimPresence Audio TundraLogicModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

link_modules (Core Foundation Interfaces RexCommon ProtocolUtilities OgreRenderingModule Scene Console
    EC_OpenSimPresence Audio TundraLogicModule)
link_package_mumbleclient()
link_package (CELT)
link_package (PROTOBUF)
link_package (OPENSSL)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
