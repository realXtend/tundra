macro (configure_speex)
    if ("${SPEEX_ROOT}" STREQUAL "")
        file (TO_CMAKE_PATH "$ENV{SPEEX_ROOT}" SPEEX_ROOT)
    endif()
    if ("${SPEEX_ROOT}" STREQUAL "")
        SET(SPEEX_ROOT ${ENV_TUNDRA_DEP_PATH}/speex)
    endif()
    
    sagase_configure_package(SPEEX
        NAMES speex
        COMPONENTS libspeex         # speex lib
                   libspeexdsp      # speex audio processing lib
                   speex            # speex.h to find include dir
        PREFIXES ${SPEEX_ROOT})
        
    sagase_configure_report (SPEEX)
endmacro (configure_speex)

macro (configure_protobuf)   
    if ("${PROTOBUF_SRC_ROOT_FOLDER}" STREQUAL "")
        SET(PROTOBUF_SRC_ROOT_FOLDER $ENV{PROTOBUF_SRC_ROOT_FOLDER})
    endif()
    if ("${PROTOBUF_SRC_ROOT_FOLDER}" STREQUAL "")
        SET(PROTOBUF_SRC_ROOT_FOLDER ${ENV_TUNDRA_DEP_PATH}/protobuf)
    endif()

    find_package(Protobuf REQUIRED)
    sagase_configure_report (PROTOBUF)
endmacro (configure_protobuf)

macro (configure_openssl)       
    if ("${OPENSSL_ROOT_DIR}" STREQUAL "")
        file (TO_CMAKE_PATH "$ENV{OPENSSL_ROOT_DIR}" OPENSSL_ROOT_DIR)
    endif()
    if ("${OPENSSL_ROOT_DIR}" STREQUAL "")
        SET(OPENSSL_ROOT_DIR ${ENV_TUNDRA_DEP_PATH}/openssl)
    endif()
    
    find_package(OpenSSL REQUIRED)
    set(OPENSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR}) # Duplicate found includes to the proper variable so sagase is pleased.
    sagase_configure_report (OPENSSL)
endmacro (configure_openssl)

macro (configure_celt)
    if ("${CELT_ROOT}" STREQUAL "")
        file (TO_CMAKE_PATH "$ENV{CELT_ROOT}" CELT_ROOT)
    endif()
    if ("${CELT_ROOT}" STREQUAL "")
        SET(CELT_ROOT ${ENV_TUNDRA_DEP_PATH}/celt)
    endif()

    sagase_configure_package(CELT
        NAMES celt
        COMPONENTS libcelt  # for libcelt
                   celt0    # for old celt0 name (linux?)
                   celt     # for celt.h
        PREFIXES ${CELT_ROOT})
        
    sagase_configure_report (CELT)
endmacro (configure_celt)

configure_speex ()
configure_celt ()
configure_protobuf ()
configure_openssl ()

#################################################################################

# Define target name and output directory
init_target (MumblePlugin OUTPUT plugins)

# Define source files
file (GLOB CPP_FILES *.cpp *.cc)
file (GLOB H_FILES *.h)
file (GLOB UI_FILES *.ui)
file (GLOB QRC_FILES resource/*.qrc)

# Do manual source grouping as we need to detect a .cc file
file(GLOB H_FILES_MUMBLE mumble/*.h)
file(GLOB CPP_FILES_MUMBLE_${folder_name} mumble/*.cpp mumble/*.cc)
source_group("Header Files\\mumble" FILES ${H_FILES_MUMBLE})
source_group("Source Files\\mumble" FILES ${CPP_FILES_MUMBLE_})
set(H_FILES ${H_FILES} ${H_FILES_MUMBLE})
set(CPP_FILES ${CPP_FILES} ${CPP_FILES_MUMBLE_})

MocFolder ()
UiFolder ()

QT4_WRAP_CPP (MOC_SRCS ${H_FILES})
QT4_WRAP_UI (UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(QRC_SRCS ${QRC_FILES})

set (SOURCE_FILES ${SOURCE_FILES} ${CPP_FILES} ${H_FILES} ${MOC_SRCS} ${UI_SRCS} ${QRC_SRCS})

use_package (SPEEX)
use_package (CELT)
use_package (PROTOBUF)
use_package (OPENSSL)

# Gnerate build rule to generate header and source file(s) from a .proto file. 
# The files are generated on all builds if they don't exists already.
# So typically the in the start of the first build.
if (PROTOBUF_FOUND)
    message (STATUS "Generating custom build step for Mumble.proto")
    PROTOBUF_GENERATE_CPP(PROTOBUF_CPP_FILES PROTOBUF_H_FILES Mumble.proto)
endif ()

use_core_modules (Framework Math Scene Audio Console OgreRenderingModule TundraProtocolModule)
use_app_modules (JavascriptModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${PROTOBUF_CPP_FILES} ${PROTOBUF_H_FILES})

link_modules (Framework Math Scene Audio Console OgreRenderingModule TundraProtocolModule JavascriptModule)
link_entity_components (EC_Sound)

link_package (SPEEX)
link_package (CELT)
link_package (PROTOBUF)
link_package (OPENSSL)

if (APPLE)
    link_ogre()
endif ()

if (WIN32)
    target_link_libraries (${TARGET_NAME} ws2_32.lib)
endif()

SetupCompileFlags()

final_target ()
