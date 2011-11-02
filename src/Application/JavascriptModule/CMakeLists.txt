# Define target name and output directory
init_target (JavascriptModule OUTPUT plugins)

GetEntityComponents()

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)
file (GLOB MOC_FILES JavascriptModule.h ScriptMetaTypeDefines.h JavascriptInstance.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})

use_core_modules(Framework Math Scene Input Audio Console TundraProtocolModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

link_ogre()
link_modules (Framework EC_Script Input Audio Console TundraProtocolModule)
link_entity_components (EC_Script)

SetupCompileFlags()

final_target ()

# Install files
setup_install_directory (${TUNDRA_BIN}/jsmodules)

# Install deps for windows, qtscriptgenerator plugins
setup_install_windows_deps_regexp ("[/]script[/]qtscript_+.*[^d][.]dll$" Release|RelWithDebInfo "qtscript_+.*[d][.]dll$" Debug)
