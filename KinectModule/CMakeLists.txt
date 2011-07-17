if (WIN32 AND VC100)
    configure_kinect ()

    # Define target name and output directory
    init_target (KinectModule OUTPUT modules/core)

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

    use_modules (Core Foundation Interfaces OgreRenderingModule)
    use_package (KINECT)
    
    build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

    link_modules (Core Foundation Interfaces OgreRenderingModule)
    link_package (KINECT)
    
    SetupCompileFlagsWithPCH()
    CopyModuleXMLFile()

    final_target ()
endif ()

if (WIN32 AND VC90)
    message (WARNING "KinectModule can only be included with a VC100 compiler build at the moment.")
    message (WARNING "Try cmake_vs2010.bat with Visual Studio 2010 installed to enable kinect functionality.")
endif ()
