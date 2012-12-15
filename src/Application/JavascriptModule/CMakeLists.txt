# Define target name and output directory
init_target (JavascriptModule OUTPUT plugins)

GetEntityComponents()

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB UI_FILES ui/*.ui)
file (GLOB MOC_FILES JavascriptModule.h JavascriptInstance.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})

UseTundraCore()
use_core_modules(TundraCore Math TundraProtocolModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

link_package(QT4)
link_ogre()
link_modules(TundraCore Math TundraProtocolModule)
link_entity_components(EC_Script)

SetupCompileFlags()

final_target ()

# Install files
setup_install_directory (${TUNDRA_BIN}/jsmodules)

# Install deps for windows, qtscriptgenerator plugins.
if (IS_DIRECTORY ${ENV_TUNDRA_DEP_PATH}/qtscriptgenerator/plugins/script) # Full deps
    setup_install_windows_deps_regexp (qtplugins/script qtscriptgenerator/plugins/script/ "[/]script[/]qtscript_+.*[^d][.]dll$" Release|RelWithDebInfo 
        qtscriptgenerator/plugins/script/ "[/]script[/]qtscript_+.*[d][.]dll$" Debug)
elseif (IS_DIRECTORY ${ENV_TUNDRA_DEP_PATH}/qt/plugins/script) # Prebuilt from full deps
    setup_install_windows_deps_regexp (qtplugins/script qt/plugins/script/ "[/]script[/]qtscript_+.*[^d][.]dll$" Release|RelWithDebInfo 
        qt/plugins/script/ "[/]script[/]qtscript_+.*[d][.]dll$" Debug)
endif ()
