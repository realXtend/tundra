/** main.cpp
    @brief Provides a command-line utility for mass-processing texture files into .dds file format.

    To build, check the property sheets of the TextureTool project in the Property Manager tab.
    There are three property sheets:
    - NVTT: Not yet used. This is for programmatically accessing http://developer.nvidia.com/object/texture_tools.html ,
       instead of having to do it through a command-line script. NVTT is used to convert data into DXT file formats. 
       See ConvertAll.cmd in the project directory on how the tool is used.
    - D3D9: This should be configured to point to your DirectX SDK includes and libs paths, unless you have those in your
       global VS settings.
    - J2K: This should be configured to point to the includes and libs of your OpenJPEG codec installation. Download
       is available from http://www.openjpeg.org/ . Note that there are separate sheets for debug and release modes.
*/

#include <iostream>
#include <cstdio>
#include <vector>
#include <sstream>
#include <cassert>
#include <string>
#include <exception>

/// Comment this out if you do not want to have J2K support, and want to avoid building a dependency to it.
//#define J2K_DECODE_SUPPORT

#ifdef J2K_DECODE_SUPPORT
#include <openjpeg.h>
#pragma comment(lib, "OpenJPEG.lib")
#endif

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>

using namespace std;

/// Prints a readable version of the given HRESULT.
/// @param expression A custom string that is embedded in the error string. (pass in the expression that failed, for example)
string FormatErrorString(const char *expression, HRESULT hr)
{
    stringstream ss;
    ss << "Expression \"" << (expression ? expression : "(null)") << "\" failed: returned HRESULT 0x" << ios::hex << hr << ". Reason: \"" 
        << DXGetErrorString(hr) << "\". Description: \"" << DXGetErrorDescription(hr) << "\".";
    return ss.str();
}

/// Checks that the given hr SUCCEEDED(hr), or otherwise throws an std::exception.
void Win32Try(HRESULT hr, const char *expression, const char *errorString)
{
    if (FAILED(hr))
    {
        cerr << FormatErrorString(expression, hr) << endl;
        cerr << errorString << endl;
        throw exception(expression);
    }
}

// Treat std::string == const char* equivalent.
void Win32Try(HRESULT hr, const char *expression, const string &errorString) { Win32Try(hr, expression, errorString.c_str()); }

/// Macro for auto-checking successes in D3D API calls, and logging and throwing an exception if they fail.
#define WIN32TRY(expression, errorString) { HRESULT hr = expression; Win32Try(hr, #expression, errorString); }

/// Stores a raw texture surface in memory.
struct RawImage
{
    RawImage():width(0), height(0)
    {}

    std::vector<unsigned char> data;

    int width;
    int height;

    D3DFORMAT format;

    /// The number of components per pixel. (the number of color channels)
    /// Depending on this number, the internal format is one of ARGB8888, RGB888, L8A8 (grayscale with alpha), L8 (grayscale).
    int numColorPlanes;

    /// @return True if the image contains an alpha channel that is filled with a single color (0x00, 0xFF or something similar).
    /// False if the image does not have an alpha channel or if there exists actual data in that channel.
    bool HasRedundantAlphaChannel() const
    {
        // No alpha channel at all?
        if (numColorPlanes == 1 || numColorPlanes == 3)
            return false;

        // Compare alpha over the image against this point.
        int alpha = GetAlpha(0, 0);

        for(int y = 0; y < height; ++y)
            for(int x = 0; x < width; ++x)
                if (GetAlpha(x, y) != alpha)
                    return false;

        return true;
    }

    /// Gets the value of the color component of the pixel at the given coordinates.
    // plane 0 == B
    // plane 1 == G
    // plane 2 == R
    // plane 3 == A
    int GetPlane(int x, int y, int plane) const
    {
        return data[(y*width+x)*numColorPlanes + plane];
    }

    /// Sets the value of the color component of the pixel at the given coordinates.
    void SetPlane(int x, int y, int plane, int color)
    {
        data[(y*width+x)*numColorPlanes + plane] = color;
    }

    bool HasAlphaChannel() const { return numColorPlanes == 2 || numColorPlanes == 4; }

    int GetAlpha(int x, int y) const
    {
        if (numColorPlanes == 1 || numColorPlanes == 3)
            return 0xFF;

        return data[(y * width + x) * numColorPlanes + numColorPlanes-1];
    }
};

/// Converts the image color format in-place.
void ARGB8888ToRGB888(RawImage &img)
{
    assert(img.numColorPlanes == 4);
//    assert(img.format == D3DFMT_A8R8G8B8);
    RawImage copy = img;
    --img.numColorPlanes;

    img.data.resize(img.width * img.height * img.numColorPlanes);
    for(int y = 0; y < img.height; ++y)
        for(int x = 0; x < img.width; ++x)
        {
            img.SetPlane(x, y, 0, copy.GetPlane(x, y, 0)); // B
            img.SetPlane(x, y, 1, copy.GetPlane(x, y, 1)); // G
            img.SetPlane(x, y, 2, copy.GetPlane(x, y, 2)); // R
        }

    img.format = D3DFMT_R8G8B8;
}

/// Converts the image color format in-place.
void ABGR8888ToARGB8888(RawImage &img)
{
    assert(img.numColorPlanes == 4);
//    assert(img.format == D3DFMT_A8R8G8B8);
    RawImage copy = img;

    img.data.resize(img.width * img.height * img.numColorPlanes);
    for(int y = 0; y < img.height; ++y)
        for(int x = 0; x < img.width; ++x)
        {
            img.SetPlane(x, y, 0, copy.GetPlane(x, y, 2)); // B
            img.SetPlane(x, y, 1, copy.GetPlane(x, y, 1)); // G
            img.SetPlane(x, y, 2, copy.GetPlane(x, y, 0)); // R

            img.SetPlane(x, y, 3, copy.GetPlane(x, y, 3)); // A
        }

    img.format = D3DFMT_A8R8G8B8;
}

/// Converts the image color format in-place.
void RGBA8888ToARGB8888(RawImage &img)
{
    assert(img.numColorPlanes == 4);
//    assert(img.format == D3DFMT_A8R8G8B8);
    RawImage copy = img;

    img.data.resize(img.width * img.height * img.numColorPlanes);
    for(int y = 0; y < img.height; ++y)
        for(int x = 0; x < img.width; ++x)
        {
            img.SetPlane(x, y, 0, copy.GetPlane(x, y, 1)); // B
            img.SetPlane(x, y, 1, copy.GetPlane(x, y, 2)); // G
            img.SetPlane(x, y, 2, copy.GetPlane(x, y, 3)); // R

            img.SetPlane(x, y, 3, copy.GetPlane(x, y, 0)); // A
        }

    img.format = D3DFMT_A8R8G8B8;
}

/// Swaps the endianness of the color components.
/// Converts the image color format in-place.
void BGR888ToRGB888(RawImage &img)
{
//    assert(img.format == D3DFMT_B8G8R8);
    RawImage copy = img;
    for(int y = 0; y < img.height; ++y)
        for(int x = 0; x < img.width; ++x)
            for(int i = 0; i < img.numColorPlanes; ++i)
                img.SetPlane(x, y, i, copy.GetPlane(x, y, img.numColorPlanes-1 - i));

    img.format = D3DFMT_R8G8B8;
}

/// Halves the texture size by averaging every adjacent 2x2 block into one pixel. Operates in-place. 
/// Requires that the image dimensions are divisible by two.
void HalveTextureSize(RawImage &img)
{
    RawImage copy = img;
// The following assumption is not strictly needed, it just means we will discard one column or row of pixels at the edge of the image.
//    assert(img.width % 2 == 0);
//    assert(img.height % 2 == 0);
    img.width /= 2;
    img.height /= 2;

    img.data.resize(img.width * img.height * img.numColorPlanes);
    for(int y = 0; y < img.height; ++y)
        for(int x = 0; x < img.width; ++x)
            for(int c = 0; c < img.numColorPlanes; ++c)
                img.SetPlane(x, y, c, (copy.GetPlane(x*2, y*2, c) + copy.GetPlane(x*2+1, y*2, c) + copy.GetPlane(x*2, y*2+1, c) + copy.GetPlane(x*2+1, y*2+1, c)+2)/4);
}

#ifdef J2K_DECODE_SUPPORT

void HandleJ2KDecodeError(const char *msg, void *client_data)
{
    if (!msg || strlen(msg) == 0)
        cerr << "J2K decode failed with unknown reason!" << endl;
    else
        cerr << "J2K decode failed with reason " << msg << "!" << endl;
}

void HandleJ2KDecodeWarning(const char *msg, void *client_data)
{
    if (!msg || strlen(msg) == 0)
        cerr << "J2K decode: unknown warning!" << endl;
    else
        cerr << "J2K decode warning: " << msg << "!" << endl;
}

void HandleJ2KDecodeInfo(const char *msg, void *client_data)
{
    if (!msg || strlen(msg))
        cerr << "J2K decode: unknown info!" << endl;
    else
        cerr << "J2K decode info: " << msg << "." << endl;
}

/// Decodes the given J2K buffer and outputs the raw image to outputData.
void DecodeJ2K(unsigned char *data, int numBytes, RawImage &outputData)
{
    opj_codestream_info_t cstr_info;  // codestream info
    memset(&cstr_info, 0, sizeof(opj_codestream_info_t));
       
    // Set up the decoding parameter structure.
    opj_dparameters_t parameters;
    opj_set_default_decoder_parameters(&parameters);
    parameters.cp_reduce = 0; // Decode the best quality level straight away.
    
    // Create the decoder instance.
    opj_dinfo_t *dinfo = opj_create_decompress(CODEC_J2K); 
    opj_setup_decoder(dinfo, &parameters);

    // Set up the decoder event manager.
    opj_event_mgr_t event_mgr;
    memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
    event_mgr.error_handler = HandleJ2KDecodeError;
    event_mgr.warning_handler = HandleJ2KDecodeWarning;
    event_mgr.info_handler = HandleJ2KDecodeInfo;
    opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, 0);

    // Initialize a decoder stream.
    opj_cio_t *cio = opj_cio_open((opj_common_ptr)dinfo, data, numBytes);
    
    /// Do the actual decoding.
    opj_image_t *image = opj_decode_with_info(dinfo, cio, &cstr_info);

    opj_cio_close(cio);
    opj_destroy_decompress(dinfo);
    
    if (!image)
    {
        cerr << "Image decode failed: opj_image_t *image is null!" << endl;
        return;
    }
    if (image->numcomps == 0)
    {
        opj_image_destroy(image);
        cerr << "Image decode failed: no color components in image!" << endl;
        return;
    }

    // All the color planes must be of same size.
    for(int i = 0; i < image->numcomps; ++i)
        if (image->comps[i].w != image->comps[0].w || image->comps[i].h != image->comps[0].h)
        {
            cerr << "Image component " << (i+1) << " has size " << image->comps[i].w << "x" << image->comps[i].h << 
                ", whereas the first component has different size " << image->comps[0].w << "x" << image->comps[0].h << "!" << endl;
            opj_image_destroy(image);
            return;
        }

    outputData.width = image->comps[0].w;
    outputData.height = image->comps[0].h;
    outputData.data.resize(image->comps[0].w * image->comps[0].h * image->numcomps);
    outputData.numColorPlanes = image->numcomps;

    // Convert the data from planar to interleaved.
    unsigned char *outByteData = &outputData.data[0];
    for (int y = 0; y < image->comps[0].h; ++y)
        for (int x = 0; x < image->comps[0].w; ++x)
            for (int c = 0; c < image->numcomps; ++c)
                *outByteData++ = image->comps[c].data[y * image->comps[0].w + x];

     opj_image_destroy(image);
}

void OpenAndDecodeUsingJ2K(const char *filename, RawImage &outputData)
{
    cout << "Opening J2K file \"" << filename << "\"" << endl;

    FILE *handle = fopen(filename, "rb");
    if (!handle)
    {
        cerr << "Failed to open " << filename << "!" << endl;
        return;
    }

    fseek(handle, 0, SEEK_END);
    long filesize = ftell(handle);
    if (filesize <= 0)
    {
        cerr << "Error: file size was " << filesize << "!" << endl;
        fclose(handle);
        return;
    }
    fseek(handle, 0, SEEK_SET);

    vector<unsigned char> data;
    data.resize(filesize);

    fread(&data[0], sizeof(unsigned char), filesize, handle);
    fclose(handle); 

    DecodeJ2K(&data[0], data.size(), outputData);
}

#endif // J2K_DECODE_SUPPORT

void OpenUsingD3DX(const char *filename, RawImage &outputData, LPDIRECT3DDEVICE9 device)
{
    cout << "Opening D3DX file \"" << filename << "\"" << endl;

    LPDIRECT3DTEXTURE9 texture = 0;
    D3DXIMAGE_INFO imageInfo;
    try
    {
    WIN32TRY(D3DXCreateTextureFromFileEx(device, filename, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, D3DUSAGE_DYNAMIC,
        D3DFMT_UNKNOWN, D3DPOOL_SYSTEMMEM, D3DX_DEFAULT, D3DX_DEFAULT, 0, &imageInfo, 0, &texture),
        string("Failed to open file \"") + filename + "\"!");
    } catch(const std::exception &e)
    {
        return;
    }

    int bytesPerPixel = 0;
    switch(imageInfo.Format)
    {
    case D3DFMT_A8R8G8B8: bytesPerPixel = 4; outputData.numColorPlanes = 4; break;
    case D3DFMT_X8R8G8B8: bytesPerPixel = 4; outputData.numColorPlanes = 4; break;
//    case D3DFMT_X8R8G8B8: bytesPerPixel = 4; outputData.numColorPlanes = 3; break; ///\todo To save space, would like to save in R8G8B8.
    case D3DFMT_R8G8B8: bytesPerPixel = 3; outputData.numColorPlanes = 3; break;
    case D3DFMT_P8: bytesPerPixel = 1; outputData.numColorPlanes = 1; break;
    default: cerr << "Don't know how to handle D3DFORMAT " << imageInfo.Format << endl; return;
    }

    outputData.format = imageInfo.Format;
    outputData.width = imageInfo.Width;
    outputData.height = imageInfo.Height;
    outputData.data.resize(outputData.width * outputData.height * outputData.numColorPlanes);

    D3DLOCKED_RECT rect;
    ZeroMemory(&rect, sizeof(rect));
    WIN32TRY(texture->LockRect(0, &rect, NULL, 0), "Failed to lock texture surface!");

    // Copy the image to the raw surface.
    unsigned char *sourceData = (unsigned char *)rect.pBits;
    for(UINT y = 0; y < imageInfo.Height; ++y)
        for(UINT x = 0; x < imageInfo.Width; ++x)
            for(int i = 0; i < outputData.numColorPlanes; ++i)
                if (outputData.format == D3DFMT_X8R8G8B8 && i == 3)
                    outputData.SetPlane(x, y, i, 255);
                else
                    outputData.SetPlane(x, y, i, sourceData[y * rect.Pitch + x*bytesPerPixel + i]);

    texture->UnlockRect(0);
    texture->Release();
}

/// @return The number of mip levels used by a texture of size (w,h).
int CountMaxMipSize(int w, int h)
{
    if (w <= 1 && h <= 1)
        return 1;
    int mipLevels = 1;
    while(w > 1 && h > 1)
    {
        w /= 2;
        h /= 2;
        if (w == 0) w = 1;
        if (h == 0) h = 1;
        ++mipLevels;
    }
    return mipLevels;
}

/// Opens the given filename, runs it through a few processing steps, and saves it as .dds file with the same base name.
/// \todo alphaChoose offers one method for mass-filtering from a set of input files which files to process, but perhaps
/// something more flexible was better, like a command line flag '--onlyifformat==D3DFMT_A8R8G8B8'?
/// alphaChoose: If true, only processes the file if it has alpha channel. (These are intended to be later saved as DXT5)
///              If false, only processes the file if it does not have an alpha channel. (These are to be later saved as DXT1)
void ConvertFileToDDS(std::string filename, LPDIRECT3DDEVICE9 device, bool alphaChoose, int maxTexSize)
{
    RawImage imageData;
#ifdef J2K_DECODE_SUPPORT
    ///\bug Comparisons in tolower.
    if (filename.find(".Texture") != string::npos || // We assume that a file with ending .Texture is a J2K texture
        filename.find(".j2k") != string::npos ||     // I am not sure whether the J2K or JP2 are actually the same file format,
        filename.find(".jp2") != string::npos)       // but match both here just in case.
    {
        OpenAndDecodeUsingJ2K(filename.c_str(), imageData);
        if (imageData.HasAlphaChannel()) // J2K files with alpha are decoded into ABGR8888 format, but we want to do all future processing in ARGB8888.            
            ABGR8888ToARGB8888(imageData);
        else // J2K files without alpha are decoded into BGR888 format, but we want to do all future processing in RGB888.
           BGR888ToRGB888(imageData);

        switch(imageData.numColorPlanes)
        {
        case 4: imageData.format = D3DFMT_A8R8G8B8; break;
        case 3: imageData.format = D3DFMT_R8G8B8; break;
        case 2: imageData.format = D3DFMT_A8L8; break;
        case 1: imageData.format = D3DFMT_L8; break;
        }

    }
    else
#endif
        OpenUsingD3DX(filename.c_str(), imageData, device);

    if (imageData.width == 0 || imageData.height == 0)
    {
        cout << "Failed opening file \"" << filename << "\". Image width or height came out 0!" << endl;
        return;
    }

    // PROCESSING STEP #1: Remove alpha channel that is not needed.
    if (imageData.numColorPlanes == 4 && imageData.HasRedundantAlphaChannel())
    {
        cout << "Removing unnecessary alpha component from image with " << imageData.numColorPlanes << " color components." << endl;
        ARGB8888ToRGB888(imageData);
    }

    // PROCESSING STEP #2: Repeatedly halve the image size until we are in acceptable size.
    /// \todo Make this step optional behind a command line flag '-maxsize=512' or something. Now veryvery hardcoded.
    int oldW = imageData.width;
    int oldH = imageData.height;
    while(imageData.width > maxTexSize || imageData.height > maxTexSize)
        HalveTextureSize(imageData);
    if (oldW != imageData.width || oldH != imageData.height)
        cout << "Resized texture from " << oldW << "x" << oldH << " to " << imageData.width << "x" << imageData.height << endl;

    // Create a D3D texture we'll use for saving the file.
    if (!alphaChoose && (imageData.format == D3DFMT_A8R8G8B8 || imageData.format == D3DFMT_A8L8))
    {
        cout << "Skipping processing file. It has an alpha channel." << endl;
        return;
    }
    if (alphaChoose && (imageData.format == D3DFMT_X8R8G8B8 || imageData.format == D3DFMT_R8G8B8 || imageData.format == D3DFMT_L8))
    {
        cout << "Skipping processing file. It does not have an alpha channel." << endl;
        return;
    }

//    const int maxMipLevels = CountMaxMipSize(imageData.width, imageData.height);
    /// @bug Don't create full mip chain, but instead set the last mip level to 4x4. For some reason, the 2x2 and 1x1 levels get corrupted in DXT conversion.
    /// To work on fixing this, uncomment the above, or use D3DX_DEFAULT below.
    const int maxMipLevels = CountMaxMipSize(imageData.width, imageData.height) - 2;
    LPDIRECT3DTEXTURE9 texture;
    WIN32TRY(D3DXCreateTexture(device, imageData.width, imageData.height, /*D3DX_DEFAULT*/maxMipLevels, D3DUSAGE_DYNAMIC, imageData.format, D3DPOOL_SYSTEMMEM, &texture),
        "Failed to create texture object for output!");

    // PROCESSING STEP #3: Generate all mipmap levels. For some reason, D3D doesn't do this automatically when loading (if the source lacked the mip levels).
    // (might be missing a flag).
    int level = 0;
    while(level < maxMipLevels)
    {
        D3DLOCKED_RECT rect;
        ZeroMemory(&rect, sizeof(rect));
        WIN32TRY(texture->LockRect(level, &rect, NULL, 0),
            string("Failed to lock surface level when generating mipmaps!"));
        memcpy(rect.pBits, &imageData.data[0], imageData.data.size());
        texture->UnlockRect(level);
        ++level;
        HalveTextureSize(imageData);
    }

    // Save to file. Convert the output filename ending to '.dds'.
    std::string outfile = std::string(filename);
    int idx = outfile.find_last_of('.');
    if (idx != string::npos)
        outfile.replace(idx, outfile.length()-idx, ".dds");
    else
        outfile = outfile + ".dds";
        
    D3DXSaveTextureToFile(outfile.c_str(), D3DXIFF_DDS, texture, 0);
    texture->Release();
}

// Taken from http://msdn.microsoft.com/en-us/library/ms633575(VS.85).aspx
BOOL InitApplication(HINSTANCE hinstance)
{ 
    WNDCLASSEX wcx; 
 
    // Fill in the window class structure with parameters 
    // that describe the main window. 
 
    wcx.cbSize = sizeof(wcx);          // size of structure 
    wcx.style = CS_HREDRAW | 
        CS_VREDRAW;                    // redraw if size changes 
    wcx.lpfnWndProc = DefWindowProc;   // points to window procedure 
    wcx.cbClsExtra = 0;                // no extra class memory 
    wcx.cbWndExtra = 0;                // no extra window memory 
    wcx.hInstance = hinstance;         // handle to instance 
    wcx.hIcon = LoadIcon(NULL, 
        IDI_APPLICATION);              // predefined app. icon 
    wcx.hCursor = LoadCursor(NULL, 
        IDC_ARROW);                    // predefined arrow 
    wcx.hbrBackground = NULL;
    wcx.lpszMenuName =  "MainMenu";    // name of menu resource 
    wcx.lpszClassName = "MainWClass";  // name of window class 
    wcx.hIconSm = NULL; 
 
    // Register the window class. 
 
    return RegisterClassEx(&wcx); 
} 

HWND CreateMainWindow()
{
    // Taken from http://msdn.microsoft.com/en-us/library/ms632598(VS.85).aspx .

    // Create the main window. 
     
    BOOL success = InitApplication(NULL);
    if (success == 0)
        return 0;

    HWND hwnd = CreateWindowEx( 
        0,                      // no extended styles           
        "MainWClass",           // class name                   
        "Main Window",          // window name                  
        WS_OVERLAPPEDWINDOW |   // overlapped window            
                 WS_HSCROLL |   // horizontal scroll bar        
                 WS_VSCROLL,    // vertical scroll bar          
        CW_USEDEFAULT,          // default horizontal position  
        CW_USEDEFAULT,          // default vertical position    
        CW_USEDEFAULT,          // default width                
        CW_USEDEFAULT,          // default height               
        (HWND) NULL,            // no parent or owner window    
        (HMENU) NULL,           // class menu used              
        NULL,              // instance handle              
        NULL);                  // no window creation data      

    return hwnd;
}

/// Creates a D3D9 device suitable for windowless data processing uses.
LPDIRECT3DDEVICE9 InitD3DDevice(LPDIRECT3D9 d3d9)
{
    if (!d3d9)
    {
        cout << "Direct3DCreate9 failed!" << endl;
        return 0;
    }

    HWND wnd = CreateMainWindow();
    LPDIRECT3DDEVICE9 d3dDevice9 = 0;
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    // We are creating a REF device without a window, so it doesn't matter at all what we put here, as long as they are valid values.
    d3dpp.BackBufferWidth = 640;
    d3dpp.BackBufferHeight = 480;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.Windowed = TRUE;
    d3dpp.hDeviceWindow = wnd;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
//WIN32TRY(d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE, &d3dpp, &d3dDevice9),
//        "Failed to create a D3DDevice for data processing!");
    WIN32TRY(d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE, &d3dpp, &d3dDevice9),
        "Failed to create a D3DDevice for data processing!");

    return d3dDevice9;
}

int ParseParameter(int argc, char **argv, const char *param, bool hasValue, int valueIfMissing)
{
    if (!hasValue)
    {
        for(int i = 1; i < argc; ++i)
            if (!strcmp(argv[i], param))
                return 1; // true
    }
    else
    {
        for(int i = 1; i+1 < argc; ++i)
            if (!strcmp(argv[i], param))
            {
                int value = atoi(argv[i+1]);
                if (value == 0 && !!strcmp("0", argv[i+1]))
                    return valueIfMissing;
                return value;
            }
    }
    
    return valueIfMissing;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cout << "This tool converts the given input file from any file format to .dds. The surface format of the .dds is chosen to match the original input file." << endl;
        cout << "Usage: " << argv[0] << " inputFilename [--hasalpha] [--maxtexsize pow2number]" << endl;
        cout << "--hasalpha: If specified, the input file will be converted only if it has an alpha channel." << endl;
        cout << "If NOT specified, the input file will be converted only it it DOES NOT contain an alpha channel." << endl;
        cout << "--maxtexsize: If a value is specified, the input file will be shrunk (retaining aspect ratio) so its width and height are smaller or equal to the given limit." << endl;
        return 0;
    }

    LPDIRECT3D9 d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

    ///\todo Provide a better method for command line input filtering (boost::program_options or something).
    bool alphaChoose = (ParseParameter(argc, argv, "--hasalpha", false, 0) != 0);
    int maxTexSize = ParseParameter(argc, argv, "--maxtexsize", true, 16384);

    LPDIRECT3DDEVICE9 device = InitD3DDevice(d3d9);
    ConvertFileToDDS(argv[1], device, alphaChoose, maxTexSize);

    device->Release();
    d3d9->Release();
}
