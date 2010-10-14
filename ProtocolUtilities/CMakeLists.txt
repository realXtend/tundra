# Define target name and output directory
init_target (ProtocolUtilities)

GetEntityComponents()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES Interfaces/WorldSessionInterface.h WorldStream.h LoginCredentials.h OpenSim/GridInfoHelper.h)

# SubFolders to project with filtering
AddSourceFolder (Interfaces)
AddSourceFolder (OpenSim)
AddSourceFolder (RealXtend)
AddSourceFolder (Taiga)
AddSourceFolder (Inventory)
AddSourceFolder (NetworkMessages)

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

use_package (BOOST)
use_package (POCO)
use_package (QT4)
use_package (XMLRPC)
use_modules (Core Foundation ProtocolUtilities Interfaces RexCommon RpcUtilities
    ProtocolModuleOpenSim ProtocolModuleTaiga SceneManager)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS} )

link_package (BOOST)
link_package (POCO)
link_package (QT4)
link_modules (Core Foundation Interfaces RexCommon RpcUtilities SceneManager)

LinkEntityComponent(EntityComponents/EC_OpenSimPrim EC_OpenSimPrim)

if (MSVC)
    # Force linking to Winsock.
    target_link_libraries (${TARGET_NAME} ws2_32.lib iphlpapi.lib)
endif (MSVC)

SetupCompileFlagsWithPCH()

final_target ()
