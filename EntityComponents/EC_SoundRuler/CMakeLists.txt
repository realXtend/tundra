# Define target name and output directory
init_target (EC_SoundRuler)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES EC_SoundRuler.h)

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_modules (Core Foundation Interfaces Scene OgreRenderingModule)

if (ENABLE_TAIGA_SUPPORT)
use_modules (RexCommon EntityComponents/EC_OpenSimPrim)
endif()

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS} )

link_ogre()
link_modules (Core Foundation Interfaces Scene OgreRenderingModule)

if (ENABLE_TAIGA_SUPPORT)
link_modules (EC_OpenSimPrim)
endif()

SetupCompileFlagsWithPCH()

final_target ()
