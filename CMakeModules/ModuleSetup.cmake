# Naali build macros

# =============================================================================
# reusable macros

# define target name, and directory, if it should be output
# ARGV1 is directive to output, and ARGV2 is where to
macro (init_target NAME)

    # Define target name and output directory
    set (TARGET_NAME ${NAME})
    set (${TARGET_NAME}_OUTPUT ${ARGV1})

    message (STATUS "Found build target: " ${TARGET_NAME})

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

# include and lib directories, and definitions
macro (use_package PREFIX)
    include_directories (${${PREFIX}_INCLUDE_DIRS})
    link_directories (${${PREFIX}_LIBRARY_DIRS})
    add_definitions (${${PREFIX}_DEFINITIONS})
endmacro (use_package)

# include and link local modules
macro (use_internal_module MODULES TARGET_NAME)
    foreach (module_ ${MODULES})
        include_directories (../${module_})
        target_link_libraries (${TARGET_NAME} ${module_})
    endforeach ()
endmacro (use_internal_module)

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

macro (link_OIS ${TARGET_NAME})
    if (MSVC)
        target_link_libraries (${TARGET_NAME} debug OIS_d optimized OIS)
    else ()
        target_link_libraries (${TARGET_NAME} ${OIS_LIBRARIES})
    endif ()
endmacro (link_OIS)

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
