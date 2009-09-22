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
    include_directories (${ENV_NAALI_DEP_PATH}/include)
    link_directories (${ENV_NAALI_DEP_PATH}/lib)

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
endmacro (use_package)

# include local framework headers and libraries
macro (use_framework_modules)
    set (INTERNAL_FRAMEWORK_DIR "..")
    foreach (module_ ${ARGN})
        include_directories (${INTERNAL_FRAMEWORK_DIR}/${module_})
        target_link_libraries (${TARGET_NAME} ${module_})
    endforeach ()
endmacro (use_framework_modules)

# include local module headers 
macro (use_modules)
    message (STATUS "-- using modules: " ${ARGN})
    set (INTERNAL_MODULE_DIR "..")
    foreach (module_ ${ARGN})
        include_directories (${INTERNAL_MODULE_DIR}/${module_})
        link_directories (${INTERNAL_MODULE_DIR}/${module_})
    endforeach ()
endmacro (use_modules)

# include local module libraries
macro (link_modules)
    foreach (module_ ${ARGN})
        target_link_libraries (${TARGET_NAME} ${module_})
    endforeach ()
endmacro (link_modules)

# link directories
macro (link_package PREFIX)
    target_link_libraries (${TARGET_NAME} ${${PREFIX}_LIBRARIES})

    if (MSVC AND ${PREFIX}_DEBUG_LIBRARIES)
        target_link_libraries (${TARGET_NAME} debug ${${PREFIX}_DEBUG_LIBRARIES})
    endif ()
endmacro (link_package)

# manually find the debug libraries
macro (find_debug_libraries PREFIX DEBUG_POSTFIX)
    foreach (lib_ ${${PREFIX}_LIBRARIES})
        set (${PREFIX}_DEBUG_LIBRARIES ${${PREFIX}_DEBUG_LIBRARIES}
            ${lib_}${DEBUG_POSTFIX})
    endforeach ()
endmacro ()

# =============================================================================
# per-project configuration macros

macro (configure_boost)
    set (Boost_USE_STATIC_LIBS ON)
    sagase_configure_package (BOOST 
        NAMES Boost boost
        COMPONENTS date_time filesystem system thread program_options unit_test_framework
        PREFIXES ${ENV_NAALI_DEP_PATH})

    if (MSVC)
        set (BOOST_INCLUDE_DIRS ${BOOST_INCLUDE_DIRS} ${ENV_NAALI_DEP_PATH}/include)
        set (BOOST_LIBRARY_DIRS ${BOOST_LIBRARY_DIRS} ${ENV_NAALI_DEP_PATH}/lib)
    endif ()

endmacro (configure_boost)

macro (configure_poco)
    sagase_configure_package (POCO 
        NAMES Poco PoCo poco
        COMPONENTS Poco PocoFoundation PocoNet PocoUtil PocoXML
        PREFIXES ${ENV_NAALI_DEP_PATH})
endmacro (configure_poco)

macro (configure_qt4)
    sagase_configure_package (QT4 
        NAMES Qt4 qt4 Qt qt
        COMPONENTS QtCore QtGui QtWebkit QtScript QtXml QtNetwork QtUiTools
        PREFIXES ${ENV_NAALI_DEP_PATH} ${ENV_QT_DIR})

    # FindQt4.cmake
    if (QT4_FOUND AND QT_USE_FILE)

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

macro (configure_python)
    sagase_configure_package (PYTHON
        NAMES PythonLibs Python python Python26 python26 Python2.6 python2.6
        COMPONENTS Python python Python26 python Python2.6 python2.6
        PREFIXES ${ENV_NAALI_DEP_PATH})

    # FindPythonLibs.cmake
    if (PYTHONLIBS_FOUND)
        set (PYTHON_LIBRARIES ${PYTHON_LIBRARY})
        set (PYTHON_INCLUDE_DIRS ${PYTHON_INCLUDE_PATH})
        if (PYTHON_DEBUG_LIBRARY)
            set (PYTHON_DEBUG_LIBRARIES ${PYTHON_DEBUG_LIBRARY})
        endif ()
    endif ()
    
endmacro (configure_python)

macro (configure_python_qt)
    sagase_configure_package (PYTHON_QT
        NAMES PythonQt
        COMPONENTS PythonQt PythonQt_QtAll
        PREFIXES ${ENV_NAALI_DEP_PATH})
    
    # TODO: not currently working
    #find_debug_libraries (PYTHON "_d")

endmacro (configure_python_qt)

macro (configure_ois)
    sagase_configure_package (OIS 
        NAMES Ois ois OIS 
        COMPONENTS OIS
        PREFIXES ${ENV_NAALI_DEP_PATH})
    
    find_debug_libraries (OIS "_d")

endmacro (configure_ois)

macro (configure_ogre)
    sagase_configure_package (OGRE 
        NAMES Ogre OgreSDK ogre OGRE
        COMPONENTS Ogre ogre OGRE OgreMain 
        PREFIXES ${ENV_NAALI_DEP_PATH} ${ENV_OGRE_HOME})

    find_debug_libraries (OGRE "_d")

endmacro (configure_ogre)

macro (configure_caelum)
    sagase_configure_package (CAELUM 
        NAMES Caelum caelum CAELUM
        COMPONENTS Caelum caelum CAELUM
        PREFIXES ${ENV_NAALI_DEP_PATH})
    
    find_debug_libraries (CAELUM "_d")

endmacro (configure_caelum)

macro (configure_hydrax)
    sagase_configure_package (HYDRAX 
        NAMES Hydrax
        COMPONENTS Hydrax
        PREFIXES ${ENV_NAALI_DEP_PATH})
    
    find_debug_libraries (HYDRAX "_d")

endmacro (configure_hydrax)

macro (configure_xmlrpc)
    sagase_configure_package (XMLRPC 
        NAMES xmlrpc xmlrpcepi xmlrpc-epi
        COMPONENTS xmlrpc xmlrpcepi xmlrpc-epi
        PREFIXES ${ENV_NAALI_DEP_PATH}
        ${ENV_NAALI_DEP_PATH}/xmlrpc-epi/src
        ${ENV_NAALI_DEP_PATH}/xmlrpc-epi/Release
        ${ENV_NAALI_DEP_PATH}/xmlrpc-epi/Debug)
    
    find_debug_libraries (XMLRPC "d")

endmacro (configure_xmlrpc)

macro (configure_curl)
    sagase_configure_package (CURL 
        NAMES Curl curl libcurl
        COMPONENTS curl
        PREFIXES ${ENV_NAALI_DEP_PATH}
        ${ENV_NAALI_DEP_PATH}/libcurl/lib/DLL-Debug 
        ${ENV_NAALI_DEP_PATH}/libcurl/lib/DLL-Release)		
    
    find_debug_libraries (CURL "d")

endmacro (configure_curl)

macro (configure_openjpeg)
    sagase_configure_package (OPENJPEG 
        NAMES OpenJpeg OpenJPEG openjpeg
        COMPONENTS OpenJpeg OpenJPEG openjpeg
        PREFIXES ${ENV_NAALI_DEP_PATH}
        ${ENV_NAALI_DEP_PATH}/OpenJpeg/libopenjpeg
        ${ENV_NAALI_DEP_PATH}/OpenJpeg/Debug
        ${ENV_NAALI_DEP_PATH}/OpenJpeg/Release)
    
    find_debug_libraries (OPENJPEG "d")

endmacro (configure_openjpeg)

