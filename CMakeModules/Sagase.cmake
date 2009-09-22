# -- Ryan McDougall, realXtend -- 2009

# parse a keyword delimited list of arguments into single list
# results are in OUT
function (sagase_parse_arguments BEGIN END ARGS)
    set (take FALSE)
    foreach (var ${ARGN})
        if (${var} STREQUAL ${BEGIN})
            set (take TRUE)
        elseif (${var} STREQUAL ${END})
            set (take FALSE)
        else ()
            if (take)
                set (out ${out} ${var})
            endif ()
        endif ()
    endforeach ()

    set (${ARGS} ${out} PARENT_SCOPE)
endfunction (sagase_parse_arguments)


# generates a set of likely paths for a files (includes or libraries)
# "NAMES" is a list of names by which the package is known
# "PREFIXES" is a list of path prefixes where the components might be found
# results are in INCLUDE_PATHS, LIBRARY_PATHS  
function (sagase_generate_paths INCLUDE_PATHS LIBRARY_PATHS)
    sagase_parse_arguments ("NAMES" "PREFIXES" path_names ${ARGN})
    sagase_parse_arguments ("PREFIXES" none path_prefixes ${ARGN})

    if (MSVC)
        set (path_prefixes ${path_prefixes} "C:/")
    elseif (APPLE)
        message (FATAL_ERROR "sagase: MacOS Support incomplete")
    elseif (UNIX)
        set (path_prefixes ${path_prefixes} "/usr" "/usr/local" "/opt")
    else ()
        message (FATAL_ERROR "sagase: Unable to detect OS type")
    endif ()

    # normalizes external paths to cmake-style
    #foreach (prefix ${path_prefixes})
    #    file (TO_CMAKE_PATH "${prefix}" p)
    #    set (normal_path_prefixes ${normal_path_prefixes} ${p})
    #endforeach ()

    set (include_names "include" "includes")
    set (lib_names "lib" "libs" "bin" "bins" "dll" "dlls")

    # add prefix+name paths
    foreach (prefix ${path_prefixes})
        foreach (pkgname ${path_names})

            foreach (inclname ${include_names})
                set (includes ${includes} 
                    ${prefix}/${pkgname}/${inclname}
                    ${prefix}/${inclname}/${pkgname})
                endforeach ()

            foreach (libname ${lib_names})
                set (libraries ${libraries} 
                    ${prefix}/${pkgname}/${libname} 
                    ${prefix}/${libname}/${pkgname})
                endforeach ()

        endforeach ()
    endforeach ()

    set (${INCLUDE_PATHS} ${includes} PARENT_SCOPE)
    set (${LIBRARY_PATHS} ${libraries} PARENT_SCOPE)

endfunction (sagase_generate_paths)

# tries a series of methods to find correct compile and link info.
# "NAMES" is a list of names by which the package is known. used by
# find_package and in constructing search path names.
# "COMPONENTS" is a list of sub-components that are required. used by
# find_package and by pkg-config.
# "PREFIXES" is a list of path prefixes where the components might be found.
# "SKIP_FIND_PACKAGE" causes it to bypass find_package
# results are in ${PREFIX}_INCLUDE_DIRS, ${PREFIX}_LIBRARY_DIRS, 
# ${PREFIX}_LIBRARIES, ${PREFIX}_DEFINITIONS, or fatal error.

# Tryies the following methods to find a package, in order of decreasing
# specialization or detail:
# 1. find_package
# 2. OS-dependent (pkg-config, framework, etc.)
# 3. find_path 

# "NAMES" and "COMPONENTS" mean slightly different things depending on the
# seach methond. For find_package, consult the cmake documentation. For
# pkg-config, only the "NAMES" are used. For find_path/brute force, "NAMES"
# are possible directory names, and "COMPONENTS" are the possible header or
# library filenames.

# example usages: 
# sagase_configure_package (BOOST 
#     NAMES BOOST Boost 
#     COMPONENTS date_time filesystem system thread
#     PREFIXES "C:")
#
# sagase_configure_package (GLIB
#     NAMES glib-2.0
#     COMPONENTS glib glib-2.0)
#
# sagase_configure_package (CAELUM
#     NAMES Caelum
#     COMPONENTS Caelum
#     PREFIXES $ENV{NAALI_DEP_PATH})


macro (sagase_configure_package PREFIX)
    sagase_parse_arguments ("NAMES" "COMPONENTS" PKG_NAMES ${ARGN})
    sagase_parse_arguments ("COMPONENTS" "PREFIXES" PKG_COMPONENTS ${ARGN})
    sagase_parse_arguments ("PREFIXES" none PKG_PREFIXES ${ARGN})

    set (found_ FALSE)

    foreach (name_ ${PKG_NAMES})

        string (TOUPPER ${name_} name_upper_)

        # find_package can't handle packages in all caps.
        # cmake is a macro language, which means it defines global variables,
        # which it tries to namespace by choosing all caps variable names,
        # when searching for a package who's name is all caps, this appears to
        # cause a name collision, and is thus bypassed.
        if (NOT name_ STREQUAL name_upper_)
            message (STATUS "trying find_package: " ${name_})

            # try built-in CMake modules first
            find_package (${name_} QUIET COMPONENTS ${PKG_COMPONENTS})
        endif ()

        if (${name_}_FOUND OR ${name_upper_}_FOUND)
            message (STATUS "sagase: configured " ${PREFIX})
            set (${PREFIX}_INCLUDE_DIRS ${${name_}_INCLUDE_DIRS})
            set (${PREFIX}_LIBRARIES ${${name_}_LIBRARIES})
            set (${PREFIX}_LIBRARY_DIRS ${${name_}_LIBRARY_DIRS})
            set (${PREFIX}_DEFINITIONS ${${name_}_CFLAGS_OTHER})
            set (found_ TRUE)
            break ()

        else ()
            # try system module manager

            if (MSVC)
                # MS has no automatic module management

            elseif (APPLE)
                # I don't know how to use Apple "Framework"
                message (FATAL_ERROR "sagase: " ${PREFIX} ": MacOS Support incomplete")

            elseif (UNIX)
                # what else is there besides pkg-config?
                # non-linux OSes may be a problem
                message (STATUS "tring pkg_check_modules: " ${name_})

                include (FindPkgConfig)
                if (PKG_CONFIG_FOUND)
                    pkg_check_modules(${PREFIX} ${name_})
                else ()
                    message (STATUS "pkg-config cannot be found.")
                endif ()

                if (${PREFIX}_FOUND)
                    message (STATUS "sagase: configured " ${PREFIX})
                    # already set: ${PREFIX}_INCLUDE_DIRS, 
                    # ${PREFIX}_LIBRARY_DIRS, ${PREFIX}_LIBRARIES
                    set (${PREFIX}_DEFINITIONS ${${PREFIX}_CFLAGS_OTHER})
                    set (found_ TRUE)
                    break ()
                endif ()

            else ()
                message (FATAL_ERROR "sagase: " ${PREFIX} ": Unable to detect OS type")
            endif ()

        endif ()
    endforeach ()

    if (NOT found_)
        message (STATUS "trying brute-force search ")

        # take names to be directory names, and include "."
        set (${PREFIX}_PATH_NAMES ${PKG_NAMES} ".")

        # generate a combination of possible search paths
        sagase_generate_paths (include_paths library_paths NAMES ${${PREFIX}_PATH_NAMES} PREFIXES ${PKG_PREFIXES})

        # all C and C++ headers
        set (HEADER_POSTFIXES ".h" ".hpp" ".hh" ".hxx")

        # follow platform library naming
        if (MSVC)
            set (LIB_PREFIX "")
            set (LIB_POSTFIXES ".lib")
        elseif (UNIX)
            set (LIB_PREFIX "lib")
            set (LIB_POSTFIXES ".so" ".a")
        else ()
            message (FATAL_ERROR "sagase: " ${PREFIX} ": Unable to detect OS type")
        endif ()

        # try using "COMPONENTS" as possible file names (without prefix or extension)
        foreach (component_ ${PKG_COMPONENTS})
            
            # get header path
            foreach (pathnames_ ${${PREFIX}_PATH_NAMES})
                foreach (header_extension_ ${HEADER_POSTFIXES})
                    find_path (${PREFIX}_${component_}_INCLUDE_DIR ${pathnames_}/${component_}${header_extension_} ${include_paths})
                    
                    if (${PREFIX}_${component_}_INCLUDE_DIR)
                        set (${PREFIX}_INCLUDE_DIRS ${${PREFIX}_INCLUDE_DIRS} ${${PREFIX}_${component_}_INCLUDE_DIR})
                    endif ()
                endforeach ()
            endforeach ()

            # get library path
            foreach (lib_extension_ ${LIB_POSTFIXES})
                find_path (${PREFIX}_${component_}_LIBRARY_DIR ${LIB_PREFIX}${component_}${lib_extension_} ${library_paths})

                if (${PREFIX}_${component_}_LIBRARY_DIR)
                    set (${PREFIX}_LIBRARIES ${${PREFIX}_LIBRARIES} ${component_})
                    set (${PREFIX}_LIBRARY_DIRS ${${PREFIX}_LIBRARY_DIRS} ${${PREFIX}_${component_}_LIBRARY_DIR})
                endif ()
            endforeach ()
        endforeach ()
    endif ()

    # stop process if nothing is found through any means
    if (NOT found_ AND NOT ${PREFIX}_INCLUDE_DIRS AND NOT ${PREFIX}_LIBRARY_DIRS AND NOT ${PREFIX}_LIBRARIES)
        message (FATAL_ERROR "!! sagase: unable to configure " ${PREFIX}) 
    endif ()
    
    # remove duplicate entires from return variables
    if (${PREFIX}_INCLUDE_DIRS)
        list (REMOVE_DUPLICATES ${PREFIX}_INCLUDE_DIRS)
    endif ()

    if (${PREFIX}_LIBRARY_DIRS)
        list (REMOVE_DUPLICATES ${PREFIX}_LIBRARY_DIRS)
    endif ()

    if (${PREFIX}_LIBRARIES)
        list (REMOVE_DUPLICATES ${PREFIX}_LIBRARIES)
    endif ()

    # report to screen what was found
    message (STATUS "sagase: " ${PREFIX} " Configure Results.")
    message (STATUS "-- Include Directories: " ${${PREFIX}_INCLUDE_DIRS})
    message (STATUS "-- Libarary Directories: " ${${PREFIX}_LIBRARY_DIRS})
    message (STATUS "-- Libraries: " ${${PREFIX}_LIBRARIES})
    message (STATUS "-- Defines: " ${${PREFIX}_DEFINITIONS})

endmacro (sagase_configure_package)
