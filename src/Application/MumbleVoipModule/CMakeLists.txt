# Configure macros moved here as no other project needs these deps.
# Potentially something might need OpenSSL at some point, but then we can move these back.

macro (configure_protobuf)
    sagase_configure_package(PROTOBUF
        NAMES google protobuf
        COMPONENTS protobuf libprotobuf
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/protobuf)
    # Force include dir and libraries on MSVC
    if (MSVC)
          set (PROTOBUF_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/protobuf/include)
    endif ()
    sagase_configure_report (PROTOBUF)
endmacro (configure_protobuf)

macro (configure_openssl)
    sagase_configure_package(OPENSSL
        NAMES openssl
        COMPONENTS libeay32 ssleay32 ssl
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/OpenSSL)
    # remove 'NOTFOUND' entry which makes to linking impossible    
    if (MSVC)
        list(REMOVE_ITEM OPENSSL_LIBRARIES debug optimized SSL_EAY_RELEASE-NOTFOUND LIB_EAY_RELEASE-NOTFOUND SSL_EAY_DEBUG-NOTFOUND LIB_EAY_DEBUG-NOTFOUND NOTFOUND)
    endif ()
    sagase_configure_report (OPENSSL)
endmacro (configure_openssl)

macro (configure_celt)
    sagase_configure_package(CELT
        NAMES celt
        COMPONENTS libcelt  # for libcelt
                   celt0    # for old celt0 name (linux?)
                   celt     # for celt.h
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/celt)
    sagase_configure_report (CELT)
endmacro (configure_celt)

macro (configure_mumbleclient)
    sagase_configure_package(MUMBLECLIENT
        NAMES mumbleclient
        COMPONENTS mumbleclient client
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/libmumbleclient)
    sagase_configure_report (MUMBLECLIENT)
endmacro (configure_mumbleclient)

configure_protobuf ()
configure_openssl ()
configure_mumbleclient ()
configure_celt ()

#################################################################################

# Define target name and output directory
init_target (MumbleVoipModule OUTPUT plugins)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h )
file (GLOB UI_FILES ui/*.ui)

# Qt4 Moc files to subgroup "CMake Moc"
# and ui_*.h generated .h files to "Generated UI" subgroup
MocFolder ()
UiFolder ()

set (SOURCE_FILES ${SOURCE_FILES} ${CPP_FILES} ${H_FILES})

# Qt4 Wrap 
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})

add_definitions (-DMUMBLE_VOIP_MODULE_EXPORTS)
if (WIN32)
    add_definitions (-DLIBMUMBLECLIENT_STATIC)
    add_definitions (-D_WIN32_WINNT=0x0501)
endif ()

use_package (MUMBLECLIENT)
use_package (CELT)
use_package (PROTOBUF)
use_package (OPENSSL)

use_core_modules (Framework Scene Audio Console OgreRenderingModule TundraProtocolModule)
use_app_modules (JavascriptModule) # For registering metatypes to js engines

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

link_modules (Framework Scene Audio Console OgreRenderingModule TundraProtocolModule JavascriptModule)
    
link_package (MUMBLECLIENT)
link_package (CELT)
link_package (PROTOBUF)
link_package (OPENSSL)

SetupCompileFlagsWithPCH()

final_target ()
