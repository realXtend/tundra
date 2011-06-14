# Define target name and output directory
init_target (OgreRenderingModule OUTPUT plugins)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB UI_FILES *.ui)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES RenderWindow.h RendererSettings.h EC_*.h Renderer.h TextureAsset.h OgreMeshAsset.h
    OgreParticleAsset.h OgreSkeletonAsset.h OgreMaterialAsset.h OgreRenderingModule.h OgreWorld.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()
#UiFolder ()

# Build parameters required by Qt - define files for moc prepass, process resource and ui files.
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
#QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

add_definitions (-DOGRE_MODULE_EXPORTS)

#add_definitions (-DUSE_NVIDIA_PERFHUD)

use_package_assimp()
use_modules (Framework Scene Input Ui Asset Console)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

GetOpenAssetImport()
link_ogre()
link_modules (Framework Scene Ui Asset Console)

SetupCompileFlagsWithPCH()

final_target ()
