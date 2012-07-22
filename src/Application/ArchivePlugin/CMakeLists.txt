
macro (configure_zziplib)
    if ("${ZZIPLIB_ROOT}" STREQUAL "")
        file (TO_CMAKE_PATH "$ENV{ZZIPLIB_ROOT}" ZZIPLIB_ROOT)
    endif()
    if ("${ZZIPLIB_ROOT}" STREQUAL "")
        SET(ZZIPLIB_ROOT ${ENV_TUNDRA_DEP_PATH}/zziplib)
    endif()
    
    message("${ZZIPLIB_ROOT}")
    sagase_configure_package(ZZIPLIB
        NAMES zziplib
        COMPONENTS zziplib
        PREFIXES ${ZZIPLIB_ROOT} ${ENV_TUNDRA_DEP_PATH})
        
    set (ZZIPLIB_INCLUDE_DIRS ${ZZIPLIB_INCLUDE_DIRS} ${ZZIPLIB_ROOT}/include)
    
    sagase_configure_report (ZZIPLIB)
endmacro (configure_zziplib)

macro (configure_zlib)
    if ("${ZLIB_ROOT}" STREQUAL "")
        file (TO_CMAKE_PATH "$ENV{ZLIB_ROOT}" ZLIB_ROOT)
    endif()
    if ("${ZLIB_ROOT}" STREQUAL "")
        SET(ZLIB_ROOT ${ENV_TUNDRA_DEP_PATH}/zlib)
    endif()
    
    sagase_configure_package(ZLIB
        NAMES zlib
        COMPONENTS zlib
        PREFIXES ${ZLIB_ROOT} ${ENV_TUNDRA_DEP_PATH})
        
    sagase_configure_report (ZLIB)
endmacro (configure_zlib)

configure_zziplib ()
#configure_zlib () 

#################################################################################

# Define target name and output directory
init_target (ArchivePlugin OUTPUT plugins)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)

MocFolder ()
UiFolder ()

QT4_WRAP_CPP (MOC_SRCS ${H_FILES})

set (SOURCE_FILES ${CPP_FILES} ${H_FILES} ${MOC_SRCS})

use_package (ZZIPLIB)
#use_package (ZLIB)

use_core_modules (Framework Asset)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES})

link_modules (Framework Asset)

link_package (ZZIPLIB)
#link_package (ZLIB)

SetupCompileFlags()

final_target ()

