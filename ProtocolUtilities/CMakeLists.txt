macro (AddSourceFolder folder_name)
    file (GLOB H_FILES_IN_FOLDER_${folder_name} ${folder_name}/*.h)
    file (GLOB CPP_FILES_IN_FOLDER_${folder_name} ${folder_name}/*.cpp)
    source_group ("Header Files\\${folder_name}" FILES ${H_FILES_IN_FOLDER_${folder_name}})
    source_group ("Source Files\\${folder_name}" FILES ${CPP_FILES_IN_FOLDER_${folder_name}})
    set (H_FILES ${H_FILES} ${H_FILES_IN_FOLDER_${folder_name}})
    set (CPP_FILES ${CPP_FILES} ${CPP_FILES_IN_FOLDER_${folder_name}})
endmacro (AddSourceFolder)

macro (MocFolder)
    file (GLOB MOCS_TO_SOURCE_GROUP *.cxx */*.cxx)
    source_group ("CMake Moc" FILES ${MOCS_TO_SOURCE_GROUP})
endmacro (MocFolder)

#############################################################################################

# Define target name and output directory
init_target (ProtocolUtilities)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES Interfaces/WorldSessionInterface.h WorldStream.h)

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

use_package (BOOST)
use_package (POCO)
use_package (QT4)
use_package (XMLRPC)
use_modules (ProtocolUtilities RpcUtilities ReXLogic ProtocolModuleOpenSim ProtocolModuleTaiga OgreRenderingModule SceneManager)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS} )
use_framework_modules (Core Foundation Interfaces RexCommon)

link_package (BOOST)
link_package (POCO)
link_package (QT4)

final_target ()
