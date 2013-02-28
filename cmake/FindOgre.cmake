
# Finds ogre for the Tundra build system.
# UNIX/MAC: Uses sagase/find_library in linux as it works there mighty well when its installed on the system.
# WINDOWS: Uses some more custom logic on windows to find things correct either from Tunda deps, Ogre SDK or Ogre source repo clone.

if (NOT WIN32 AND NOT APPLE)
# TODO: Remove configure_ogre and replace it with a use_package_ogre() and link_package_ogre()
macro(configure_ogre)

  # Android uses static Ogre, include all used plugins and their include directories
  if (NOT ANDROID)
      find_path(OGRE_INCLUDE_DIR Ogre.h
        HINTS ${ENV_OGRE_HOME}/include ${ENV_NAALI_DEP_PATH}/include
        PATH_SUFFIXES OGRE)

      find_path(OGRE_LIBRARY_DIR NAMES libOgreMain.so
        HINTS ${ENV_OGRE_HOME} ${ENV_NAALI_DEP_PATH})
      find_library(OGRE_LIBRARY OgreMain
        HINTS ${ENV_OGRE_HOME}/lib ${ENV_NAALI_DEP_PATH}/lib)

      include_directories(${OGRE_INCLUDE_DIR})
      link_directories(${OGRE_LIBRARY_DIR})
  else()
      set (OGRE_LIBRARY_DIR ${ENV_OGRE_HOME}/lib ${ENV_OGRE_HOME}/AndroidDependencies/lib)
      set (OGRE_LIBRARY OgreOverlayStatic Plugin_OctreeSceneManagerStatic OgreRTShaderSystemStatic Plugin_ParticleFXStatic RenderSystem_GLES2Static OgreMainStatic freeimage freetype stdc++ supc++ z zziplib cpu-features EGL GLESv1_CM GLESv2 android)
      include_directories(${ENV_OGRE_HOME}/include ${ENV_OGRE_HOME}/OgreMain/include ${ENV_OGRE_HOME}/Components/Overlay/include ${ENV_OGRE_HOME}/Components/RTShaderSystem/include  
        ${ENV_OGRE_HOME}/RenderSystems/GLES2/include ${ENV_OGRE_HOME}/PlugIns/OctreeSceneManager/include ${ENV_OGRE_HOME}/PlugIns/ParticleFX/include)
      link_directories(${OGRE_LIBRARY_DIR})
  endif()
endmacro()

else() # Windows and OSX Ogre lookup.

# TODO: Remove configure_ogre and replace it with a use_package_ogre() and link_package_ogre()
macro(configure_ogre)
    # Find and use DirectX if enabled in the build config
    if (ENABLE_DIRECTX)
        configure_directx()
        link_directx()
    else()
        message(STATUS "DirectX disabled from the build\n")
    endif()

    # If doing TUNDRA_NO_BOOST build, TBB is used for Ogre's threading.
    if (TUNDRA_NO_BOOST)
        if ("${TBB_HOME}" STREQUAL "")
            file(TO_CMAKE_PATH "$ENV{TBB_HOME}" TBB_HOME)
            set(TBB_HOME ${TBB_HOME} CACHE PATH "TBB_HOME dependency path" FORCE)
        endif()
    endif()

    # Ogre lookup rules:
    # 1. Use the predefined OGRE_DIR CMake variable if it was set.
    # 2. Otherwise, use the OGRE_HOME environment variable if it was set  and cache it as OGRE_DIR.
    # 3. Otherwise, use Ogre from Tundra deps directory.

    if ("${OGRE_DIR}" STREQUAL "")
        file (TO_CMAKE_PATH "$ENV{OGRE_HOME}" OGRE_DIR)
        # Cache OGRE_DIR for runs that dont define $ENV{OGRE_HOME}.
        set (OGRE_DIR ${OGRE_DIR} CACHE PATH "OGRE_HOME dependency path" FORCE)
    endif()

    # On Apple, Ogre comes in the form of a Framework. The user has to have this manually installed.
    if (APPLE)
        if ("${OGRE_DIR}" STREQUAL "" OR NOT IS_DIRECTORY ${OGRE_DIR}/lib)
            find_library(OGRE_LIBRARY Ogre)
            set(OGRE_DIR ${OGRE_LIBRARY})
        else()
            #set(OGRE_DIR ${OGRE_DIR}/lib/Ogre.framework) # User specified custom Ogre directory pointing to Ogre Hg trunk directory.
            set(OGRE_BUILD_CONFIG "relwithdebinfo") # TODO: We would like to link to debug in debug mode, release in release etc, not always fixed to this.
            set(OGRE_LIBRARY ${OGRE_DIR}/lib/relwithdebinfo/Ogre.framework)
        endif()
    endif()
         
    # Finally, if no Ogre found, assume the deps path.
    if ("${OGRE_DIR}" STREQUAL "")
        if (IS_DIRECTORY ${ENV_TUNDRA_DEP_PATH}/ogre-safe-nocrashes)
            set(OGRE_DIR ${ENV_TUNDRA_DEP_PATH}/ogre-safe-nocrashes)
        else()
            set(OGRE_DIR ${ENV_TUNDRA_DEP_PATH}/Ogre)
        endif()
    endif()

    # The desired Ogre path is set in OGRE_DIR. The Ogre source tree comes in two flavors:
    # 1. If you cloned and built the Ogre Hg repository, OGRE_DIR can point to Hg root directory.
    # 2. If you are using an installed Ogre SDK, OGRE_DIR can point to the SDK root directory.
    # We want to support both so that one can do active development on the Ogre Hg repository, without
    # having to always do the intermediate SDK installation/deployment step.
    
    message("** Configuring Ogre")
    if (APPLE)# AND IS_DIRECTORY ${OGRE_DIR}/Headers) # OGRE_DIR points to a manually installed Ogre.framework?
        if (IS_DIRECTORY ${OGRE_DIR}/lib)
            include_directories(${OGRE_DIR}/lib/relwithdebinfo/Ogre.framework/Headers)
            link_directories(${OGRE_DIR}/lib/relwithdebinfo/Ogre.framework)
        else()
            include_directories(${OGRE_DIR}/Headers)
            link_directories(${OGRE_DIR})
        endif()
        message(STATUS "Using Ogre from directory " ${OGRE_DIR})
    elseif (IS_DIRECTORY ${OGRE_DIR}/OgreMain) # Ogre path points to #1 above.
        include_directories(${OGRE_DIR}/include)
        include_directories(${OGRE_DIR}/OgreMain/include)
        if (WIN32)
            include_directories(${OGRE_DIR}/RenderSystems/Direct3D9/include)
        endif()
        link_directories(${OGRE_DIR}/lib)
        message(STATUS "Using Ogre from Mercurial trunk directory " ${OGRE_DIR})
    elseif (IS_DIRECTORY ${OGRE_DIR}/include/OGRE) # If include/OGRE exists, then OGRE_DIR points to the SDK (#2 above)
        include_directories(${OGRE_DIR}/include)
        include_directories(${OGRE_DIR}/include/OGRE)
        link_directories(${OGRE_DIR}/lib)
        if (WIN32)
            include_directories(${OGRE_DIR}/include/OGRE/RenderSystems/Direct3D9)
            # Note: VC9 uses $(ConfigurationName), but #VC10 and onwards uses $(Configuration).
            # However VC10 seems to be able to understand $(ConfigurationName) also, so use that.
            link_directories(${OGRE_DIR}/lib/$(ConfigurationName))
            link_directories(${OGRE_DIR}/lib/$(ConfigurationName)/opt)
            if (TUNDRA_NO_BOOST) # TBB used for Ogre threading, so include it
                include_directories(${TBB_HOME}/include)
                if (MSVC90)
                    set(VC_VER "vc9")
                elseif(MSVC10)
                    set(VC_VER "vc10")
                #elseif(MSVC11)
                #    set(VC_VER "vc11")
                endif()
                link_directories(${TBB_HOME}/lib/ia32/${VC_VER}) # TODO ia32 is correct folder only on 32-bit
            endif()
        endif()
        message(STATUS "Using Ogre from SDK directory " ${OGRE_DIR})
    else()
        message(FATAL_ERROR "When looking for Ogre, the path ${OGRE_DIR} does not point to a valid Ogre directory!")
    endif()
    message("")
endmacro()

endif()

macro(link_ogre)
    if (WIN32)
        target_link_libraries(${TARGET_NAME} debug OgreMain_d optimized OgreMain)
        if (ENABLE_DIRECTX)
            target_link_libraries(${TARGET_NAME} debug RenderSystem_Direct3D9_d optimized RenderSystem_Direct3D9)
        endif()
    else()
        target_link_libraries(${TARGET_NAME} ${OGRE_LIBRARY})
    endif()
endmacro()
