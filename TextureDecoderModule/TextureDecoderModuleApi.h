// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoderModuleApi_h
#define incl_TextureDecoderModuleApi_h

#if defined (_WINDOWS)
#if defined(TEXTURE_DECODER_MODULE_EXPORTS) 
#define TEXTURE_DECODER_MODULE_API __declspec(dllexport)
#else
#define TEXTURE_DECODER_MODULE_API __declspec(dllimport)
#endif
#else
#define TEXTURE_DECODER_MODULE_API
#endif

#endif
