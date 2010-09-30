# Macro for mocfolder
macro (MocFolder)
    file (GLOB MOCS_TO_SOURCE_GROUP *.cxx */*.cxx)
    source_group ("CMake Moc" FILES ${MOCS_TO_SOURCE_GROUP})
endmacro (MocFolder)

macro (UiFolder)
    file (GLOB GENERATED_UI_H ui_*.h)
    source_group ("Generated UI" FILES ${GENERATED_UI_H})
endmacro (UiFolder)


# Define target name and output directory
init_target (CAVEStereoModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)


# SubFolders to project with filtering

MocFolder ()
UiFolder ()

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DCAVESTEREO_MODULE_EXPORTS)

use_package (BOOST)
use_package (QT4)
use_package (POCO)
use_package (CURL)
use_package (XMLRPC)
use_package (OGRE)
use_modules (UiModule Core Foundation Interfaces RexCommon SupportModules
    OgreRenderingModule )

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

link_modules (Core Foundation Interfaces RexCommon SupportModules OgreRenderingModule)
link_package (BOOST)
link_package (QT4)
link_package (POCO)
link_package (XMLRPC)
link_package (CURL)
link_package (OGRE)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()

