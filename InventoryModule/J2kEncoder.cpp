// For conditions of distribution and use, see copyright notice in license.txt

/// @file J2kEncoder.cpp
/// @brief J2k encoding functionality required by image uploads.

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "J2kEncoder.h"
#include "InventoryModule.h"
#include <OgreColourValue.h>

#include "openjpeg.h"
#include "MemoryLeakCheck.h"

namespace J2k
{

using namespace Inventory;

void J2kErrorCallback(const char *msg, void *)
{
    InventoryModule::LogError("J2kErrorCallback: " + std::string(msg));
}

void J2kWarningCallback(const char *msg, void *)
{
    InventoryModule::LogWarning("J2kWarningCallback: " + std::string(msg));
}

void J2kInfoCallback(const char *msg, void *)
{
    InventoryModule::LogInfo("J2kInfoCallback: " + std::string(msg));
}

bool IsPowerOfTwo(int value)
{
    int bitcount = 0;
    for (int i = 0; i < 32; ++i)
    {
        if (value & 1)
            bitcount++;
        if (bitcount > 1)
            return false;
        value >>= 1;
    }
    return true;
}

int GetClosestPowerOfTwo(int value)
{
    int closest = 1;
    // Use 2048 as max. size
    for (int i = 11; i >= 0; --i)
    {
        int ptwo = 1 << i;
        if (abs(ptwo-value) < abs(closest-value))
            closest = ptwo;
    }
    return closest;
}

// Code adapted from LibOpenJpeg (http://www.openjpeg.org/index.php?menu=download), file image_to_j2k.c.
bool J2kEncode(Ogre::Image &src_image, std::vector<u8> &outbuf, bool reversible)
{
    bool success;
    opj_cparameters_t parameters;   // compression parameters
    opj_event_mgr_t event_mgr;      // event manager
    const int cMaxComponents = 5;

    // Check for zero size
    int width = src_image.getWidth();
    int height = src_image.getHeight();
    if (!width || !height)
    {
        InventoryModule::LogError("Zero image dimensions, cannot encode.");
        return false;
    }

    // Scale the image to next power-of-two size, if necessary
    // Otherwise old viewer will crash when trying to view the image
    if (!IsPowerOfTwo(width) || !IsPowerOfTwo(height))
    {
        int new_w = GetClosestPowerOfTwo(width);
        int new_h = GetClosestPowerOfTwo(height);
        InventoryModule::LogInfo("Scaling image from " + ToString<int>(width) + "x" + ToString<int>(height) + " to " +
            ToString<int>(new_w) + "x" + ToString<int>(new_h));

        // Uses bilinear filter
        src_image.resize(new_w, new_h);
        width = src_image.getWidth();
        height = src_image.getHeight();
    }

    int num_comps = 3;
    if (src_image.getHasAlpha())
        ++num_comps;

    // Configure the event callbacks (optional).
    memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
//    event_mgr.error_handler = J2kErrorCallback;
//    event_mgr.warning_handler = J2kWarningCallback;
//    event_mgr.info_handler = J2kInfoCallback;

    // Set encoding parameters to default values.
    opj_set_default_encoder_parameters(&parameters);
    parameters.cod_format = 0; // 0 == J2K_CFMT
    parameters.cp_disto_alloc = 1;

    if (reversible)
    {
        parameters.tcp_numlayers = 1;
        parameters.tcp_rates[0] = 0.0f;
    }
    else
    {
        parameters.tcp_numlayers = 5;
        parameters.tcp_rates[0] = 1920.0f;
        parameters.tcp_rates[1] = 480.0f;
        parameters.tcp_rates[2] = 120.0f;
        parameters.tcp_rates[3] = 30.0f;
        parameters.tcp_rates[4] = 10.0f;
    }

    // Create comment for codestream.
    ///\todo Custom comments / no comments at all?
    if(parameters.cp_comment == 0)
    {
        const char comment[] = "Created by OpenJPEG version ";
        const size_t clen = strlen(comment);
        const char *version = opj_version();
        parameters.cp_comment = (char*)malloc(clen + strlen(version)+1);
        sprintf(parameters.cp_comment,"%s%s", comment, version);
    }

    // Fill in the source image from our raw image
    // Code adapted from convert.c.
    OPJ_COLOR_SPACE color_space = CLRSPC_SRGB;
    opj_image_cmptparm_t cmptparm[cMaxComponents];
    opj_image_t *image = 0;

    memset(&cmptparm[0], 0, cMaxComponents * sizeof(opj_image_cmptparm_t));
    for(int c = 0; c < num_comps; c++)
    {
        cmptparm[c].prec = 8;
        cmptparm[c].bpp = 8;
        cmptparm[c].sgnd = 0;
        cmptparm[c].dx = parameters.subsampling_dx;
        cmptparm[c].dy = parameters.subsampling_dy;
        cmptparm[c].w = width;
        cmptparm[c].h = height;
    }

    // Decode the source image.
    image = opj_image_create(num_comps, &cmptparm[0], color_space);

    // Decide if MCT should be used.
    parameters.tcp_mct = image->numcomps >= 3 ? 1 : 0;

    image->x1 = width;
    image->y1 = height;

    int i = 0;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Ogre::ColourValue pixel = src_image.getColourAt(x,y,0);
            for (int c = 0; c < num_comps; ++c)
            {
                switch (c)
                {
                case 0:
                    image->comps[c].data[i] = pixel.r * 255.0;
                    break;
                case 1:
                    image->comps[c].data[i] = pixel.g * 255.0;
                    break;
                case 2:
                    image->comps[c].data[i] = pixel.b * 255.0;
                    break;
                case 3:
                    image->comps[c].data[i] = pixel.a * 255.0;
                    break;
                }
            }
            ++i;
        }
    }

    // Encode the destination image.
    opj_cio_t *cio = 0;

    // Get a J2K compressor handle.
    opj_cinfo_t* cinfo = opj_create_compress(CODEC_J2K);

    // Catch events using our callbacks and give a local context.
    opj_set_event_mgr((opj_common_ptr)cinfo, &event_mgr, stderr);

    // Setup the encoder parameters using the current image and user parameters.
    opj_setup_encoder(cinfo, &parameters, image);

    // Open a byte stream for writing. Allocate memory for all tiles.
    cio = opj_cio_open((opj_common_ptr)cinfo, 0, 0);

    // Encode the image.
    success = opj_encode(cinfo, cio, image, 0);
    if (!success)
    {
        opj_cio_close(cio);
        opj_image_destroy(image);
        InventoryModule::LogInfo("Failed to encode image.");
        return false;
    }

    // Write encoded data to output buffer.
    outbuf.resize(cio_tell(cio));
//    InventoryModule::LogDebug("J2k datastream size: " + outbuf.size());

    memcpy(&outbuf[0], cio->buffer, outbuf.size());

    // Close and free the byte stream.
    opj_cio_close(cio);

    // Free remaining compression structures.
    opj_destroy_compress(cinfo);

    // Free image data.
    opj_image_destroy(image);

    // Free user parameters structure.
    if (parameters.cp_comment)
        free(parameters.cp_comment);

    if (parameters.cp_matrice)
        free(parameters.cp_matrice);

    return true;
}

}
