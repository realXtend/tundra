
# Macro for cleaning out the install directory before install, this is needed because module setup for the build might have changed
# from the time install was last ran. This macro is automatically called from the main CMakeLists.txt.
#
# NOTE: This macro needs to be called before any other install macros or cmake install(...) calls are done, so its the first step!       
#
macro (setup_clean_install_step)
    install (CODE "message(STATUS \"Cleaning install directory: \" ${CMAKE_INSTALL_PREFIX})")
    install (CODE "file (REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX})")
endmacro ()

# Macro for installing a directory into the install prefix.
# Mean to be used for making sure your projects data files will be installed along with the build results.
# DATA_SRC_DIR_PATH: The relative or absolute path of a directory you want to copy. (relative to your cmakelists.txt dir)
# ARGV1: Optional path in the install destination.
#
# Example: setup_install_directory (media) will copy <project_dir>/media to <prefix>/bin/media
#
function (setup_install_directory DATA_SRC_DIR_PATH)
    # Resolve destination dir
    if (NOT ARGV1)
        set (DATA_DEST_DIR_PATH_FINAL "bin")
    else ()
        set (DATA_DEST_DIR_PATH_FINAL "bin/${ARGV1}")
    endif ()
    # Install directory
    install (DIRECTORY ${DATA_SRC_DIR_PATH} DESTINATION ${DATA_DEST_DIR_PATH_FINAL})
endfunction ()

# Macro for installing a file into the install prefix.
# Mean to be used for making sure your projects data files will be installed along with the build results.
# DATA_SRC_FILES: Absolute or relative path to file.
# ARGV1: Optional path in the install destination.
#
# Example:  setup_install_file (ui/LoginWidget.ui "data/ui") will copy <project_dir>/ui/LoginWidget.ui to <prefix>/bin/data/ui
# Example:  file (GLOB MY_INSTALL_FILES "config/*.cfg")
#           setup_install_file ("${MY_INSTALL_FILES}") will copy <project_dir>/config/myconfig.cfg to <prefix>/bin 
#           NOTE: Must have" " around ${MY_INSTALL_FILES} for lists! seem to be needed if you 
#           are not using ARGN which we cant coz we have multiple params (right?)
#
function (setup_install_files DATA_SRC_FILES)   
    # Resolve destination dir
    if (NOT ARGV1)
        set (DATA_DEST_PATH_FINAL "bin")
    else ()
        set (DATA_DEST_PATH_FINAL "bin/${ARGV1}")
    endif ()
    # Install file
    install (FILES ${DATA_SRC_FILES} DESTINATION ${DATA_DEST_PATH_FINAL})
endfunction ()

# Macro for finding install files from a dir with a glob.
# ARGV0: Search directory path
# ARGV1: Search GLOB
# ARGV2: OPTIONAL Configurations to install (Release|Debug|RelWithDebInfo)
# ARGV3: OPTIONAL Destination directory in install <prefix>. If you give this param you need to give at least "" to ARGV2.
#
# Examples:
# setup_install_files_find("project-config/generic" "*.xml" "" "config/xml")
# setup_install_files_find("." "plugins.cfg" Release|RelWithDebInfo)
# setup_install_files_find("debug-configs" "pluginsd.cfg" Debug)
#
function (setup_install_files_find FIND_DIR FIND_GLOB)   
    # Find files
    file (GLOB FOUND_INSTALL_FILES "${FIND_DIR}/${FIND_GLOB}")
    
    if (FOUND_INSTALL_FILES)
        # All configurations if not defined
        if (ARGV2)
            set (FIND_INSTALL_CONFIGURATION ${ARGV2})
        else ()
            set (FIND_INSTALL_CONFIGURATION ${CMAKE_CONFIGURATION_TYPES})
        endif ()
        
        # Resolve destination dir
        if (NOT ARGV3)
            set (DATA_DEST_PATH_FINAL "bin")
        else ()
            set (DATA_DEST_PATH_FINAL "bin/${ARGV3}")
        endif ()
    
        # Install file
        install (FILES ${FOUND_INSTALL_FILES} 
                 DESTINATION ${DATA_DEST_PATH_FINAL}
                 CONFIGURATIONS ${FIND_INSTALL_CONFIGURATION})
    endif ()
endfunction ()

# Mark windows dependencies for the install step with a regular expression.
#
# Example: setup_install_windows_deps_regexp ("RenderSystem_+.*[^d][.]dll$" Release|RelWithDebInfo "RenderSystem_+.*[d][.]dll$" Debug)
#
function (setup_install_windows_deps_regexp DLL_REGEXP CONFIGURATION) # Optional params: DLL_REGEXP2 CONFIGURATION2
    if (WIN32)
        message("RegExp 1: " ${DLL_REGEXP})
        message("Config 1: " ${CONFIGURATION})
        message("\nRegExp 2: " ${ARGV2})
        message("Config 2: " ${ARGV3})
        message("")
        
        install (DIRECTORY ${ENV_TUNDRA_DEP_PATH}/../runtime_deps/
                 DESTINATION "bin"
                 CONFIGURATIONS ${CONFIGURATION}
                 FILES_MATCHING REGEX ${DLL_REGEXP})
        if (ARGV2 AND ARGV3)
            install (DIRECTORY ${ENV_TUNDRA_DEP_PATH}/../runtime_deps/
                     DESTINATION "bin"
                     CONFIGURATIONS ${ARGV3}
                     FILES_MATCHING REGEX ${ARGV2})
        endif ()
    endif()
endfunction ()

# Mark windows dependencies for the install step by ; delimited file list.
#
# Examples:
# setup_install_windows_deps ("cg.dll" Release|RelWithDebInfo|Debug)
# setup_install_windows_deps ("OgreMain.dll;RenderSystem_Direct3D9.dll" Release|RelWithDebInfo "OgreMain_d.dll;RenderSystem_Direct3D9_d.dll" Debug)
#
function (setup_install_windows_deps DLL_FILES DLL_CONFIGURATION) # Optional params: DLL_REGEXP2 CONFIGURATION2
    if (WIN32)
        foreach(DLL_FILE ${DLL_FILES})
            install (FILES ${ENV_TUNDRA_DEP_PATH}/../runtime_deps/${DLL_FILE}
                     DESTINATION "bin"
                     CONFIGURATIONS ${DLL_CONFIGURATION})
        endforeach ()
        if (ARGV2 AND ARGV3)
            foreach(DLL_FILE ${ARGV2})
                install (FILES ${ENV_TUNDRA_DEP_PATH}/../runtime_deps/${DLL_FILE}
                         DESTINATION "bin"
                         CONFIGURATIONS ${ARGV3})
            endforeach ()
        endif ()
    endif ()
endfunction ()

# Macro removes file from install <prefix>
# Handy for removing files you dont want in the installation after setup_install_directory() or setup_install_files_find().
#
# Example: setup_remove_file ("data/assets/dummy.txt")
#
function (setup_remove_file FILE_PATH)
    install (CODE "message(STATUS \"[TUNDRA CUSTOM CODE STEP] Removing file: \" ${FILE_PATH})")
    install (CODE "file (REMOVE ${CMAKE_INSTALL_PREFIX}/bin/${FILE_PATH})")
endfunction ()

# Macro for installing the target build results into the install prefix.
# This macro gets called automatically in final_target() for all projects.
# - shared libraries    <prefix>/bin/plugins
# - executables         <prefix>/bin
# - static libraries    <prefix>/lib
#
macro (setup_install_target)
    # install libraries and executables
    if (${TARGET_LIB_TYPE} STREQUAL "SHARED")
        install (TARGETS ${TARGET_NAME} 
                 LIBRARY DESTINATION "bin/plugins" # non DLL platforms shared libs are LIBRARY
                 RUNTIME DESTINATION "bin/plugins" # DLL platforms shared libs are RUNTIME
                 ARCHIVE DESTINATION "lib" # DLL platforms static link part of the shared lib are ARCHIVE
                 CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES})
    endif ()
    if (${TARGET_LIB_TYPE} STREQUAL "STATIC")
        install (TARGETS ${TARGET_NAME} 
                 ARCHIVE DESTINATION "lib" 
                 CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES})
    endif ()
    if (${TARGET_LIB_TYPE} STREQUAL "EXECUTABLE")
        install (TARGETS ${TARGET_NAME} 
                 RUNTIME DESTINATION "bin" 
                 CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES})
    endif ()
    
    # install headers, directly from current source dir and look for subfolders with headers
    file (GLOB_RECURSE TARGET_INSTALL_HEADERS "*.h")
    file (GLOB_RECURSE TARGET_INSTALL_INLINE "*.inl")
    install (FILES ${TARGET_INSTALL_HEADERS} DESTINATION "include/${TARGET_NAME}")
    install (FILES ${TARGET_INSTALL_INLINE} DESTINATION "include/${TARGET_NAME}")
endmacro ()
