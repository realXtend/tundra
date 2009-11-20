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
init_target (RexLogicModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES Avatar/AvatarEditor.h Login/*.h)

# SubFolders to project with filtering
AddSourceFolder (Avatar)
AddSourceFolder (EntityComponent)
AddSourceFolder (EventHandlers)
AddSourceFolder (Environment)
AddSourceFolder (Login)

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
#QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DUSE_QT)

use_package (BOOST)
use_package (POCO)
use_package (OGRE)
use_package (QT4)
use_package (CURL)
use_package (OPENJPEG)
use_package (OIS)
use_package (XMLRPC)

use_modules (RexLogicModule OgreRenderingModule AssetModule RexCommon QtModule HttpUtilities RpcUtilities ProtocolUtilities InputModuleOIS ProtocolModuleOpenSim ProtocolModuleTaiga)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})
use_framework_modules (Core Foundation Interfaces SceneManager)

link_modules (OgreRenderingModule AssetModule RexCommon QtModule HttpUtilities RpcUtilities ProtocolUtilities ProtocolModuleOpenSim ProtocolModuleTaiga)

link_package (OIS)
link_package (OGRE)
link_package (BOOST)
link_package (POCO)
link_package (QT4)
link_package (CURL)
link_package (OPENJPEG)
link_package (XMLRPC)

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
    add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

# Precompiled headers on MSVC
if (MSVC)
    foreach( src_file ${CPP_FILES} )
            set_source_files_properties( ${src_file}
                    PROPERTIES
                    COMPILE_FLAGS "/YuStableHeaders.h")
    endforeach( src_file ${CPP_FILES} )

    set_source_files_properties(StableHeaders.cpp
            PROPERTIES
            COMPILE_FLAGS "/YcStableHeaders.h")
endif (MSVC)

final_target ()
