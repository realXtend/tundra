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
file (GLOB MOC_FILES RexLogicModule.h EventHandlers/LoginHandler.h RexMovementInput.h EventHandlers/MainPanelHandler.h 
                     EntityComponent/EC_*.h Environment/Primitive.h Communications/*.h Communications/InWorldChat/*.h)

# SubFolders to project with filtering
AddSourceFolder (Avatar)
AddSourceFolder (EntityComponent)
AddSourceFolder (EventHandlers)
AddSourceFolder (Environment)
AddSourceFolder (Communications)
AddSourceFolder (Communications/InWorldChat)

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
    HttpUtilities RpcUtilities ProtocolUtilities ProtocolModuleOpenSim ProtocolModuleTaiga EnvironmentModule
    EntityComponents/EC_Highlight EntityComponents/EC_HoveringText EntityComponents/EC_Clone EntityComponents/EC_Light
    EntityComponents/EC_OpenSimPresence EntityComponents/EC_OpenSimPrim EntityComponents/EC_Touchable EntityComponents/EC_3DCanvas
    EntityComponents/EC_3DCanvasSource EntityComponents/EC_ChatBubble EntityComponents/EC_Ruler EntityComponents/EC_SoundRuler 
    EntityComponents/EC_ParticleSystem EntityComponents/EC_SoundListener EntityComponents/EC_Sound EntityComponents/EC_Mesh
    EntityComponents/EC_InputMapper EntityComponents/EC_Movable EntityComponents/EC_HoveringWidget)

if (NOT UISERVICE_TEST)
    use_modules (UiModule)
endif ()

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_modules (Core Foundation Interfaces SceneManager OgreRenderingModule AssetModule AvatarModule RexCommon
    HttpUtilities RpcUtilities ProtocolUtilities ProtocolModuleOpenSim ProtocolModuleTaiga EnvironmentModule
    EC_Highlight EC_HoveringText EC_Clone EC_Light EC_OpenSimPresence EC_OpenSimPrim EC_Touchable EC_3DCanvas
    EC_3DCanvasSource EC_ChatBubble EC_Ruler EC_SoundRuler EC_ParticleSystem EC_SoundListener EC_Sound EC_Mesh
    EC_InputMapper EC_Movable EC_HoveringWidget)

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

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
    add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

# MSVC -specific settings for preprocessor and PCH use
if (MSVC)
    # Label StableHeaders.cpp to create the PCH file and mark all other .cpp files to use that PCH file.
    # Add a #define DEBUG_CPP_NAME "this compilation unit name" to each compilation unit to aid in memory leak checking.
    foreach(src_file ${CPP_FILES})
            if (${src_file} MATCHES "StableHeaders.cpp$")
                set_source_files_properties(${src_file} PROPERTIES COMPILE_FLAGS "/YcStableHeaders.h")
            else()
                get_filename_component(basename ${src_file} NAME)
                set_source_files_properties(${src_file} PROPERTIES COMPILE_FLAGS "/YuStableHeaders.h -DDEBUG_CPP_NAME=\"\\\"${basename}\"\\\"")
            endif()
    endforeach(src_file ${CPP_FILES})
endif (MSVC)

final_target ()
