
# Finds ogre for the Tundra build system.
# UNIX/MAC: Uses sagase/find_library in linux as it works there mighty well when its installed on the system.
# WINDOWS: Uses some more custom logic on windows to find things correct either from Tunda deps, Ogre SDK or Ogre source repo clone.

if (NOT WIN32)
# TODO Remove the use of Sagase for Linux and Mac Ogre lookup.
# TODO: Remove configure_ogre and replace it with a use_package_ogre() and link_package_ogre()
macro(configure_ogre)
    # Mac
    if (APPLE)
        FIND_LIBRARY(OGRE_LIBRARY NAMES Ogre)
        set(OGRE_INCLUDE_DIRS ${OGRE_LIBRARY}/Headers)
        set(OGRE_LIBRARIES ${OGRE_LIBRARY})
    # Linux
    else()
        sagase_configure_package (OGRE 
            NAMES Ogre OgreSDK ogre OGRE
            COMPONENTS Ogre ogre OGRE OgreMain 
            PREFIXES ${ENV_OGRE_HOME} ${ENV_NAALI_DEP_PATH})
    endif ()

    sagase_configure_report(OGRE)
endmacro()
    
else() # Windows Ogre lookup.

# TODO: Remove configure_ogre and replace it with a use_package_ogre() and link_package_ogre()
macro(configure_ogre)
    # Find and use DirectX if enabled in the build config
    if (ENABLE_DIRECTX)
        configure_directx()
        link_directx()
    else()
        message(STATUS "DirectX disabled from the build")
    endif()

    # Ogre lookup rules:
    # 1. Use the predefined OGRE_DIR CMake variable if it was set.
    # 2. Otherwise, use the OGRE_HOME environment variable if it was set.
    # 3. Otherwise, use Ogre from Tundra deps directory.

    if ("${OGRE_DIR}" STREQUAL "")
        set(OGRE_DIR $ENV{OGRE_HOME})
    endif()

    if ("${OGRE_DIR}" STREQUAL "")
        set(OGRE_DIR ${ENV_TUNDRA_DEP_PATH}/Ogre)
    endif()

    # The desired Ogre path is set in OGRE_DIR. The Ogre source tree comes in two flavors:
    # 1. If you cloned and built the Ogre Hg repository, OGRE_DIR can point to Hg root directory.
    # 2. If you are using an installed Ogre SDK, OGRE_DIR can point to the SDK root directory.
    # We want to support both so that one can do active development on the Ogre Hg repository, without
    # having to always do the intermediate SDK installation/deployment step.
    
    if (IS_DIRECTORY ${OGRE_DIR}/OgreMain) # Ogre path points to #1 above.    
        include_directories(${OGRE_DIR}/include)
        include_directories(${OGRE_DIR}/OgreMain/include)
        include_directories(${OGRE_DIR}/RenderSystems/Direct3D9/include)
        link_directories(${OGRE_DIR}/lib)
        message(STATUS "Using Ogre from Mercurial trunk directory " ${OGRE_DIR})
    elseif (IS_DIRECTORY ${OGRE_DIR}/include/OGRE) # If include/OGRE exists, then OGRE_DIR points to the SDK (#2 above)    
        include_directories(${OGRE_DIR}/include/OGRE)
        include_directories(${OGRE_DIR}/include/OGRE/RenderSystems/Direct3D9)        
        link_directories(${OGRE_DIR}/lib)
        link_directories(${OGRE_DIR}/lib/$(OutDir)/opt)
        message(STATUS "Using Ogre from SDK directory " ${OGRE_DIR})
    else()
        message(FATAL_ERROR "When looking for Ogre, the path ${OGRE_DIR} does not point to a valid Ogre directory!")
    endif()
endmacro()

endif()

macro(link_ogre)
    if (NOT WIN32)
        use_package(OGRE)
        link_package(OGRE)
    else()
        target_link_libraries(${TARGET_NAME} debug OgreMain_d debug RenderSystem_Direct3D9_d)
        target_link_libraries(${TARGET_NAME} optimized OgreMain optimized RenderSystem_Direct3D9)        
    endif()
endmacro()