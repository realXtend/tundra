# Define target name and output directory
init_target (Tundra OUTPUT ./)

# Define source files
file (GLOB CPP_FILES main.cpp)
file (GLOB H_FILES "") # Dont do *.h it will pickup windows spesific resource.h
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})
set (RESOURCE_FILES "")

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

# Never compile server in consoleless mode
# Todo: We need to fix this somehow now that server and viewer are in the same project to get a non-shelled viewer!
set (WINDOWS_APP 0)

if (MSVC)
    # Windows resource files for icon, for Visual Studio 2008
    if (VC90)
        set (RESOURCE_FILES ${RESOURCE_FILES} resource.h Tundra.rc)
    endif ()
    
    # Preprocessor defines on MSVC
    set_property (DIRECTORY PROPERTY COMPILE_DEFINITIONS_RELEASE _DMEMDUMP)
    set_property (DIRECTORY PROPERTY COMPILE_DEFINITIONS_RELWITHDEBINFO _DMEMDUMP)

    # Add the DEBUG_CPP_NAME to all other .cpp files but StableHeaders.cpp (would conflict with the #defines in PCH)
    # This define is used to remember the name of the compilation unit for memory leak checking.
    # Also label all files to use a PCH.
    foreach(src_file ${CPP_FILES})
            if (${src_file} MATCHES "StableHeaders.cpp$")
            else()
                get_filename_component(basename ${src_file} NAME)
                set_source_files_properties(${src_file}
                    PROPERTIES COMPILE_FLAGS "-DDEBUG_CPP_NAME=\"\\\"${basename}\"\\\"")
            endif()
    endforeach(src_file ${CPP_FILES})
endif (MSVC)

use_core_modules (Framework Scene Console)

build_executable (${TARGET_NAME} ${SOURCE_FILES} ${RESOURCE_FILES})

link_modules (Framework Scene Console)

final_target ()

# Install files
setup_install_files (${TUNDRA_BIN}/plugins/TundraInstallationDirectory.txt "plugins")   # C++ plugins
setup_install_files_find (${TUNDRA_BIN} "*.xml")    # Tundra executable config xml files
setup_install_files (${TUNDRA_BIN}/../README.md)    # Readme
setup_install_files (${TUNDRA_BIN}/../LICENSE)      # Licence

# Install certain things only in non BUILD_SDK_ONLY mode
if (NOT BUILD_SDK_ONLY)
    setup_install_directory (${TUNDRA_BIN}/data)        # Data: ui, assets, etc
    setup_install_directory (${TUNDRA_BIN}/scenes)      # Example application scenes
    setup_remove_file ("data/assets/dummy.txt")         # Remove git dummy file from data/assets
endif ()

# Install deps for windows
setup_install_windows_deps_regexp ("[/][Q][t][A-Z].*[^d][4][.]dll$" Release|RelWithDebInfo "[/][Q][t][A-Z].*[d][4][.]dll$" Debug)
setup_install_windows_deps_regexp ("[/]qtplugins[/].*[/][q].*[^d][4][.]dll$" Release|RelWithDebInfo "[/]qtplugins[/].*[/][q].*[d][4][.]dll$" Debug)
setup_install_windows_deps ("phonon4.dll" Release|RelWithDebInfo "phonond4.dll" Debug)
