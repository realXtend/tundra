
macro (configure_speex)
    set (SPEEX_ROOT C:/Work/admino-tundra/deps/custom/speex) # this is a hack for initial development
    set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${SPEEX_ROOT}/include)
    set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${SPEEX_ROOT}/lib)
    set (CMAKE_PROGRAM_PATH ${CMAKE_PROGRAM_PATH} ${SPEEX_ROOT}/bin)
    
    message (STATUS "## EARLY DEVELOPMENT HACK ## Speex lookup from: " ${SPEEX_ROOT})
    
    sagase_configure_package(SPEEX
        NAMES speex
        COMPONENTS libspeex         # speex lib
                   libspeexdsp      # speex audio processing lib
                   speex            # speex.h to find include dir
        PREFIXES ${SPEEX_ROOT} ${SPEEX_ROOT}/include/speex)  # this is a hack for initial development
        
    sagase_configure_report (SPEEX)
endmacro (configure_speex)

macro (configure_protobuf)
    # Give cmake some hints where to look for libs/includes/executables in Find*.cmake helpers.
    # You can add paths here if the below are not maching to your setup.
    set (PROTOBUF_ROOT C:/Work/admino-tundra/deps/custom/protobuf) # this is a hack for initial development
    set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${PROTOBUF_ROOT}/include)
    set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${PROTOBUF_ROOT}/lib)
    set (CMAKE_PROGRAM_PATH ${CMAKE_PROGRAM_PATH} ${PROTOBUF_ROOT}/bin)
    
    message (STATUS "## EARLY DEVELOPMENT HACK ## Protobuf lookup from: " ${PROTOBUF_ROOT})
    
    sagase_configure_package(PROTOBUF
        NAMES google protobuf
        COMPONENTS protobuf         # protobuf lib
                   libprotobuf      # protobuf lib
                   service          # service.h to find include dir
        PREFIXES ${PROTOBUF_ROOT} ${PROTOBUF_ROOT}/include/google/protobuf)  # this is a hack for initial development
        #PREFIXES ${ENV_TUNDRA_DEP_PATH}/protobuf                            # Finding the libs
        #         ${ENV_TUNDRA_DEP_PATH}/protobuf/include/google/protobuf)   # Finding the include

    # Remove FindProtobuf.cmake generated not found item
    list (REMOVE_ITEM PROTOBUF_LIBRARIES PROTOBUF_LIBRARY-NOTFOUND)
    
    # Duplicate found includes to the proper variable so sagase is pleased. 
    if (MSVC)
      set (PROTOBUF_INCLUDE_DIRS ${PROTOBUF_INCLUDE_DIR})
    endif ()

    sagase_configure_report (PROTOBUF)
endmacro (configure_protobuf)

macro (configure_openssl)
    sagase_configure_package(OPENSSL
        NAMES openssl
        COMPONENTS libeay32 ssleay32 ssl
        PREFIXES C:/Work/admino-tundra/deps/custom/OpenSSL) # this is a hack for initial development
        #PREFIXES ${ENV_TUNDRA_DEP_PATH}/OpenSSL)
    
    message (STATUS "## EARLY DEVELOPMENT HACK ## OpenSSL lookup from: C:/Work/admino-tundra/deps/custom/OpenSSL")
    
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
