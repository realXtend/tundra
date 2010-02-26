macro (AddSourceFolder folder_name)
    file (GLOB H_FILES_IN_FOLDER_${folder_name} ${folder_name}/*.h)
    file (GLOB CPP_FILES_IN_FOLDER_${folder_name} ${folder_name}/*.cpp)
    source_group ("Header Files\\${folder_name}" FILES ${H_FILES_IN_FOLDER_${folder_name}})
    source_group ("Source Files\\${folder_name}" FILES ${CPP_FILES_IN_FOLDER_${folder_name}})
    set (H_FILES ${H_FILES} ${H_FILES_IN_FOLDER_${folder_name}})
    set (CPP_FILES ${CPP_FILES} ${CPP_FILES_IN_FOLDER_${folder_name}})
endmacro (AddSourceFolder)

macro (MocFolder)
    file (GLOB MOCS_TO_SOURCE_GROUP *.cxx */*.cxx */*/*.cxx)
    source_group ("CMake Moc" FILES ${MOCS_TO_SOURCE_GROUP})
endmacro (MocFolder)

macro (UiFolder)
    file (GLOB GENERATED_UI_H ui_*.h)
    source_group ("Generated UI" FILES ${GENERATED_UI_H})
endmacro (UiFolder)

#############################################################################################

# Define target name and output directory
init_target (UiModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)
file (GLOB RESOURCE_FILES resource/*.qrc)

# SubFolders to project with filtering
AddSourceFolder (Ether)
AddSourceFolder (Ether/View)
AddSourceFolder (Ether/Data)
AddSourceFolder (Login)
AddSourceFolder (MainPanel)
AddSourceFolder (Notify)
AddSourceFolder (Console)

# Qt4 Moc files to "CMake Moc" subgroup
# and ui_*.h generated .h files to "Generated UI" subgroup
MocFolder ()
UiFolder ()

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DUI_MODULE_EXPORTS)

use_package (BOOST)
use_package (QT4)
use_package (POCO)
use_package (CURL)
use_package (XMLRPC)
use_modules (Core Foundation Interfaces SceneManager UiModule RexCommon ProtocolUtilities SupportModules)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

link_modules (Core Foundation Interfaces SceneManager RexCommon SupportModules ProtocolUtilities)
link_package (BOOST)
link_package (QT4)
link_package (POCO)
link_package (XMLRPC)
link_package (CURL)

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
    add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

# Precompiled headers on MSVC
if (MSVC)
    foreach( src_file ${CPP_FILES} )
            set_source_files_properties( ${src_file} PROPERTIES COMPILE_FLAGS "/YuStableHeaders.h")
    endforeach( src_file ${CPP_FILES} )
    set_source_files_properties(StableHeaders.cpp PROPERTIES COMPILE_FLAGS "/YcStableHeaders.h")
endif (MSVC)

final_target ()

