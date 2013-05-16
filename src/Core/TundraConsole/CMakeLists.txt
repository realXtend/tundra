# Define target name and output directory
init_target (TundraConsole OUTPUT ./)

# Define source files
file (GLOB CPP_FILES main.cpp)
file (GLOB H_FILES "") # Dont do *.h it will pickup Windows-spesific resource.h
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})
set (RESOURCE_FILES "")

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

if (MSVC)
    # Windows resource files for icon.
    if (BUILDING_INSTALLER)
        set(RESOURCE_FILES ${RESOURCE_FILES} resource.h Tundra.rc)
    endif()

    # Preprocessor defines on MSVC
    set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS_RELEASE _DMEMDUMP)
    set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS_RELWITHDEBINFO _DMEMDUMP)
endif()

SetupCompileFlags()

UseTundraCore()
use_core_modules(TundraCore Math)

build_executable(${TARGET_NAME} ${SOURCE_FILES} ${RESOURCE_FILES})

if (WIN32 AND CMAKE_CL_64)
    link_package_knet() # For kNet's BottomMemoryAllocator
endif()

link_modules(TundraCore)

final_target ()
