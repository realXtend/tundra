# Define target name and output directory
init_target (EtherModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)

AddSourceFolder (View)
#AddSourceFolder (View/Classical)
AddSourceFolder (Data)

# Qt4 Moc files to "CMake Moc" subgroup
# and ui_*.h generated .h files to "Generated UI" subgroup
MocFolder ()
UiFolder ()

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})
set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})

add_definitions (-DDYNAMIC_LOGIN_SCENE)

use_modules (EtherModule Core Foundation Interfaces Input RexCommon ProtocolUtilities OgreRenderingModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

link_modules (Core Foundation Interfaces Input RexCommon ProtocolUtilities OgreRenderingModule)
link_ogre()

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()
