# Define target name and output directory
init_target (viewer OUTPUT ./)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Include directories
include_directories (
	../Core
	../Foundation
	../Interfaces
	../SupportModules
	../TestModule
)

# External library includes
INCLUDE_POCO()
INCLUDE_BOOST()

# Preprocessor defines
if (MSVC)
   set_property (DIRECTORY PROPERTY COMPILE_DEFINITIONS_RELEASE _DMEMDUMP)
   set_property (DIRECTORY PROPERTY COMPILE_DEFINITIONS_DEBUG _DMEMORYLEAKS)
endif (MSVC)

if (MSVC AND WINDOWS_APP)
   add_executable (${TARGET_NAME} WIN32 ${SOURCE_FILES})
else (MSVC AND WINDOWS_APP)
   add_executable (${TARGET_NAME} ${SOURCE_FILES})
endif (MSVC AND WINDOWS_APP)

set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)

# Define libraries to be linked into target
target_link_libraries (${TARGET_NAME} Core Foundation)
if (MSVC)
   target_link_libraries (${TARGET_NAME} optimized dbghelp.lib)
endif (MSVC)

# External library linkage
LINK_POCO()
LINK_BOOST()
