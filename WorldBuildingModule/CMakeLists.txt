# Define target name and output directory
init_target (WorldBuildingModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)

# SubFolders to project with filtering
AddSourceFolder (OpenSimScene)

# Qt4 Moc files to "CMake Moc" subgroup
# and ui_*.h generated .h files to "Generated UI" subgroup
MocFolder ()
UiFolder ()

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})

add_definitions (-DWORLD_BUILDING_MODULE_EXPORTS)

# Includes
use_package (BOOST)
use_package (QT4)
use_package (POCO)
use_package (OGRE)
use_package (CURL)    # NEEDED BY ProtocolUtilities
use_package (XMLRPC)  # NEEDED BY ProtocolUtilities
use_package (QT_PROPERTY_BROWSER)
use_modules (WorldBuildingModule Core Foundation Interfaces SceneManager RexCommon SupportModules ProtocolUtilities OgreRenderingModule InventoryModule Input
             EntityComponents/EC_OpenSimPrim EntityComponents/EC_DynamicComponent EntityComponents/EC_Highlight)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

# Linking
link_package (BOOST)
link_package (QT4)
link_package (POCO)
link_ogre()
link_package (CURL)   # NEEDED BY ProtocolUtilities
link_package (XMLRPC) # NEEDED BY ProtocolUtilities
link_package (QT_PROPERTY_BROWSER)
link_modules (Core Foundation Interfaces SceneManager RexCommon SupportModules ProtocolUtilities OgreRenderingModule InventoryModule Input
              EC_OpenSimPrim EC_DynamicComponent EC_Highlight)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
