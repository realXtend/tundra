
# Finds ogre for the Tundra build system.
# UNIX/MAC: Uses sagase/find_library in linux as it works there mighty well when its installed on the system.
# WINDOWS: Uses some more custom logic on windows to find things correct either from Tunda deps, Ogre SDK or Ogre source repo clone.

macro (configure_ogre)

    if (NOT WIN32)
    
        # Mac
        if (APPLE)
    	  FIND_LIBRARY (OGRE_LIBRARY NAMES Ogre)
    	  set (OGRE_INCLUDE_DIRS ${OGRE_LIBRARY}/Headers)
    	  set (OGRE_LIBRARIES ${OGRE_LIBRARY})
        # Linux
        else ()
            sagase_configure_package (OGRE 
                NAMES Ogre OgreSDK ogre OGRE
                COMPONENTS Ogre ogre OGRE OgreMain 
                PREFIXES ${ENV_OGRE_HOME} ${ENV_NAALI_DEP_PATH})
        endif ()
        
    else ()

        # Find and use DirectX if enabled in the build config
        if (ENABLE_DIRECTX)
            configure_directx ()
            link_directx ()
        else ()
            message (STATUS "DirectX disabled from the build")
        endif()

        # Needed components from Ogre
        set (OGRE_LIBS_TO_FIND OgreMain)
        if (ENABLE_DIRECTX AND DirectX_FOUND)
            set (OGRE_LIBS_TO_FIND ${OGRE_LIBS_TO_FIND} RenderSystem_Direct3D9)
        endif ()
        
        # Prepare search paths
        set (OGRE_SEARCH_SEARCH_PATHS
             ${ENV_OGRE_HOME}                # Repository clone or SDK installation
             ${ENV_OGRE_HOME}/SDK            # Repository default SDK install folder
             ${ENV_TUNDRA_DEP_PATH}/Ogre)    # Tundra deps searched last
            
        # Note these are in search priority order eg. /lib/release is looked before relwithdebinfo
        # so if you have built both the 'release' will be found first and used.
        set (OGRE_INCLUDE_SEARCH_PATHS "")
        set (OGRE_LIBRARY_RELEASE_SEARCH_PATHS "")
        set (OGRE_LIBRARY_DEBUG_SEARCH_PATHS "")
        
        foreach (OGRE_SEARCH_PATH ${OGRE_SEARCH_SEARCH_PATHS})
            # include
            set (OGRE_INCLUDE_SEARCH_PATHS ${OGRE_INCLUDE_SEARCH_PATHS} ${OGRE_SEARCH_PATH}/include)
            set (OGRE_INCLUDE_SEARCH_PATHS ${OGRE_INCLUDE_SEARCH_PATHS} ${OGRE_SEARCH_PATH}/include/OGRE)
            # debug lib
            set (OGRE_LIBRARY_DEBUG_SEARCH_PATHS ${OGRE_LIBRARY_DEBUG_SEARCH_PATHS} ${OGRE_SEARCH_PATH}/lib)
            set (OGRE_LIBRARY_DEBUG_SEARCH_PATHS ${OGRE_LIBRARY_DEBUG_SEARCH_PATHS} ${OGRE_SEARCH_PATH}/lib/opt)
            set (OGRE_LIBRARY_DEBUG_SEARCH_PATHS ${OGRE_LIBRARY_DEBUG_SEARCH_PATHS} ${OGRE_SEARCH_PATH}/lib/debug)                        
            set (OGRE_LIBRARY_DEBUG_SEARCH_PATHS ${OGRE_LIBRARY_DEBUG_SEARCH_PATHS} ${OGRE_SEARCH_PATH}/lib/debug/opt)
            # release lib
            set (OGRE_LIBRARY_RELEASE_SEARCH_PATHS ${OGRE_LIBRARY_RELEASE_SEARCH_PATHS} ${OGRE_SEARCH_PATH}/lib)
            set (OGRE_LIBRARY_RELEASE_SEARCH_PATHS ${OGRE_LIBRARY_RELEASE_SEARCH_PATHS} ${OGRE_SEARCH_PATH}/lib/opt)
            set (OGRE_LIBRARY_RELEASE_SEARCH_PATHS ${OGRE_LIBRARY_RELEASE_SEARCH_PATHS} ${OGRE_SEARCH_PATH}/lib/release)
            set (OGRE_LIBRARY_RELEASE_SEARCH_PATHS ${OGRE_LIBRARY_RELEASE_SEARCH_PATHS} ${OGRE_SEARCH_PATH}/lib/release/opt)
            set (OGRE_LIBRARY_RELEASE_SEARCH_PATHS ${OGRE_LIBRARY_RELEASE_SEARCH_PATHS} ${OGRE_SEARCH_PATH}/lib/relwithdebinfo)
            set (OGRE_LIBRARY_RELEASE_SEARCH_PATHS ${OGRE_LIBRARY_RELEASE_SEARCH_PATHS} ${OGRE_SEARCH_PATH}/lib/relwithdebinfo/opt)
            set (OGRE_LIBRARY_RELEASE_SEARCH_PATHS ${OGRE_LIBRARY_RELEASE_SEARCH_PATHS} ${OGRE_SEARCH_PATH}/lib/minsizerel)
            set (OGRE_LIBRARY_RELEASE_SEARCH_PATHS ${OGRE_LIBRARY_RELEASE_SEARCH_PATHS} ${OGRE_SEARCH_PATH}/lib/minsizerel/opt)
        endforeach ()

        # Find include folder
        find_path (OGRE_INCLUDE_DIRS "Ogre.h" PATHS ${OGRE_INCLUDE_SEARCH_PATHS} NO_DEFAULT_PATH)
       
        if (NOT OGRE_INCLUDE_DIRS)
            message(FATAL_ERROR "Could not find required Ogre include: Ogre.h")
        endif ()
        
        # Find required components
        foreach (REQUIRED_OGRE_LIB ${OGRE_LIBS_TO_FIND})
            set (REQUIRED_OGRE_LIB_RELEASE ${REQUIRED_OGRE_LIB}.lib)

            find_path (OGRE_RELEASE_LIBRARY_DIR_${REQUIRED_OGRE_LIB} ${REQUIRED_OGRE_LIB_RELEASE}
                       PATHS ${OGRE_LIBRARY_RELEASE_SEARCH_PATHS} NO_DEFAULT_PATH)
            
            set (REQUIRED_OGRE_LIB_DEBUG ${REQUIRED_OGRE_LIB}_d.lib)      
            
            find_path (OGRE_DEBUG_LIBRARY_DIR_${REQUIRED_OGRE_LIB} ${REQUIRED_OGRE_LIB_DEBUG}
                       PATHS ${OGRE_LIBRARY_DEBUG_SEARCH_PATHS} NO_DEFAULT_PATH)
                       
            if (NOT OGRE_RELEASE_LIBRARY_DIR_${REQUIRED_OGRE_LIB} OR NOT OGRE_DEBUG_LIBRARY_DIR_${REQUIRED_OGRE_LIB})
                message(FATAL_ERROR "Could not find required Ogre component: " ${REQUIRED_OGRE_LIB})
            endif ()
            
            set (OGRE_LIBRARY_DIRS ${OGRE_LIBRARY_DIRS} ${OGRE_RELEASE_LIBRARY_DIR_${REQUIRED_OGRE_LIB}} ${OGRE_DEBUG_LIBRARY_DIR_${REQUIRED_OGRE_LIB}})
            set (OGRE_LIBRARIES ${OGRE_LIBRARIES} ${REQUIRED_OGRE_LIB})
            set (OGRE_DEBUG_LIBRARIES ${OGRE_DEBUG_LIBRARIES} ${REQUIRED_OGRE_LIB}_d)
        endforeach ()
                   
        # Remove duplicate entires from return variables
        if (OGRE_INCLUDE_DIRS)
            list (REMOVE_DUPLICATES OGRE_INCLUDE_DIRS)
        endif ()
        if (OGRE_LIBRARY_DIRS)
            list (REMOVE_DUPLICATES OGRE_LIBRARY_DIRS)
        endif ()
        if (OGRE_LIBRARIES)
            list (REMOVE_DUPLICATES OGRE_LIBRARIES)
        endif ()
        if (OGRE_DEBUG_LIBRARIES)
            list (REMOVE_DUPLICATES OGRE_DEBUG_LIBRARIES)
        endif ()
        
    endif ()
    
    sagase_configure_report (OGRE)

endmacro (configure_ogre)

macro (link_ogre)
    use_package (OGRE)
    link_package (OGRE)
endmacro ()