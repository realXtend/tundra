
# Macro for cleaning out the install directory before install, this is needed because module setup for the build might have changed
# from the time install was last ran. This macro is automatically called from the main CMakeLists.txt.
#
# NOTE: This macro needs to be called before any other install macros or cmake install(...) calls are done, so its the first step!       
#
macro (setup_clean_install_step)
    # Never do the recursive remove to the install dir on other systems than windows.
    # This might do horrible things if you set CMAKE_INSTALL_PREFIX to /usr or similar.
    if (WIN32)
        install (CODE "message(\"\nCleaning target directory ${CMAKE_INSTALL_PREFIX}\")")
        install (CODE "file (REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX})")
        install (CODE "message(\" \")")
    endif()
endmacro ()

# Macro for installing a directory into the install prefix.
# Mean to be used for making sure your projects data files will be installed along with the build results.
# DATA_SRC_DIR_PATH: The relative or absolute path of a directory you want to copy. (relative to your cmakelists.txt dir)
# ARGV1: Optional path in the install destination.
#
# Example: setup_install_directory (media) will copy <project_dir>/media to <prefix>/bin/media
#
function (setup_install_directory)
    # Resolve destination dir
    if (ARGC EQUAL 0)
        message(FATAL_ERROR "setup_install_directory() called without parameters. At minumum give the folder you want to install.")
    endif ()
    
    if (ARGC EQUAL 1)
        set (DATA_DEST_DIR_PATH_FINAL "bin")
    else ()
        set (DATA_DEST_DIR_PATH_FINAL "bin/${ARGV1}")
    endif ()
    # Install directory
    install (CODE "message(\"\nCopying directory ${ARGV0} to ${CMAKE_INSTALL_PREFIX}/${DATA_DEST_DIR_PATH_FINAL}\")")
    install (DIRECTORY ${ARGV0} DESTINATION ${DATA_DEST_DIR_PATH_FINAL})
    install (CODE "message(\" \")")
endfunction ()

# Macro for installing a file into the install prefix.
# Mean to be used for making sure your projects data files will be installed along with the build results.
# DATA_SRC_FILES: Absolute or relative path to file.
# ARGV1: Optional path in the install destination.
#
# Example:  setup_install_file (ui/LoginWidget.ui "data/ui") will copy <project_dir>/ui/LoginWidget.ui to <prefix>/bin/data/ui
# Example:  file (GLOB MY_INSTALL_FILES "config/*.cfg")
#           setup_install_file ("${MY_INSTALL_FILES}") will copy <project_dir>/config/myconfig.cfg to <prefix>/bin 
#           NOTE: Must have " " around ${MY_INSTALL_FILES} for lists! seem to be needed if you 
#           are not using ARGN which we cant coz we have multiple params (right?)
#
function (setup_install_files)   
    # Resolve destination dir
    if (ARGC EQUAL 1)
        set (DATA_DEST_PATH_FINAL "bin")
    else ()
        set (DATA_DEST_PATH_FINAL "bin/${ARGV1}")
    endif ()
    # Install file
    install (FILES ${ARGV0} DESTINATION ${DATA_DEST_PATH_FINAL})
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
        install (CODE "message(\"\nCopying files with ${FIND_DIR}/${FIND_GLOB} to ${CMAKE_INSTALL_PREFIX}/${DATA_DEST_PATH_FINAL}\")")
        install (FILES ${FOUND_INSTALL_FILES} 
                 DESTINATION ${DATA_DEST_PATH_FINAL}
                 CONFIGURATIONS ${FIND_INSTALL_CONFIGURATION})
        install (CODE "message(\" \")")
    endif ()
endfunction ()

# Mark windows dependencies for the install step with a regular expression. This has to give a number of parameters for playing nice.
# DEP_INSTALL_DIR: Relative install path inside the target directory. Use empty string "" for root.
# RELATIVE_DEP_DIR: Relative path where to look inside ${ENV_TUNDRA_DEP_PATH} eg. "qt/bin/". NOTE: This _must_ have trailing slash "/" othewise it will also copy the source directory.
# DLL_REGEXP: Regular expression
# CONFIGURATION: Configuration to apply the install step eg. Release|RelWithDebInfo|Debug or Release.
#
# Optional parameters: You can give another set with a different configuration. DEP_INSTALL_DIR can only be given once, both sets go to that directory.
#
# Example: setup_install_windows_deps_regexp (ogre-safe-nocrashes/bin/release "RenderSystem_+.*[^d][.]dll$" Release ogre-safe-nocrashes/bin/debug "RenderSystem_+.*[d][.]dll$" Debug)
#
function (setup_install_windows_deps_regexp DEP_INSTALL_DIR RELATIVE_DEP_DIR DLL_REGEXP CONFIGURATION) # Optional params: RELATIVE_DEP_DIR2 DLL_REGEXP2 CONFIGURATION2 
    if (WIN32)
        if ("${DEP_INSTALL_DIR}" STREQUAL "")
            SET (DEP_INSTALL_DIR "bin")
        else ()
            set (DEP_INSTALL_DIR "bin/${DEP_INSTALL_DIR}")
        endif ()

        install (DIRECTORY ${ENV_TUNDRA_DEP_PATH}/${RELATIVE_DEP_DIR}
                 DESTINATION ${DEP_INSTALL_DIR}
                 CONFIGURATIONS ${CONFIGURATION}
                 FILES_MATCHING REGEX ${DLL_REGEXP})
        if (ARGV4 AND ARGV5 AND ARGV6)
            install (DIRECTORY ${ENV_TUNDRA_DEP_PATH}/${ARGV4}
                     DESTINATION ${DEP_INSTALL_DIR}
                     CONFIGURATIONS ${ARGV6}
                     FILES_MATCHING REGEX ${ARGV5})
        endif ()
    endif()
endfunction ()

# Mark windows dependencies for the install step by ; delimited file list (if multiple). Source path is ${TUNDRA_BIN}.
#
# Examples:
# setup_install_windows_deps ("cg.dll" Release|RelWithDebInfo|Debug)
# setup_install_windows_deps ("OgreMain.dll;RenderSystem_Direct3D9.dll" Release|RelWithDebInfo "OgreMain_d.dll;RenderSystem_Direct3D9_d.dll" Debug)
#
function (setup_install_windows_deps DLL_FILES DLL_CONFIGURATION) # Optional params: DLL_FILES2 DLL_CONFIGURATION2
    if (WIN32)
        foreach(DLL_FILE ${DLL_FILES})
            install (FILES ${TUNDRA_BIN}/${DLL_FILE}
                     DESTINATION "bin"
                     CONFIGURATIONS ${DLL_CONFIGURATION})
        endforeach ()
        if (ARGV2 AND ARGV3)
            foreach(DLL_FILE ${ARGV2})
                install (FILES ${TUNDRA_BIN}/${DLL_FILE}
                         DESTINATION "bin"
                         CONFIGURATIONS ${ARGV3})
            endforeach ()
        endif ()
    endif ()
endfunction ()

# Macro removes file from install <prefix>
# Handy for removing files you don't want in the installation after setup_install_directory() or setup_install_files_find().
#
# Example: setup_remove_file ("data/assets/dummy.txt")
#
function (setup_remove_file FILE_PATH)
    install (CODE "message(\"\nRemoving file ${CMAKE_INSTALL_PREFIX}/bin/${FILE_PATH}\")")
    install (CODE "file (REMOVE ${CMAKE_INSTALL_PREFIX}/bin/${FILE_PATH})")
    install (CODE "message(\" \")")
endfunction ()

# Removes directory and its content recursively from install <prefix>
# Handy for removing directories you don't want in the installation after setup_install_directory() or setup_install_files_find().
#
# Example: setup_remove_directory("data/assets/translations")
#
function(setup_remove_directory FILE_PATH)
    install(CODE "message(\"\nRemoving directory ${CMAKE_INSTALL_PREFIX}/bin/${FILE_PATH}\")")
    install(CODE "file (REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX}/bin/${FILE_PATH})")
    install(CODE "message(\" \")")
endfunction()

# Macro for installing the target build results into the install prefix.
# This macro gets called automatically in final_target() for all projects.
# - shared libraries    <prefix>/bin/plugins
# - executables         <prefix>/bin
# - static libraries    <prefix>/lib
macro (setup_install_target)
    # Shared libraries
    if (${TARGET_LIB_TYPE} STREQUAL "SHARED")
        # assume /plugins for shared libs, if not plugins assume root.
        set (INSTALL_SHARED_TARGET "plugins")
        if (NOT "${TARGET_OUTPUT}" STREQUAL "plugins")
            set (INSTALL_SHARED_TARGET "")
        endif ()
        install (TARGETS ${TARGET_NAME} 
                 LIBRARY DESTINATION "bin/${INSTALL_SHARED_TARGET}" # non DLL platforms shared libs are LIBRARY
                 RUNTIME DESTINATION "bin/${INSTALL_SHARED_TARGET}" # DLL platforms shared libs are RUNTIME
                 CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES})

        if (NOT INSTALL_BINARIES_ONLY)
            install(TARGETS ${TARGET_NAME} ARCHIVE DESTINATION "lib" CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}) # DLL platforms static link part of the shared lib are ARCHIVE
        endif()
    endif()

    # Static libraries
    if (NOT INSTALL_BINARIES_ONLY)
        if (${TARGET_LIB_TYPE} STREQUAL "STATIC")
            install (TARGETS ${TARGET_NAME} ARCHIVE DESTINATION "lib" CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES})
        endif()
    endif()

    # Executables
    if (${TARGET_LIB_TYPE} STREQUAL "EXECUTABLE")
        install (TARGETS ${TARGET_NAME} RUNTIME DESTINATION "bin" CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES})
    endif()

    # Headers
    if (NOT INSTALL_BINARIES_ONLY)
        # install headers, directly from current source dir and look for subfolders with headers
        file (GLOB_RECURSE TARGET_INSTALL_HEADERS "*.h")
        file (GLOB_RECURSE TARGET_INSTALL_INLINE "*.inl")
        install (FILES ${TARGET_INSTALL_HEADERS} DESTINATION "include/${TARGET_NAME}")
        install (FILES ${TARGET_INSTALL_INLINE} DESTINATION "include/${TARGET_NAME}")
    endif()
endmacro()
