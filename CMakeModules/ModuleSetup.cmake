# Naali build macros
#
# Generally used as follows:
# 1. call configure_${PACKAGE}() once
# 2. call init_target (${NAME}) once per build target
# 3. call build_library/executable() on the source files
# 4. call use_framework_modules() with a list of framework module names
# 4. call use_module_headers() with a list of local module names for includes
# 4. call use_module_libraries() with a list of local module names libraries
# 4. call use_package (${PACKAGE}) once per build target

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
    include_directories ($ENV{NAALI_DEP_PATH}/include)
    link_directories ($ENV{NAALI_DEP_PATH}/lib)

    # if OUTPUT is defined
    if (${TARGET_NAME}_OUTPUT)# AND ${TARGET_NAME}_OUTPUT STREQUAL "OUTPUT")
        
        #message (STATUS "target dir: " ${ARGV2})
        set (TARGET_DIR ${PROJECT_BINARY_DIR}/bin/${ARGV2})

        if (MSVC)
            # export symbols
            add_definitions (-DMODULE_EXPORTS)

            # copy to target directory
            add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${TARGET_DIR})
            add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TargetPath)\" ${TARGET_DIR})
        else ()
            # copy to target directory
            set (LIBRARY_OUTPUT_PATH ${TARGET_DIR})
            set (EXECUTABLE_OUTPUT_PATH ${TARGET_DIR})
        endif ()
    endif ()

    # Disable warnings
    if (MSVC)
        add_definitions (-D_CRT_SECURE_NO_WARNINGS)
    endif ()
endmacro (init_target)

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
    target_link_libraries (${TARGET_NAME} ${${PREFIX}_LIBRARIES})
endmacro (use_package)

# include local framework headers and libraries
macro (use_framework_modules TARGET_NAME)
    set (INTERNAL_FRAMEWORK_DIR "..")
    foreach (module_ ${ARGN})
        include_directories (${INTERNAL_FRAMEWORK_DIR}/${module_})
        target_link_libraries (${TARGET_NAME} ${module_})
    endforeach ()
endmacro (use_framework_modules)

# include local module headers 
macro (use_module_headers TARGET_NAME)
    set (INTERNAL_MODULE_DIR "..")
    foreach (module_ ${ARGN})
        include_directories (${INTERNAL_MODULE_DIR}/${module_})
    endforeach ()
endmacro (use_module_headers)

# include local module libraries
macro (use_module_libraries TARGET_NAME)
    foreach (module_ ${ARGN})
        target_link_libraries (${TARGET_NAME} ${module_})
    endforeach ()
endmacro (use_module_libraries)

# link directories
macro (link_package PREFIX TARGET_NAME)
    target_link_libraries (${TARGET_NAME} ${${PREFIX}_LIBRARIES})
endmacro (link_package)

# =============================================================================
# per-project macros

macro (configure_boost)
    set (Boost_USE_STATIC_LIBS ON)
    sagase_configure_package (BOOST 
        NAMES Boost boost
        COMPONENTS date_time filesystem system thread program_options unit_test_framework
        PREFIXES $ENV{NAALI_DEP_PATH})
endmacro (configure_boost)

macro (configure_poco)
    sagase_configure_package (POCO 
        NAMES Poco PoCo poco
        COMPONENTS Poco PocoFoundation PocoNet PocoUtil PocoXML
        PREFIXES $ENV{NAALI_DEP_PATH})
endmacro (configure_poco)

macro (configure_qt4)
    sagase_configure_package (QT4 
        NAMES Qt4 qt4 Qt qt
        COMPONENTS QtCore QtGui QtWebkit QtScript QtXml QtNetwork QtUiTools
        PREFIXES $ENV{NAALI_DEP_PATH} $ENV{QTDIR})

    # FindQt4.cmake does it's own thing...
    if (QT4_FOUND)

        include (${QT_USE_FILE})

        set (QT4_INCLUDE_DIRS 
            ${QT_INCLUDE_DIR}
            ${QT_QTCORE_INCLUDE_DIR}
            ${QT_QTDBUS_INCLUDE_DIR}
            ${QT_QTGUI_INCLUDE_DIR}
            ${QT_QTUITOOLS_INCLUDE_DIR}
            ${QT_QTNETWORK_INCLUDE_DIR}
            ${QT_QTXML_INCLUDE_DIR}
            ${QT_QTSCRIPT_INCLUDE_DIR}
            ${QT_QTWEBKIT_INCLUDE_DIR}
            ${QT_PHONON_INCLUDE_DIR})

        set (QT4_LIBRARY_DIR  
            ${QT_LIBRARY_DIR})

        set (QT4_LIBRARIES 
            ${QT_LIBRARIES}
            ${QT_QTCORE_LIBRARY}
            ${QT_QTDBUS_LIBRARY}
            ${QT_QTGUI_LIBRARY}
            ${QT_QTUITOOLS_LIBRARY}
            ${QT_QTNETWORK_LIBRARY}
            ${QT_QTXML_LIBRARY}
            ${QT_QTSCRIPT_LIBRARY}
            ${QT_QTWEBKIT_LIBRARY}
            ${QT_PHONON_LIBRARY})
    endif ()
endmacro (configure_qt4)

macro (configure_ois)
    sagase_configure_package (OIS 
        NAMES Ois ois OIS 
        COMPONENTS OIS
        PREFIXES $ENV{NAALI_DEP_PATH})
endmacro (configure_ois)

macro (link_ois ${TARGET_NAME})
    if (MSVC)
        target_link_libraries (${TARGET_NAME} debug OIS_d optimized OIS)
    else ()
        target_link_libraries (${TARGET_NAME} ${OIS_LIBRARIES})
    endif ()
endmacro (link_ois)

macro (configure_ogre)
    sagase_configure_package (OGRE 
        NAMES Ogre OgreSDK ogre OGRE
        COMPONENTS Ogre ogre OGRE OgreMain 
        PREFIXES $ENV{NAALI_DEP_PATH} $ENV{OGRE_HOME})
endmacro (configure_ogre)

macro (link_ogre ${TARGET_NAME})
    if (MSVC)
        target_link_libraries (${TARGET_NAME} debug OgreMain_d optimized OgreMain)
    else ()
        target_link_libraries (${TARGET_NAME} ${OGRE_LIBRARIES})
    endif ()
endmacro (link_ogre)
