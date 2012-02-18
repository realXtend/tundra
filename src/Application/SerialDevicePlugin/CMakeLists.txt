if (MSVC)

    # Define target name and output directory
    init_target (SerialDevicePlugin OUTPUT plugins)

    # Define source files
    file (GLOB CPP_FILES *.cpp)
    file (GLOB H_FILES *.h)

    # Do manual source grouping as we need to detect a .cc file
    #file(GLOB H_FILES_qextserialport qextserialport/*.h)
    #file(GLOB CPP_FILES_qextserialport qextserialport/*.cpp)
    #source_group("Header Files\\qextserialport" FILES ${H_FILES_qextserialport})
    #source_group("Source Files\\qextserialport" FILES ${CPP_FILES_qextserialport})
    #set(H_FILES ${H_FILES} ${H_FILES_qextserialport})
    #set(CPP_FILES ${CPP_FILES} ${CPP_FILES_qextserialport})

    MocFolder ()

    QT4_WRAP_CPP (MOC_SRCS ${H_FILES})

    set (SOURCE_FILES ${SOURCE_FILES} ${CPP_FILES} ${H_FILES} ${MOC_SRCS})

    use_core_modules (Framework Scene TundraProtocolModule)
    use_app_modules (JavascriptModule)

    build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${PROTOBUF_CPP_FILES} ${PROTOBUF_H_FILES})

    link_modules (Framework Scene JavascriptModule JavascriptModule)
    
    # See BUILD_AND_USAGE_INSTRUCTIONS.txt how to get the needed dependencies used below
    
    include_directories (${CMAKE_CURRENT_SOURCE_DIR}/qextserialport)
    target_link_libraries (${TARGET_NAME} optimized ${CMAKE_CURRENT_SOURCE_DIR}/qextserialport/qextserialport-1.2.lib)
    target_link_libraries (${TARGET_NAME} debug ${CMAKE_CURRENT_SOURCE_DIR}/qextserialport/qextserialport-1.2d.lib)
    target_link_libraries (${TARGET_NAME} setupapi.lib)
    
    SetupCompileFlags()

    final_target ()
    
endif ()
