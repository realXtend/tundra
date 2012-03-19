# Find DirectX SDK
#
# Defines:
# DirectX_FOUND
# DirectX_INCLUDE_DIR
# DirectX_LIBRARY_DIR
# DirectX_ROOT_DIR
# DirectX_LIBRARY - points to d3d9 library

set (DirectX_FOUND false)

macro (configure_directx)
    if (WIN32) # The only platform it makes sense to check for DirectX SDK

        #### DirextX headers
        # We start with Microsoft DirectX SDK (August 2008) 9.24.1400
        # Example of path is C:\apps_x86\Microsoft DirectX SDK (August 2008)\Include
        
        find_path(DirectX_INCLUDE_DIR d3dx9.h
            "$ENV{DIRECTX_ROOT}/Include"
            "$ENV{DXSDK_DIR}/Include"
            "C:/apps_x86/Microsoft DirectX SDK*/Include"
            "C:/Program Files (x86)/Microsoft DirectX SDK*/Include"
            "C:/apps/Microsoft DirectX SDK*/Include"
            "C:/Program Files/Microsoft DirectX SDK*/Include")

        #### DirextX Libraries
        # dlls are in DirectX_ROOT_DIR/Developer Runtime/x64|x86
        # lib files are in DirectX_ROOT_DIR/Lib/x64|x86
      
        if (CMAKE_CL_64)
            set (DirectX_LIBRARY_PATHS
                "$ENV{DIRECTX_ROOT}/Lib/x64"
                "$ENV{DXSDK_DIR}/Lib/x64"
                "C:/Program Files (x86)/Microsoft DirectX SDK*/Lib/x64"
                "C:/Program Files/Microsoft DirectX SDK*/Lib/x64")
        else ()
            set (DirectX_LIBRARY_PATHS
                "$ENV{DIRECTX_ROOT}/Lib"
                "$ENV{DIRECTX_ROOT}/Lib/x86"
                "$ENV{DXSDK_DIR}/Lib"
                "$ENV{DXSDK_DIR}/Lib/x86"
                "C:/Program Files (x86)/Microsoft DirectX SDK*/Lib"
                "C:/Program Files (x86)/Microsoft DirectX SDK*/Lib/x86"
                "C:/Program Files/Microsoft DirectX SDK*/Lib"
                "C:/Program Files/Microsoft DirectX SDK*/Lib/x86")
        endif ()
        
        find_path (DirectX_LIBRARY_DIR d3d9.lib ${DirectX_LIBRARY_PATHS})
        find_library (DirectX_LIBRARY d3d9 ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
      
        # Check if DX was found
        if (DirectX_INCLUDE_DIR AND DirectX_INCLUDE_DIR AND DirectX_LIBRARY)
            set(DirectX_FOUND true)
        endif ()

        # DirectX SDK found, use DX9 surface blitting
        message ("** Configuring DirectX")
        if (DirectX_FOUND)
            message (STATUS "-- Include Directories:")
            message (STATUS "       " ${DirectX_INCLUDE_DIR})
            message (STATUS "-- Library Directories:")
            message (STATUS "       " ${DirectX_LIBRARY_DIR})
            message (STATUS "-- Defines:")
            message (STATUS "        DIRECTX_ENABLED")
        else ()
            message (STATUS "DirectX not found!")
            message (STATUS "-- Install DirectX SDK to enable additional features. If you already have the DirectX SDK installed")
            message (STATUS "   please set DIRECTX_ROOT env variable as your installation directory.")
        endif()
        message ("")
    endif(WIN32)
endmacro (configure_directx)

macro (link_directx)
    if (WIN32 AND DirectX_FOUND)
        add_definitions (-DDIRECTX_ENABLED)
        include_directories (${DirectX_INCLUDE_DIR})    
        link_directories (${DirectX_LIBRARY_DIR})
        # Not linking directly atm, we just need the headers, comment in if needed.
        #target_link_libraries (${TARGET_NAME} ${DirectX_LIBRARY})
    endif ()
endmacro (link_directx)
