# Define target name and output directory
init_target (EnvironmentModule OUTPUT plugins)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES EC_*.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

add_definitions (-DENVIRONMENT_MODULE_EXPORTS)
set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${UI_FILES} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

UseTundraCore()
use_core_modules(TundraCore Math OgreRenderingModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${RESOURCE_SRCS} ${UI_SRCS})

link_ogre()
link_package(QT4)
link_modules(TundraCore Math OgreRenderingModule)

SetupCompileFlagsWithPCH()

final_target ()
