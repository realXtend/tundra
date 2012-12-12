# Tundra build macros
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

    # Define target name and output directory.
    # Skip ARGV1 that is the keyword OUTPUT.
    set (TARGET_NAME ${NAME})
    set (TARGET_OUTPUT ${ARGV2})
    
    message ("** " ${TARGET_NAME})

    # Headers or libraries are found here will just work
    # Removed from windows due we dont have anything here directly.
    # On linux this might have useful things but still ConfigurePackages should
    # find them properly so essentially this is not needed.
    if (NOT WIN32)
        include_directories (${ENV_TUNDRA_DEP_PATH}/include)
        link_directories (${ENV_TUNDRA_DEP_PATH}/lib)
    endif ()
    
    # Include our own module path. This makes #include "x.h" 
    # work in project subfolders to include the main directory headers
    # note: CMAKE_INCLUDE_CURRENT_DIR could automate this
    include_directories (${CMAKE_CURRENT_SOURCE_DIR})
    
    # Add the SDK static libs build location for linking
    link_directories (${PROJECT_BINARY_DIR}/lib)
    
    # set TARGET_DIR
    if (NOT "${TARGET_OUTPUT}" STREQUAL "")
        set (TARGET_DIR ${PROJECT_BINARY_DIR}/bin/${TARGET_OUTPUT})
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
            if (APPLE)
                set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flat_namespace")
            endif()
            set_target_properties (${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${TARGET_DIR})
            set_target_properties (${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TARGET_DIR})
        endif ()
    endif ()
    
    # pretty printing
    message ("")
    
    # run the setup install macro for everything included in this build
    setup_install_target ()
    
endmacro (final_target)

# build a library from internal sources
macro (build_library TARGET_NAME LIB_TYPE)

    # On Android force all libs to be static, except the main lib (Tundra)
    if (ANDROID AND ${LIB_TYPE} STREQUAL "SHARED" AND NOT (${TARGET_NAME} STREQUAL "Tundra"))
        message("- Forcing lib to static for Android")
        set (TARGET_LIB_TYPE "STATIC")
    else ()
        set (TARGET_LIB_TYPE ${LIB_TYPE})
    endif ()

    message (STATUS "-- build type:")
    message (STATUS "       " ${TARGET_LIB_TYPE} " library")
   
    # *unix add -fPIC for static libraries
    if (UNIX AND ${TARGET_LIB_TYPE} STREQUAL "STATIC")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    endif ()

    add_library (${TARGET_NAME} ${TARGET_LIB_TYPE} ${ARGN})

    if (MSVC AND ENABLE_BUILD_OPTIMIZATIONS)
        if (${TARGET_LIB_TYPE} STREQUAL "SHARED")
            set_target_properties (${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})
            set_target_properties (${TARGET_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})
        else ()
            set_target_properties (${TARGET_NAME} PROPERTIES STATIC_LIBRARY_FLAGS_RELEASE "/LTCG")
            set_target_properties (${TARGET_NAME} PROPERTIES STATIC_LIBRARY_FLAGS_RELWITHDEBINFO "/LTCG")
        endif ()
    endif ()

    if (NOT ANDROID)
        # build static libraries to /lib if
        # - Is part of the SDK (/src/Core/)
        # - Is a static EC declared by SDK on the build (/src/EntityComponents/)
        if (${TARGET_LIB_TYPE} STREQUAL "STATIC" AND NOT TARGET_DIR)
            string (REGEX MATCH  ".*/src/Core/?.*" TARGET_IS_CORE ${CMAKE_CURRENT_SOURCE_DIR})
            string (REGEX MATCH  ".*/src/EntityComponents/?.*" TARGET_IS_EC ${CMAKE_CURRENT_SOURCE_DIR})
            if (TARGET_IS_CORE)
                message (STATUS "-- SDK lib output path:")
            elseif (TARGET_IS_EC)
                message (STATUS "-- SDK EC lib output path:")
            endif ()
            if (TARGET_IS_CORE OR TARGET_IS_EC)
                message (STATUS "       " ${PROJECT_BINARY_DIR}/lib)
                set_target_properties (${TARGET_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
            endif ()
        endif ()
    endif ()

    # internal library naming convention
    set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)
    if (NOT ANDROID OR NOT (${TARGET_NAME} STREQUAL "Tundra"))
        set_target_properties (${TARGET_NAME} PROPERTIES PREFIX "")
    endif ()
    set_target_properties (${TARGET_NAME} PROPERTIES LINK_INTERFACE_LIBRARIES "")

endmacro (build_library)

# build an executable from internal sources 
macro (build_executable TARGET_NAME)

    set (TARGET_LIB_TYPE "EXECUTABLE")
    message (STATUS "building executable: " ${TARGET_NAME})

    add_executable (${TARGET_NAME} ${ARGN})
    if (MSVC)
        target_link_libraries (${TARGET_NAME} optimized dbghelp.lib)
    endif ()

    set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d)

endmacro (build_executable)

# include and lib directories, and definitions
macro (use_package PREFIX)
    message (STATUS "-- using " ${PREFIX})
    add_definitions (${${PREFIX}_DEFINITIONS})
    include_directories (${${PREFIX}_INCLUDE_DIRS})
    link_directories (${${PREFIX}_LIBRARY_DIRS})
endmacro (use_package)

# Macro for using modules outside the normal src/{Core|Application}/Project modules: 
# include module header dir and add link dir. Takes in list of relative paths to the modules.
# Example:      use_modules(mycompany/plugins/OurModuleOne 3rdParty/AnotherModule)
#               link_modules(OurModuleOne AnotherModule)
macro (use_modules)
    message (STATUS "-- using modules:")
    foreach (modulePath_ ${ARGN})
        message (STATUS "       " ${modulePath_})
        include_directories (${modulePath_})
        link_directories (${modulePath_})
    endforeach ()
endmacro (use_modules)

# Macro for src/Core modules: include local module headers and add link directory
# Example:      use_core_modules(Framework Input Ui)
#               link_modules(Framework Input Ui)
macro (use_core_modules)
    message (STATUS "-- using Core modules:")
    set (INTERNAL_MODULE_DIR ${PROJECT_SOURCE_DIR}/src/Core)
    foreach (module_ ${ARGN})
        message (STATUS "       " ${module_})
        include_directories (${INTERNAL_MODULE_DIR}/${module_})
        link_directories (${INTERNAL_MODULE_DIR}/${module_})
    endforeach ()
endmacro (use_core_modules)

# Macro for src/Application modules: include local module headers and add link directory
# Example:      use_app_modules(JavascripModule)
#               link_modules(JavascripModule)
macro (use_app_modules)
    message (STATUS "-- using Application modules:")
    set (INTERNAL_MODULE_DIR ${PROJECT_SOURCE_DIR}/src/Application)
    foreach (module_ ${ARGN})
        message (STATUS "       " ${module_})
        include_directories (${INTERNAL_MODULE_DIR}/${module_})
        link_directories (${INTERNAL_MODULE_DIR}/${module_})
    endforeach ()
endmacro (use_app_modules)

# Macro for EC include and link directory addition.
# The EC list can have items from src/EntityComponents/ or any relative path from the Tundra source tree root.
# note: You should not use this directly, use link_entity_components that will call this when needed.
# Example:      use_entity_components(EC_Sound 3rdparty/myecs/EC_Thingie)
macro (use_entity_components)
    message (STATUS "-- using Entity-Components:")
    set (INTERNAL_MODULE_DIR ${PROJECT_SOURCE_DIR}/src/EntityComponents)
    foreach (entityComponent_ ${ARGN})
        if (IS_DIRECTORY ${INTERNAL_MODULE_DIR}/${entityComponent_})
            set (_compNameInternal ${entityComponent_})
            include_directories (${INTERNAL_MODULE_DIR}/${entityComponent_})
            link_directories (${INTERNAL_MODULE_DIR}/${entityComponent_})
        elseif (IS_DIRECTORY ${PROJECT_BINARY_DIR}/${entityComponent_})
            GetLastElementFromPath(${entityComponent_} _compNameInternal)
            include_directories (${PROJECT_BINARY_DIR}/${entityComponent_})
            link_directories (${PROJECT_BINARY_DIR}/${entityComponent_})
        else ()
            message(FATAL_ERROR "Could not resolve use_entity_components() call with " ${entityComponent_} ". Are you sure the component is there?")
        endif ()
        message (STATUS "       " ${_compNameInternal})
    endforeach ()
endmacro (use_entity_components)

# Links the current project to the given EC, if that EC has been added to the build. Otherwise omits the EC.
# The EC list can have items from src/EntityComponents/ or any relative path from the Tundra source tree root.
# Example:      link_entity_components(EC_Sound 3rdparty/myecs/EC_Thingie)
macro(link_entity_components)
    # Link and track found components
    set (foundComponents "")
    foreach(componentName ${ARGN})
        # Determine if this is a component under the usual static EC dir
        # or a custom EC that is in a relative path.
        if (IS_DIRECTORY ${PROJECT_SOURCE_DIR}/src/EntityComponents/${componentName})
            set (_compNameInternal ${componentName})
        elseif (IS_DIRECTORY ${PROJECT_BINARY_DIR}/${componentName})
            GetLastElementFromPath(${componentName} _compNameInternal)
        else ()
            message(FATAL_ERROR "Could not resolve link_entity_components() call with " ${componentName} ". Are you sure the component is there?")
        endif ()
        
        # Check if the component is included in the build
        if (${_compNameInternal}_ENABLED)
            # Link to the project folder name
            link_modules(${_compNameInternal})
            # Add the input 'path' to list of component we are using includes from
            # 1. Its either a folder name under src/EntityComponents/componentName
            # 2. Its a relative path from project binary dir <clone>/path/componentPath
            set (foundComponents ${foundComponents} ${componentName})
            add_definitions(-D${componentName}_ENABLED)
        endif()
    endforeach ()
    # Include the ones that were found on this build
    if (foundComponents)
        use_entity_components(${foundComponents})
    endif ()
endmacro(link_entity_components)

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
			FILE(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/bin/data/translations/tundra_translations.pro "SOURCES += ${file} \n")
		else()
			FILE(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/bin/data/translations/tundra_translations.pro "SOURCES = ${file} \n")
			SET(CREATED_PRO_FILE "true")
		endif()
	endforeach()
	
	file (GLOB PRO_FILE bin/data/translations/*.pro)
	
	foreach(ts_file ${${TRANSLATION_FILES}})
		execute_process(COMMAND ${QT_LUPDATE_EXECUTABLE} -silent ${PRO_FILE} -ts ${ts_file} )
	endforeach()
	
	FILE(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/bin/data/translations/tundra_translations.pro)
	
endmacro()

# Update current qm files.
macro (update_qm_files TRANSLATION_FILES)
	foreach(file ${${TRANSLATION_FILES}})
		get_filename_component(name ${file} NAME_WE)
		execute_process(COMMAND ${QT_LRELEASE_EXECUTABLE} -silent ${file} -qm ${CMAKE_CURRENT_SOURCE_DIR}/bin/data/translations/${name}.qm)
	endforeach()
endmacro()