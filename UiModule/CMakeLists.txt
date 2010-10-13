# Define target name and output directory
init_target (UiModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)
file (GLOB RESOURCE_FILES resource/*.qrc)

# SubFolders to project with filtering
AddSourceFolder (Common)

AddSourceFolder (Ether)
AddSourceFolder (Ether/View)
AddSourceFolder (Ether/View/Classical)
AddSourceFolder (Ether/Data)

AddSourceFolder (Inworld)
AddSourceFolder (Inworld/View)
AddSourceFolder (Inworld/Menus)
AddSourceFolder (Inworld/ControlPanel)
AddSourceFolder (Inworld/Notifications)

# Qt4 Moc files to "CMake Moc" subgroup
# and ui_*.h generated .h files to "Generated UI" subgroup
MocFolder ()
UiFolder ()

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)
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
use_package (OGRE)
use_modules (UiModule Core Foundation Interfaces SceneManager RexCommon ProtocolUtilities SupportModules OgreRenderingModule Ui Input)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

link_modules (Core Foundation Interfaces SceneManager RexCommon ProtocolUtilities SupportModules OgreRenderingModule)
link_package (BOOST)
link_package (QT4)
link_package (POCO)
link_package (XMLRPC)
link_package (CURL)
link_ogre()

# ETHER TYPE DEFINE
if (DYNAMIC_LOGIN_SCENE)
   add_definitions (-DDYNAMIC_LOGIN_SCENE)
endif (DYNAMIC_LOGIN_SCENE)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()

