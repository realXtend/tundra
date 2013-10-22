# TODO for lasse: configure_boost() makes cmake add include/lib pats to all projects created 
# after the call. This needs to be fixed so that modules can opt in to using boost with use_boost()
# and link_boost() (is this needed due to the auto lib linking via pragmas?)
if (TUNDRA_NO_BOOST)
    configure_boost()
endif ()

###################################################################################################

# Define target name and output directory
init_target (WebSocketServerModule OUTPUT plugins)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
MocFolder ()

add_definitions (-DWEBSOCKET_MODULE_EXPORTS)

# Includes
UseTundraCore ()
use_core_modules (TundraCore Math Asset Scene Console OgreRenderingModule TundraProtocolModule)

build_library (${TARGET_NAME} SHARED ${CPP_FILES} ${H_FILES} ${MOC_SRCS})

# Linking
link_package_knet ()
link_modules (TundraCore Math OgreRenderingModule TundraProtocolModule)

# TODO for Lasse: Change this to point to automated build script results!
# Or I guess better use use_websocketpp() and link_websocketpp() macros.
if (WIN32)
    include_directories (
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/websocketpp/win/include
    )
    target_link_libraries (${TARGET_NAME} 
        ws2_32.lib
        optimized ${CMAKE_CURRENT_SOURCE_DIR}/deps/websocketpp/win/lib/websocketpp.lib
        debug ${CMAKE_CURRENT_SOURCE_DIR}/deps/websocketpp/win/lib/websocketppd.lib
    )
elseif (APPLE)
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/websocketpp/mac/include)

    target_link_libraries(${TARGET_NAME}
        optimized ${CMAKE_CURRENT_SOURCE_DIR}/deps/websocketpp/mac/lib/libwebsocketpp.a
        debug ${CMAKE_CURRENT_SOURCE_DIR}/deps/websocketpp/mac/lib/libwebsocketpp.a)
endif ()

SetupCompileFlags()

final_target ()
