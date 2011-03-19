# Define target name and output directory
init_target (EC_WebView)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES EC_WebView.h)

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_modules (Core Foundation Interfaces Ui Input Scene Asset OgreRenderingModule)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS} )

LinkEntityComponent(EntityComponents/EC_3DCanvas EC_3DCanvas)

link_ogre()
link_modules (Core Foundation Interfaces Ui Input Scene Asset OgreRenderingModule)

SetupCompileFlagsWithPCH()

final_target ()
