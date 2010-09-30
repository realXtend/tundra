macro (AddSourceFolder folder_name)
    file (GLOB H_FILES_IN_FOLDER_${folder_name} ${folder_name}/*.h)
    file (GLOB CPP_FILES_IN_FOLDER_${folder_name} ${folder_name}/*.cpp)
    source_group ("Header Files\\${folder_name}" FILES ${H_FILES_IN_FOLDER_${folder_name}})
    source_group ("Source Files\\${folder_name}" FILES ${CPP_FILES_IN_FOLDER_${folder_name}})
    set (H_FILES ${H_FILES} ${H_FILES_IN_FOLDER_${folder_name}})
    set (CPP_FILES ${CPP_FILES} ${CPP_FILES_IN_FOLDER_${folder_name}})
endmacro (AddSourceFolder)

macro (MocFolder)
    file (GLOB MOCS_TO_SOURCE_GROUP *.cxx */*.cxx */*/*.cxx */*/*/*.cxx)
    source_group ("CMake Moc" FILES ${MOCS_TO_SOURCE_GROUP})
endmacro (MocFolder)

macro (UiFolder)
    file (GLOB GENERATED_UI_H ui_*.h)
    source_group ("Generated UI" FILES ${GENERATED_UI_H})
endmacro (UiFolder)

#############################################################################################

# Define target name and output directory
init_target (WorldBuildingModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)

# SubFolders to project with filtering
#AddSourceFolder (Subfolder)

# Qt4 Moc files to "CMake Moc" subgroup
# and ui_*.h generated .h files to "Generated UI" subgroup
MocFolder ()
UiFolder ()

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
#QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DWORLD_BUILDING_MODULE_EXPORTS)

# Includes
use_package (BOOST)
use_package (QT4)
use_package (POCO)
use_package (OGRE)
use_package (CURL)    # NEEDED BY ProtocolUtilities
use_package (XMLRPC)  # NEEDED BY ProtocolUtilities
use_package (QT_PROPERTY_BROWSER)
use_modules (Core Foundation Interfaces SceneManager RexCommon SupportModules ProtocolUtilities OgreRenderingModule InventoryModule EntityComponents/EC_OpenSimPrim)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

# Linking
link_package (BOOST)
link_package (QT4)
link_package (POCO)
link_package (OGRE)
link_package (CURL)   # NEEDED BY ProtocolUtilities
link_package (XMLRPC) # NEEDED BY ProtocolUtilities
link_package (QT_PROPERTY_BROWSER)
link_modules (Core Foundation Interfaces SceneManager RexCommon SupportModules ProtocolUtilities OgreRenderingModule InventoryModule EC_OpenSimPrim)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()


final_target ()

