
# Configure KinectSDK, additional dependency for this project.
macro (configure_kinect)
    if (WIN32 AND VC100)
        message ("** Configuring KINECT")
        
        # If your kinect sdk is not found set KINECT_ROOT to your installation
        set (_KINECT_FIND_PATHS 
            "$ENV{KINECT_ROOT}/inc"
            "$ENV{KINECT_ROOT}/lib"
            "C:/Program Files (x86)/Microsoft*Kinect*/inc"
            "C:/Program Files (x86)/Microsoft*Kinect*/lib"
            "C:/Program Files/Microsoft*Kinect*/inc"
            "C:/Program Files/Microsoft*Kinect*/lib"
        )
        
        find_path (KINECT_INCLUDE_DIRS MSR_NuiApi.h PATHS ${_KINECT_FIND_PATHS})   
        find_path (KINECT_LIBRARY_DIRS MSRKinectNUI.lib PATHS ${_KINECT_FIND_PATHS})
        find_library (KINECT_LIBRARIES MSRKinectNUI PATHS ${_KINECT_FIND_PATHS})
        set (KINECT_RUNTIME_DIR ${KINECT_LIBRARY_DIRS}/..)

        if (${KINECT_INCLUDE_DIRS} STREQUAL "KINECT_INCLUDE_DIRS-NOTFOUND" OR
            ${KINECT_LIBRARY_DIRS} STREQUAL "KINECT_LIBRARY_DIRS-NOTFOUND" OR
            ${KINECT_LIBRARIES} STREQUAL "KINECT_LIBRARIES-NOTFOUND")
            message (WARNING "-- Could not automatically find Microsoft Kinect SDK, set KINECT_ROOT to your install directory.")
        endif ()
        
        message (STATUS "")
        sagase_configure_report (KINECT)
    endif ()    
endmacro ()

if (WIN32 AND VC100)
    configure_kinect ()

    # Define target name and output directory
    init_target (KinectModule OUTPUT plugins)

    # Define source files
    file (GLOB CPP_FILES *.cpp)
    file (GLOB H_FILES *.h )
    file (GLOB XML_FILES *.xml)
    #file (GLOB UI_FILES ui/*.ui)
    #file (GLOB RESOURCE_FILES resource/*.qrc)

    # Qt4 Moc files to subgroup "CMake Moc"
    # and ui_*.h generated .h files to "Generated UI" subgroup
    MocFolder ()
    UiFolder ()

    set (SOURCE_FILES ${SOURCE_FILES} ${CPP_FILES} ${H_FILES})

    # Qt4 Wrap 
    QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
    QT4_WRAP_UI(UI_SRCS ${UI_FILES})
    QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

    add_definitions (-DKINECT_MODULE_EXPORTS)

    UseTundraCore()
    use_core_modules(TundraCore Math OgreRenderingModule)
    use_package(KINECT)
    
    build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

    link_modules(TundraCore OgreRenderingModule)
    link_package(KINECT)
    
    SetupCompileFlagsWithPCH()

    final_target ()
    
    # Kinect module install requires some DLLs from the SDK
    if (WIN32)
        setup_install_files_find(${KINECT_RUNTIME_DIR} "MSRKINECTNUI.DLL" Release|RelWithDebInfo|Debug)
        setup_install_files_find(${KINECT_RUNTIME_DIR} "KINECTSQM.DLL" Release|RelWithDebInfo|Debug)
        setup_install_files_find(${KINECT_RUNTIME_DIR} "SQMAPI.DLL" Release|RelWithDebInfo|Debug)
    endif()
endif ()

if (UNIX OR VC90)
    message (WARNING "KinectModule can only be included in Windows with a VC100 compiler build at the moment.")
    message (WARNING "Try cmake_vs2010.bat with Visual Studio 2010 installed to enable kinect functionality.")
endif ()
