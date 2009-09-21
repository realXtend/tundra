# Define target name
init_target (Foundation)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES RexQEngine.h OgreUIView.h)

# Include directories

# External library includes
INCLUDE_BOOST()
INCLUDE_POCO()
INCLUDE_QT4()

# Build parameters required by Qt - define files for moc prepass, process resource and ui files.
include(${QT_USE_FILE})
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

add_definitions (-DUSE_QT)

# Define static library

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})
use_framework_modules (${TARGET_NAME} Core Foundation Interfaces SceneManager)

# Precompiled headers on MSVC
if (MSVC)
	foreach (src_file ${CPP_FILES})
        	set_source_files_properties (${src_file} PROPERTIES COMPILE_FLAGS "/YuStableHeaders.h")
	endforeach (src_file ${CPP_FILES})

	set_source_files_properties (StableHeaders.cpp PROPERTIES COMPILE_FLAGS "/YcStableHeaders.h")
endif (MSVC)

LINK_BOOST()
LINK_POCO()
LINK_QT4()
