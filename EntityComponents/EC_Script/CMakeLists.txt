# Define target name and output directory
init_target (EC_Script)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES EC_Script.h IScriptInstance.h ScriptAsset.h ScriptAssetFactory.h)

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_modules (Framework Interfaces Scene Asset)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS} )

link_modules (Framework Interfaces Scene EC_Script Asset)

SetupCompileFlagsWithPCH()

final_target ()