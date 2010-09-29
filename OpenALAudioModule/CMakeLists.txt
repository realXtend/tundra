macro (MocFolder)
    file (GLOB MOCS_TO_SOURCE_GROUP *.cxx */*.cxx)
    source_group ("CMake Moc" FILES ${MOCS_TO_SOURCE_GROUP})
endmacro (MocFolder)

#############################################################################################

configure_openal ()
configure_ogg ()
configure_vorbis ()

# Define target name and output directory
init_target (OpenALAudioModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES SoundSettings.h)
file (GLOB RESOURCE_FILES resource/*.qrc)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

add_definitions (-DOPENAL_MODULE_EXPORTS)

use_package (BOOST)
use_package (POCO)
use_package (OPENAL)
use_package (OGG)
use_package (QT4)
use_package (VORBIS)
use_modules (Core Foundation Interfaces RexCommon AssetModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_modules (Core Foundation Interfaces RexCommon)

link_package (BOOST)
link_package (POCO)
link_package (OPENAL)
link_package (OGG)
link_package (QT4)
link_package (VORBIS)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()

