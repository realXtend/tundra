# Define target name and output directory
init_target (WebSocketServerModule OUTPUT plugins)

if (TUNDRA_NO_BOOST)
	# Linux can cope with only system.
	# TODO: Check if this is the same for windows.
	if (UNIX)
		set(TUNDRA_BOOST_LIBRARIES system)
	endif ()
    configure_boost()
endif()

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES WebSocketServer.h WebSocketServerModule.h WebSocketSyncManager.h WebSocketUserConnection.h)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

add_definitions (-DWEBSOCKET_MODULE_EXPORTS)
if (TUNDRA_CPP11_ENABLED)
	add_definitions (-D_WEBSOCKETPP_CPP11_STL_)
endif()

# Includes
UseTundraCore ()
use_core_modules (TundraCore Math Asset Scene Console OgreRenderingModule PhysicsModule TundraProtocolModule)

if (WIN32)
include_directories(
    ${ENV_TUNDRA_DEP_PATH}/websocketpp
)
else()
include_directories(
    ${ENV_TUNDRA_DEP_PATH}/include
)
endif()

build_library (${TARGET_NAME} SHARED ${CPP_FILES} ${H_FILES} ${MOC_SRCS})

# Linking
link_ogre ()
link_package_knet ()
link_modules (TundraCore Math OgreRenderingModule TundraProtocolModule)

if (WIN32)
    target_link_libraries (${TARGET_NAME}
        ws2_32.lib
    )
else()
    # Visual Studio uses library auto-linking, so this is only necessary for the other platforms.
    # TODO: Check if APPLE work with only system and remove this check
    if (UNIX)
        target_link_libraries(${TARGET_NAME} optimized ${Boost_SYSTEM_LIBRARY_RELEASE} debug ${Boost_SYSTEM_LIBRARY_DEBUG})
    else()
        target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})
    endif()
endif()

SetupCompileFlags()

final_target ()
