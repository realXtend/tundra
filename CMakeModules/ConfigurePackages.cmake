# =============================================================================
# per-dependency configuration macros
#
# All per-dependency configuration (or hacks) should go here. All per-module
# build instructions should go in <Module>/CMakeLists.txt. The rest should 
# remain generic.

macro (configure_boost)
    set (Boost_USE_STATIC_LIBS ON)
    sagase_configure_package (BOOST 
        NAMES Boost boost
        COMPONENTS date_time filesystem system thread program_options unit_test_framework
        PREFIXES ${ENV_NAALI_DEP_PATH})

    # boost library naming is complex, and FindBoost.cmake is preferred to 
    # find the correct names. however on windows it appears to not find the
    # library directories correctly. find_path cannot be counted on to find
    # the libraries as component thread -> libboost_thread_vc90-mt.lib (etc.)

    if (MSVC)
        set (BOOST_INCLUDE_DIRS ${BOOST_INCLUDE_DIRS} ${ENV_NAALI_DEP_PATH}/Boost/include)
        set (BOOST_LIBRARY_DIRS ${BOOST_LIBRARY_DIRS} ${ENV_NAALI_DEP_PATH}/Boost/lib)
    endif ()

    sagase_configure_report (BOOST)
endmacro (configure_boost)

macro (configure_poco)
    sagase_configure_package (POCO 
        NAMES Poco PoCo poco
        COMPONENTS Poco PocoFoundation PocoNet PocoUtil PocoXML
        PREFIXES ${ENV_NAALI_DEP_PATH})

    # POCO autolinks on MSVC
    if (MSVC)
        set (POCO_LIBRARIES "")
        set (POCO_DEBUG_LIBRARIES "")
    endif ()

    sagase_configure_report (POCO)
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
            ${QT_QTWEBKIT_INCLUDE_DIR})
            #${QT_PHONON_INCLUDE_DIR})
		
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
    
    sagase_configure_report (QT4)
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
        #unset (PYTHON_DEBUG_LIBRARIES ${PYTHON_DEBUG_LIBRARY})
    endif ()
	
    # FindPythonLibs.cmake prefers the system-wide Python, which does not
    # include debug libraries, so we force to NAALI_DEP_PATH.

	if (MSVC)
		set (PYTHON_LIBRARY_DIRS ${ENV_NAALI_DEP_PATH}/Python/lib)
		set (PYTHON_INCLUDE_DIRS ${ENV_NAALI_DEP_PATH}/Python/include)
		set (PYTHON_LIBRARIES python26)
		set (PYTHON_DEBUG_LIBRARIES python26_d)
	endif()
    
    sagase_configure_report (PYTHON)
endmacro (configure_python)

macro (configure_python_qt)
    sagase_configure_package (PYTHON_QT
        NAMES PythonQt
        COMPONENTS PythonQt PythonQt_QtAll
        PREFIXES ${ENV_NAALI_DEP_PATH})

    sagase_configure_report (PYTHON_QT)
endmacro (configure_python_qt)

macro (configure_ois)
    sagase_configure_package (OIS 
        NAMES Ois ois OIS 
        COMPONENTS OIS
        PREFIXES ${ENV_NAALI_DEP_PATH})
    
    sagase_configure_report (OIS)
endmacro (configure_ois)

macro (configure_ogre)
    sagase_configure_package (OGRE 
        NAMES Ogre OgreSDK ogre OGRE
        COMPONENTS Ogre ogre OGRE OgreMain 
        PREFIXES ${ENV_NAALI_DEP_PATH} ${ENV_OGRE_HOME})

    sagase_configure_report (OGRE)
endmacro (configure_ogre)

macro (configure_caelum)
    sagase_configure_package (CAELUM 
        NAMES Caelum caelum CAELUM
        COMPONENTS Caelum caelum CAELUM
        PREFIXES ${ENV_NAALI_DEP_PATH})
    
    sagase_configure_report (CAELUM)
endmacro (configure_caelum)

macro (configure_hydrax)
    sagase_configure_package (HYDRAX 
        NAMES Hydrax
        COMPONENTS Hydrax
        PREFIXES ${ENV_NAALI_DEP_PATH})
    
    sagase_configure_report (HYDRAX)
endmacro (configure_hydrax)

macro (configure_xmlrpc)
    sagase_configure_package (XMLRPC 
        NAMES xmlrpc xmlrpcepi xmlrpc-epi
        COMPONENTS xmlrpc xmlrpcepi xmlrpc-epi xmlrpcepid 
        PREFIXES ${ENV_NAALI_DEP_PATH}
        ${ENV_NAALI_DEP_PATH}/xmlrpc-epi/src
        ${ENV_NAALI_DEP_PATH}/xmlrpc-epi/Debug
		${ENV_NAALI_DEP_PATH}/xmlrpc-epi/Release)
   
	if (MSVC)
		set (XMLRPC_LIBRARIES xmlrpcepi)
		set (XMLRPC_DEBUG_LIBRARIES xmlrpcepid)
	endif()
	
    sagase_configure_report (XMLRPC)
endmacro (configure_xmlrpc)

macro (configure_curl)
    sagase_configure_package (CURL 
        NAMES Curl curl libcurl
        COMPONENTS curl libcurl_imp libcurld_imp
        PREFIXES ${ENV_NAALI_DEP_PATH}
        ${ENV_NAALI_DEP_PATH}/libcurl/lib/DLL-Debug 
        ${ENV_NAALI_DEP_PATH}/libcurl/lib/DLL-Release)		
    
	if (MSVC)
		set (CURL_LIBRARIES libcurl_imp)
		set (CURL_DEBUG_LIBRARIES libcurld_imp)
	endif()
	
    sagase_configure_report (CURL)
endmacro (configure_curl)

macro (configure_openjpeg)
    sagase_configure_package (OPENJPEG 
        NAMES OpenJpeg OpenJPEG openjpeg
        COMPONENTS OpenJpeg OpenJPEG openjpeg
        PREFIXES ${ENV_NAALI_DEP_PATH}
        ${ENV_NAALI_DEP_PATH}/OpenJpeg/libopenjpeg
        ${ENV_NAALI_DEP_PATH}/OpenJpeg/Debug
        ${ENV_NAALI_DEP_PATH}/OpenJpeg/Release)
    
    sagase_configure_report (OPENJPEG)
endmacro (configure_openjpeg)

macro (configure_telepathy_qt4)
    sagase_configure_package (TELEPATHY_QT4 
        NAMES QtTelepathy Telepathy-QT4 telepathy-qt4
        COMPONENTS QtTelepathyCore QtTelepathyCommon QtTelepathyClient telepathy-qt4
        PREFIXES ${ENV_NAALI_DEP_PATH})

    # TODO: find out why sagase magic cannot find include dir on windows
    if (MSVC)
        set (TELEPATHY_QT4_INCLUDE_DIRS ${TELEPATHY_QT4_INCLUDE_DIRS} ${ENV_NAALI_DEP_PATH}/telepathy-qt4/include)
    endif ()
        
    sagase_configure_report (TELEPATHY_QT4)
endmacro (configure_telepathy_qt4)

