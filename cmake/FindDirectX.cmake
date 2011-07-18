# Find DirectX SDK
#
# Defines:
# DirectX_FOUND
# DirectX_INCLUDE_DIR
# DirectX_LIBRARY_DIR
# DirectX_ROOT_DIR
# DirectX_LIBRARY - points to d3d9 library

set(DirectX_FOUND false)

if(WIN32) # The only platform it makes sense to check for DirectX SDK

    #### DirextX headers
    # We start with Microsoft DirectX SDK (August 2008) 9.24.1400
    # Example of path is C:\apps_x86\Microsoft DirectX SDK (August 2008)\Include
    
    find_path(DirectX_INCLUDE_DIR d3d9.h
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
  
  if (DirectX_INCLUDE_DIR AND DirectX_INCLUDE_DIR AND DirectX_LIBRARY)
    set(DirectX_FOUND true)
  endif ()

endif(WIN32)
