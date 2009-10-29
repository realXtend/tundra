// For conditions of distribution and use, see copyright notice in license.txt

/// @file AssetUploader.cpp
/// @brief Asset HTTP uploading functionality for OS & reX servers.

#include "StableHeaders.h"
#include "AssetUploader.h"
#include "RexLogicModule.h"
#include "HttpRequest.h"
#include "LLSDUtilities.h"
#include "RexUUID.h"
#include "InventoryEvents.h"

#include "openjpeg.h"
#include <Ogre.h>

namespace
{

using namespace RexLogic;

void J2kErrorCallback(const char *msg, void *)
{
    RexLogicModule::LogError("J2kErrorCallback: " + std::string(msg));
}

void J2kWarningCallback(const char *msg, void *)
{
    RexLogicModule::LogWarning("J2kWarningCallback: " + std::string(msg));
}

void J2kInfoCallback(const char *msg, void *)
{
    RexLogicModule::LogInfo("J2kInfoCallback: " + std::string(msg));
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
bool J2kEncode(Ogre::Image &src_image, std::vector<Core::u8> &outbuf, bool reversible)
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
        RexLogic::RexLogicModule::LogError("Zero image dimensions, cannot encode.");
        return false;
    }

    // Scale the image to next power-of-two size, if necessary
    // Otherwise old viewer will crash when trying to view the image
    if (!IsPowerOfTwo(width) || !IsPowerOfTwo(height))
    {
        int new_w = GetClosestPowerOfTwo(width);
        int new_h = GetClosestPowerOfTwo(height);
        RexLogic::RexLogicModule::LogInfo("Scaling image from " + Core::ToString<int>(width) + "x" + Core::ToString<int>(height) + " to " +
            Core::ToString<int>(new_w) + "x" + Core::ToString<int>(new_h));

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
    event_mgr.error_handler = J2kErrorCallback;
    event_mgr.warning_handler = J2kWarningCallback;
    event_mgr.info_handler = J2kInfoCallback;

    // Set encoding parameters to default values.
    opj_set_default_encoder_parameters(&parameters);
    parameters.cod_format = 0; // 0 == J2K_CFMT
    parameters.cp_disto_alloc = 1;

    //DELETE?
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
    //END DELETE

    // Create comment for codestream.
    ///\todo Custom comments / no comments at all?
    if(parameters.cp_comment == NULL)
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
    cio = opj_cio_open((opj_common_ptr)cinfo, NULL, 0);

    // Encode the image.
    success = opj_encode(cinfo, cio, image, NULL);
    if (!success)
    {
        opj_cio_close(cio);
        opj_image_destroy(image);
        RexLogic::RexLogicModule::LogInfo("Failed to encode image.");
        return false;
    }

    // Write encoded data to output buffer.
    outbuf.resize(cio_tell(cio));
    RexLogicModule::LogDebug("J2k datastream size: " + outbuf.size());

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

namespace RexLogic
{

AssetUploader::AssetUploader(Foundation::Framework* framework) : framework_(framework), uploadCapability_("")
{
}

AssetUploader::~AssetUploader()
{
}

bool AssetUploader::UploadFile(
    const RexTypes::asset_type_t &asset_type,
    const std::string &filename,
    const std::string &name,
    const std::string &description,
    const RexTypes::RexUUID &folder_id)
{
    if (uploadCapability_ == "")
    {
        RexLogicModule::LogError("Upload capability not set! Uploading not possible.");
        return false;
    }

    // Create the asset uploading info XML message.
    std::string it_str = RexTypes::GetInventoryTypeString(asset_type);
    std::string at_str = RexTypes::GetAssetTypeString(asset_type);
    std::string asset_xml = CreateNewFileAgentInventoryXML(at_str, it_str, folder_id.ToString(), name, description);

    // Post NewFileAgentInventory message informing the server about upcoming asset upload.
    HttpUtilities::HttpRequest request;
    request.SetUrl(uploadCapability_);
    request.SetMethod(HttpUtilities::HttpRequest::Post);
    request.SetRequestData("application/xml", asset_xml);
    request.Perform();

    if (!request.GetSuccess())
    {
        RexLogicModule::LogError(request.GetReason());
        return false;
    }

    std::vector<Core::u8> response;
    response = request.GetResponseData();

    if (response.size() == 0)
    {
        RexLogicModule::LogError("Size of the response data to \"NewFileAgentInventory\" message was zero.");
        return false;
    }

    response.push_back('\0');
    std::string response_str = (char *)&response[0];

    // Parse the upload url from the response.
    std::map<std::string, std::string> llsd_map = RexTypes::ParseLLSDMap(response_str);
    std::string upload_url = llsd_map["uploader"];
    if (upload_url == "")
    {
        RexLogicModule::LogError("Invalid response data for uploading an asset.");
        return false;
    }

    // Open the file.
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file.is_open())
    {
        RexLogicModule::LogError("Could not open file the file: " + filename + ".");
        return false;
    }

    std::vector<Core::u8> buffer_vec;

    // If the file is texture, use Ogre image and J2k encoding.
    if (asset_type == RexTypes::RexAT_Texture)
    {
        Ogre::Image image;
        std::vector<Core::u8> src_vec;
        std::filebuf *pbuf = file.rdbuf();
        size_t size = pbuf->pubseekoff(0, std::ios::end, std::ios::in);
        src_vec.resize(size);
        pbuf->pubseekpos(0, std::ios::in);
        pbuf->sgetn((char *)&src_vec[0], size);
        file.close();

        try
        {
            Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)&src_vec[0], size, false));
            image.load(stream);
        }
        catch (Ogre::Exception& e)
        {
            RexLogicModule::LogError("Error loading image: " + std::string(e.what()));
            return false;
        }

        bool success = J2kEncode(image, buffer_vec, false);
        if (!success)
        {
            RexLogicModule::LogError("Could not J2k encode the image file.");
            return false;
        }
    }
    else
    {
        // Other assets can be uploaded as raw data.
        std::filebuf *pbuf = file.rdbuf();
        size_t size = pbuf->pubseekoff(0, std::ios::end, std::ios::in);
        buffer_vec.resize(size);
        pbuf->pubseekpos(0, std::ios::in);
        pbuf->sgetn((char *)&buffer_vec[0], size);
        file.close();
    }

    response.clear();
    response_str.clear();

    HttpUtilities::HttpRequest request2;
    request2.SetUrl(upload_url);
    request2.SetMethod(HttpUtilities::HttpRequest::Post);
    request2.SetRequestData("application/octet-stream", buffer_vec);
    request2.Perform();
    
    if (!request2.GetSuccess())
    {
        RexLogicModule::LogError("HTTP POST asset upload did not succeed: " + request.GetReason());
        return false;
    }

    response = request2.GetResponseData();

    if (response.size() == 0)
    {
        RexLogicModule::LogError("Size of the response data to file upload was zero.");
        return false;
    }

    response.push_back('\0');
    response_str = (char *)&response[0];
    llsd_map.clear();

    llsd_map = RexTypes::ParseLLSDMap(response_str);
    std::string asset_id = llsd_map["new_asset"];
    std::string inventory_id = llsd_map["new_inventory_item"];
    if (asset_id == "" || inventory_id == "")
    {
        RexLogicModule::LogError("Invalid XML response data for uploading an asset.");
        return false;
    }

    // Send event, if applicable.
    Foundation::EventManagerPtr event_mgr = framework_->GetEventManager();
    Core::event_category_id_t event_category = event_mgr->QueryEventCategory("Inventory");
    if (event_category != 0)
    {
        Inventory::InventoryItemEventData asset_data(Inventory::IIT_Asset);
        asset_data.id = RexTypes::RexUUID(inventory_id);
        asset_data.parentId = folder_id;
        asset_data.assetId = RexTypes::RexUUID(asset_id);
        asset_data.assetType = asset_type;
        asset_data.inventoryType = RexTypes::GetInventoryTypeFromAssetType(asset_type);
        asset_data.name = name;
        asset_data.description = description;
        event_mgr->SendEvent(event_category, Inventory::Events::EVENT_INVENTORY_DESCENDENT, &asset_data);
    }

    RexLogicModule::LogInfo("Upload succesfull. Asset id: " + asset_id + ", inventory id: " + inventory_id + ".");
    return true;
}

void AssetUploader::UploadFiles(Core::StringList filenames, OpenSimProtocol::InventorySkeleton *inventory)
{
    if (uploadCapability_ == "")
    {
        RexLogicModule::LogError("Upload capability not set! Uploading not possible.");
        return;
    }

    // Iterate trought every asset.
    int asset_count = 0;
    for(Core::StringList::iterator it = filenames.begin(); it != filenames.end(); ++it)
    {
        std::string filename = *it;
        RexTypes::asset_type_t asset_type = RexTypes::GetAssetTypeFromFilename(filename);
        if (asset_type == RexAT_None)
        {
            RexLogicModule::LogError("Invalid file extension. File can't be uploaded: " + filename);
            continue;
        }

        // Create the asset uploading info XML message.
        std::string it_str = RexTypes::GetInventoryTypeString(asset_type);
        std::string at_str = RexTypes::GetAssetTypeString(asset_type);
        std::string cat_name = RexTypes::GetCategoryNameForAssetType(asset_type);

        ///\todo User-defined name and desc when we got the UI.
        std::string name = CreateNameFromFilename(filename);
        std::string description = "(No Description)";

        RexTypes::RexUUID folder_id = inventory->GetFirstChildFolderByName(cat_name.c_str())->id;
        if (folder_id.IsNull())
        {
            RexLogicModule::LogError("Inventory folder for this type of file doesn't exists. File can't be uploaded.");
            continue;
        }

        if (UploadFile(asset_type, filename, name, description, folder_id))
            ++asset_count;
    }

    RexLogicModule::LogInfo("Multiupload:" + Core::ToString(asset_count) + " assets succesfully uploaded.");
}

std::string AssetUploader::CreateNameFromFilename(std::string filename)
{
    std::string name = "asset";
    bool no_path = false;
    size_t name_start_pos = filename.find_last_of('/');
    // If the filename doesn't have the full path, start from index 0.
    if (name_start_pos == std::string::npos)
    {
        name_start_pos = 0;
        no_path = true;
    }

    size_t name_end_pos = filename.find_last_of('.');
    if (name_end_pos != std::string::npos)
    {
        if (no_path)
            name = filename.substr(name_start_pos, name_end_pos - name_start_pos);
        else
            name = filename.substr(name_start_pos + 1, name_end_pos - name_start_pos - 1);
    }

    return name;
}

std::string AssetUploader::CreateNewFileAgentInventoryXML(
    const std::string &asset_type,
    const std::string &inventory_type,
    const std::string &folder_id,
    const std::string &name,
    const std::string &description)
{
    std::string xml = "<llsd><map><key>asset_type</key><string>";
    xml += asset_type;
    xml += "</string><key>description</key><string>";
    xml += description;
    xml += "</string><key>folder_id</key><uuid>";
    xml += folder_id;
    xml += "</uuid><key>inventory_type</key><string>";
    xml += inventory_type;
    xml += "</string><key>name</key><string>";
    xml += name;
    xml += "</string></map></llsd>";
    return xml;
}

} // namespace RexLogic
