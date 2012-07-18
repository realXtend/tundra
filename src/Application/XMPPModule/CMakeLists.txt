macro(configure_speex)
    sagase_configure_package(SPEEX
        NAMES speex
        COMPONENTS libspeex
        PREFIXES ${ENV_TUNDRA_DEP_PATH} "/usr")
    sagase_configure_report (SPEEX)
endmacro()

macro(configure_qxmpp)
    if(NOT WIN32)
    sagase_configure_package(QXMPP
        NAMES qxmpp
        COMPONENTS qxmpp
        PREFIXES ${ENV_TUNDRA_DEP_PATH})
    sagase_configure_report(QXMPP)
    else ()
        # Unimplemented
    endif ()
endmacro(configure_qxmpp)

# Define target name and output directory
init_target (XMPPModule OUTPUT plugins)

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
#file (GLOB MOC_FILES XMPPModule.h)

AddSourceFolder (Client)
AddSourceFolder (Server)

set (SOURCE_FILES ${SOURCE_FILES} ${CPP_FILES} ${H_FILES})
set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})

configure_qxmpp()
configure_speex()
use_package (QXMPP)
use_package (SPEEX)

use_app_modules (XMPPModule)
use_core_modules (Framework Audio Math)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

link_modules (Framework Audio)
link_package (QXMPP)
link_package (SPEEX)

SetupCompileFlagsWithPCH()

final_target ()

