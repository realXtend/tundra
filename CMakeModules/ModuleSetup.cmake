# Naali build macros
#
# Generally used as follows:
# 1. call configure_${PACKAGE}() once from the main CMakeLists.txt
# 2. call init_target (${NAME}) once per build target in the module CMakeLists.txt
# 3. call use_package (${PACKAGE}) once per build target
# 3. call use_modules() with a list of local module names for includes
# 4. call build_library/executable() on the source files
# 5. call link_package (${PACKAGE}) once per build target
# 5. call link_modules() with a list of local module names libraries
# 6. call final_target () at the end of build target's cmakelists.txt
#    (not needed for lib targets, only exe's & modules)

# =============================================================================
# reusable macros

# define target name, and directory, if it should be output
# ARGV1 is directive to output, and ARGV2 is where to
macro (init_target NAME)

    # Define target name and output directory
    set (TARGET_NAME ${NAME})
    set (${TARGET_NAME}_OUTPUT ${ARGV1})
    
    message (STATUS "Found build target: " ${TARGET_NAME})

    # headers or libraries are found here will just work
    include_directories (${ENV_NAALI_DEP_PATH}/include)
    link_directories (${ENV_NAALI_DEP_PATH}/lib)

    # set TARGET_DIR
    if (${TARGET_NAME}_OUTPUT)
        set (TARGET_DIR ${PROJECT_BINARY_DIR}/bin/${ARGV2})

        if (MSVC)
            # export symbols, copy needs to be added via copy_target
            add_definitions (-DMODULE_EXPORTS)
        endif ()
    endif ()
endmacro (init_target)

macro (final_target)
    # set TARGET_DIR
    if (TARGET_DIR)
        if (MSVC)
            # copy to target directory
            add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${TARGET_DIR})
            add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TargetPath)\" ${TARGET_DIR})
        else ()
            # set target directory
            set (LIBRARY_OUTPUT_PATH ${TARGET_DIR})
            set (EXECUTABLE_OUTPUT_PATH ${TARGET_DIR})
        endif ()
    endif ()
endmacro (final_target)

# build a library from internal sources
macro (build_library TARGET_NAME LIB_TYPE)

    message (STATUS "building " ${LIB_TYPE} " library: " ${TARGET_NAME})

    if (UNIX AND ${LIB_TYPE} STREQUAL "STATIC")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    endif ()

    add_library (${TARGET_NAME} ${LIB_TYPE} ${ARGN})

    # internal library naming convention
    set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)
    set_target_properties (${TARGET_NAME} PROPERTIES PREFIX "")
    set_target_properties (${TARGET_NAME} PROPERTIES LINK_INTERFACE_LIBRARIES "")

endmacro (build_library)

# build an executable from internal sources 
macro (build_executable TARGET_NAME)

    message (STATUS "building executable: " ${TARGET_NAME})
    
    if (MSVC AND WINDOWS_APP)
        add_executable (${TARGET_NAME} WIN32 ${ARGN})
    else ()
        add_executable (${TARGET_NAME} ${ARGN})
    endif ()

    if (MSVC)
        target_link_libraries (${TARGET_NAME} optimized dbghelp.lib)
    endif (MSVC)

    set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)

endmacro (build_executable)

# include and lib directories, and definitions
macro (use_package PREFIX)
    message (STATUS "-- using " ${PREFIX})
    add_definitions (${${PREFIX}_DEFINITIONS})
    include_directories (${${PREFIX}_INCLUDE_DIRS})
    link_directories (${${PREFIX}_LIBRARY_DIRS})
endmacro (use_package)

# include local module headers
macro (use_modules)
    message (STATUS "-- using modules:")
    set (INTERNAL_MODULE_DIR ${PROJECT_SOURCE_DIR})
    foreach (module_ ${ARGN})
        message (STATUS "       " ${module_})
        include_directories (${INTERNAL_MODULE_DIR}/${module_})
        link_directories (${INTERNAL_MODULE_DIR}/${module_})
    endforeach ()
endmacro (use_modules)

# link directories
macro (link_package PREFIX)
    if (${PREFIX}_DEBUG_LIBRARIES)
        foreach (releaselib_  ${${PREFIX}_LIBRARIES})
            target_link_libraries (${TARGET_NAME} optimized ${releaselib_})
        endforeach ()
        foreach (debuglib_ ${${PREFIX}_DEBUG_LIBRARIES})
            target_link_libraries (${TARGET_NAME} debug ${debuglib_})
        endforeach ()
    else ()
        target_link_libraries (${TARGET_NAME} ${${PREFIX}_LIBRARIES})
    endif ()
endmacro (link_package)

# include local module libraries
macro (link_modules)
    foreach (module_ ${ARGN})
        target_link_libraries (${TARGET_NAME} ${module_})
    endforeach ()
endmacro (link_modules)

# manually find the debug libraries
macro (find_debug_libraries PREFIX DEBUG_POSTFIX)
    foreach (lib_ ${${PREFIX}_LIBRARIES})
        set (${PREFIX}_DEBUG_LIBRARIES ${${PREFIX}_DEBUG_LIBRARIES}
            ${lib_}${DEBUG_POSTFIX})
    endforeach ()
endmacro ()

# Update current translation files. 
macro (update_translation_files TRANSLATION_FILES)
	
	foreach(file ${FILES_TO_TRANSLATE})
		if(CREATED_PRO_FILE)
			FILE(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/bin/data/translations/naali_translations.pro "SOURCES += ${file} \n")
		else()
			FILE(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/bin/data/translations/naali_translations.pro "SOURCES = ${file} \n")
			SET(CREATED_PRO_FILE "true")
		endif()
	endforeach()
	
	file (GLOB PRO_FILE bin/data/translations/*.pro)
	
	foreach(ts_file ${${TRANSLATION_FILES}})
		execute_process(COMMAND ${QT_LUPDATE_EXECUTABLE} -silent ${PRO_FILE} -ts ${ts_file} )
	endforeach()
	
	FILE(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/bin/data/translations/naali_translations.pro)
	
endmacro()

# Update current qm files.
macro (update_qm_files TRANSLATION_FILES)
	foreach(file ${${TRANSLATION_FILES}})
		get_filename_component(name ${file} NAME_WE)
		execute_process(COMMAND ${QT_LRELEASE_EXECUTABLE} -silent ${file} -qm ${CMAKE_CURRENT_SOURCE_DIR}/bin/data/translations/${name}.qm)
	endforeach()
endmacro()