# Define target name and output directory
init_target (EC_Script)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES EC_Script.h IScriptInstance.h ScriptAsset.h ScriptAssetFactory.h)

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

UseTundraCore()
use_core_modules(TundraCore Math)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS} )

link_modules(TundraCore)

SetupCompileFlagsWithPCH()

final_target ()
