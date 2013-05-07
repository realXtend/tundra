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
        PREFIXES ${SPEEX_ROOT}      # full prebuilt deps
                 ${ENV_TUNDRA_DEP_PATH}
                 ${SPEEX_ROOT}/lib/Release) # packaged prebuilt deps
        
    sagase_configure_report (SPEEX)
endmacro (configure_speex)

macro (configure_protobuf)
    if (APPLE)
        find_package(Protobuf REQUIRED)

        set (PROTOBUF_LIBRARIES ${ENV_TUNDRA_DEP_PATH}/protobuf/lib/libprotobuf.7.dylib)
        set (PROTOBUF_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/protobuf/include)

        if ("${PROTOBUF_PROTOC_EXECUTABLE}" STREQUAL "PROTOBUF_PROTOC_EXECUTABLE-NOTFOUND")
            set (PROTOBUF_PROTOC_EXECUTABLE ${ENV_TUNDRA_DEP_PATH}/protobuf/bin/protoc)
            message (STATUS "Protoc executable not found automatically, guessing location to " ${PROTOBUF_PROTOC_EXECUTABLE})
        else ()
            message (STATUS "Protoc executable found from: " ${PROTOBUF_PROTOC_EXECUTABLE})
        endif ()
        
        # Force the protobuf as found or Mumble.proto wont be processed.
        set (PROTOBUF_FOUND 1)
    else()
        if ("${PROTOBUF_SRC_ROOT_FOLDER}" STREQUAL "")
            file (TO_CMAKE_PATH "$ENV{PROTOBUF_SRC_ROOT_FOLDER}" PROTOBUF_SRC_ROOT_FOLDER)
        endif()
        if ("${PROTOBUF_SRC_ROOT_FOLDER}" STREQUAL "")
            SET(PROTOBUF_SRC_ROOT_FOLDER ${ENV_TUNDRA_DEP_PATH}/protobuf)
        endif()

        find_package(Protobuf REQUIRED)

        # Stupid FindProtobuf.cmake does not find includes on windows for packaged deps, set manually.
        # It is assuming include path is <protobuf>/vcprojects/include on windows, ARGH!
        if ("${PROTOBUF_INCLUDE_DIR}" STREQUAL "PROTOBUF_INCLUDE_DIR-NOTFOUND")
            # ~Hack for prebuilt deps. todo: change the structure in prebuilt deps so this goes away!
            if (IS_DIRECTORY ${PROTOBUF_SRC_ROOT_FOLDER}/include)
                set (PROTOBUF_INCLUDE_DIR ${PROTOBUF_SRC_ROOT_FOLDER}/include)
                set (PROTOBUF_INCLUDE_DIRS ${PROTOBUF_SRC_ROOT_FOLDER}/include)
                set (PROTOBUF_FOUND 1)
            else()
                message(FATAL_ERROR "Could not detect include folder from ${PROTOBUF_SRC_ROOT_FOLDER}/include")
            endif()
        endif()
    endif()
    
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
        PREFIXES ${CELT_ROOT}
                 ${ENV_TUNDRA_DEP_PATH}
                 ${CELT_ROOT}/lib/Debug
                 ${CELT_ROOT}/lib/Release)
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

QT4_WRAP_CPP(MOC_SRCS AudioProcessor.h AudioWizard.h CeltCodec.h MumbleData.h MumbleDefines.h
    MumbleNetworkHandler.h MumblePlugin.h mumble/AudioStats.h)
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
else()
    message(FATAL_ERROR "Protobuf was not found properly, cannot make custom build step for Mumble.proto")
endif ()

UseTundraCore()
use_core_modules(TundraCore Math OgreRenderingModule TundraProtocolModule)
use_app_modules(JavascriptModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${PROTOBUF_CPP_FILES} ${PROTOBUF_H_FILES})

link_modules(TundraCore Math OgreRenderingModule TundraProtocolModule JavascriptModule)
link_entity_components(EC_Sound)

link_package(QT4)
link_package(SPEEX)
link_package(CELT)
link_package(PROTOBUF)
link_package(OPENSSL)

if (NOT WIN32)
    link_ogre()

    #TODO This line should not be needed, as these libraries should be picked up by above link_package. However,
    # does not seem to be happening. Remove this line when the above link_packages have been fixed.
    target_link_libraries(${TARGET_NAME} speex speexdsp)
endif ()

if (WIN32)
    target_link_libraries (${TARGET_NAME} ws2_32.lib)
    # Delayed qwave.dll loading for QoS
    target_link_libraries (${TARGET_NAME} delayimp.lib)
    target_link_libraries (${TARGET_NAME} qwave.lib)
    set_target_properties (${TARGET_NAME} PROPERTIES LINK_FLAGS "/DELAYLOAD:qwave.dll")
endif()

SetupCompileFlags()

final_target ()

setup_install_files (mumble/MumbleLicence.txt)
