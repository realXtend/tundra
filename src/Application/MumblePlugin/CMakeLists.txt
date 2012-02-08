
macro (configure_speex)
    sagase_configure_package(SPEEX
        NAMES speex
        COMPONENTS libspeex         # speex lib
                   libspeexdsp      # speex audio processing lib
                   speex            # speex.h to find include dir
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/speex)
        
    sagase_configure_report (SPEEX)
endmacro (configure_speex)

macro (configure_protobuf)
    # Give FindProtobuf.cmake some hints on windows, the script is not very good on its own if the full protobuf build is not present.
    if (MSVC)
        set (PROTOBUF_ROOT ${ENV_TUNDRA_DEP_PATH}/protobuf)
        set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${PROTOBUF_ROOT}/include ${PROTOBUF_ROOT}/include/google/protobuf)
        set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${PROTOBUF_ROOT}/lib)
        set (CMAKE_PROGRAM_PATH ${CMAKE_PROGRAM_PATH} ${PROTOBUF_ROOT}/bin)
    endif ()
    
    sagase_configure_package(PROTOBUF
        NAMES protobuf
        COMPONENTS libprotobuf      # protobuf lib
                   service          # service.h to find include dir
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/protobuf                            # Finding the libs
                 ${ENV_TUNDRA_DEP_PATH}/protobuf/include/google/protobuf)   # Finding the include

    # Remove FindProtobuf.cmake generated not found item
    list (REMOVE_ITEM PROTOBUF_LIBRARIES PROTOBUF_LIBRARY-NOTFOUND)
    
    # Duplicate found includes to the proper variable so sagase is pleased. FindProtobuf.cmake
    # does not always populate the PROTOBUF_INCLUDE_DIRS but does for PROTOBUF_INCLUDE_DIR that is cached, pleh.
    # FindProtobuf.cmake is not particularly smart about finding the debug library 
    # at least with a d.dll postfix that we currently have. Only use the release one.
    if (MSVC)
        set (PROTOBUF_LIBRARIES ${PROTOBUF_LIBRARY})
        set (PROTOBUF_INCLUDE_DIRS ${PROTOBUF_INCLUDE_DIR})
    endif ()

    sagase_configure_report (PROTOBUF)
endmacro (configure_protobuf)

macro (configure_openssl)
    # This is a variable FindOpenSSL.cmake will inspect.
    set (OPENSSL_ROOT_DIR ${ENV_TUNDRA_DEP_PATH}/OpenSSL)
    
    sagase_configure_package(OPENSSL
        NAMES openssl
        COMPONENTS libeay32 ssleay32 ssl
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/OpenSSL)
       
    # Remove FindOpenSSL.cmake generated not found items
    if (MSVC)
        list(REMOVE_ITEM OPENSSL_LIBRARIES debug optimized SSL_EAY_RELEASE-NOTFOUND LIB_EAY_RELEASE-NOTFOUND SSL_EAY_DEBUG-NOTFOUND LIB_EAY_DEBUG-NOTFOUND NOTFOUND)
    endif ()
    
    # Duplicate found includes to the proper variable so sagase is pleased.
    set(OPENSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR})
    
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
