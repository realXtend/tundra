macro(configure_speex)
    sagase_configure_package(SPEEX
        NAMES speex
        COMPONENTS libspeex
        PREFIXES ${ENV_TUNDRA_DEP_PATH} "/usr")
    sagase_configure_report (SPEEX)
endmacro()

macro(configure_qxmpp)
    if ("${QXMPP_ROOT}" STREQUAL "")
        file (TO_CMAKE_PATH "$ENV{QXMPP_ROOT}" QXMPP_ROOT)
    endif()
    if ("${QXMPP_ROOT}" STREQUAL "")
        SET(QXMPP_ROOT ${ENV_TUNDRA_DEP_PATH}/qxmpp)
    endif()

    sagase_configure_package(QXMPP
        NAMES qxmpp
        COMPONENTS qxmpp       # works on Ubuntu
               qxmpp_d     # find .lib on Windows
           QxmppClient # find QxmppClient.h on Windows
    PREFIXES ${QXMPP_ROOT}
                 ${ENV_TUNDRA_DEP_PATH})
    sagase_configure_report(QXMPP)
endmacro(configure_qxmpp)

# Define target name and output directory
init_target (XMPPModule OUTPUT plugins)

MocFolder()

# Define source files
file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB MOC_FILES XMPPModule.h Client/AccountManager.h Client/Call.h Client/CallExtension.h
    Client/ChatExtension.h Client/Client.h Client/Extension.h Client/MucExtension.h Client/UserItem.h)

AddSourceFolder(Client)
AddSourceFolder(Server)

set(SOURCE_FILES ${SOURCE_FILES} ${CPP_FILES} ${H_FILES})
set(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

configure_qxmpp()
configure_speex()
use_package(QXMPP)
use_package(SPEEX)

add_definitions(-DXMPP_MODULE_EXPORTS)

UseTundraCore()
use_app_modules(XMPPModule)
use_core_modules(TundraCore Math)

build_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

link_package(QT4)
link_modules(TundraCore)
link_package(QXMPP)
link_package(SPEEX)

SetupCompileFlagsWithPCH()

final_target()
