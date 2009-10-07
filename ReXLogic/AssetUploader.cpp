// For conditions of distribution and use, see copyright notice in license.txt

/// @file AssetUploader.cpp
/// @brief Asset HTTP uploading functionality for OS & reX servers.

#include "StableHeaders.h"
#include "AssetUploader.h"
#include "RexLogicModule.h"
#include "HttpRequest.h"
#include "LLSDUtilities.h"
#include "RexUUID.h"
#include "InventoryModel.h"

#include "openjpeg.h"
#include "curl/curl.h"
#include <QImage>

namespace
{

void J2kErrorCallback(const char *msg, void *)
{
    std::cout << "J2kErrorCallback: " << msg << std::endl;
}

void J2kWarningCallback(const char *msg, void *)
{
    std::cout << "J2kWarningCallback: " << msg << std::endl;
}

void J2kInfoCallback(const char *msg, void *)
{
    std::cout << "J2kInfoCallback: " << msg << std::endl;
}

// Code adapted from LibOpenJpeg (http://www.openjpeg.org/index.php?menu=download), file image_to_j2k.c.
bool J2kEncode(QImage q_image, char *outbuf, size_t *size, bool reversible)
{
//    char *outbuf;
    bool success;
    opj_cparameters_t parameters;   // compression parameters
    opj_event_mgr_t event_mgr;      // event manager
    const int cMaxComponents = 5;

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
    int width = q_image.width();
    int height = q_image.height();
    int num_comps = 3;

    if (q_image.hasAlphaChannel())
        ++num_comps;

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
    const Core::u8 *bits = q_image.bits();
    for (int x = height - 1; x >= 0; --x)
    {
        for(int y = 0; y < width; ++y)
        {
            const Core::u8 *pixel = bits + (x * width + y) * num_comps;
            for(int c = 0; c < num_comps; ++c)
            {
                image->comps[c].data[i] = *pixel;
                ++pixel;
            }
            
            ++i;
        }
    }

    // Encode the destination image.
//    int codestream_length;
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
    success = opj_encode(cinfo, cio, image, parameters.index);
    if (!success)
    {
        opj_cio_close(cio);
        std::cerr << "Failed to encode image." << std::endl;
        return false;
    }

    // Write encoded data to output buffer.
    *size /*codestream_length*/ = cio_tell(cio);

    outbuf = (char*)malloc(*size/*codestream_length*/);
//    *size_t = codestream_length;
//outbuf.resize(codestream_length);
    
    memcpy(outbuf, cio->buffer, *size/*codestream_length*/);

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

    //return outbuf;
    return true;
}

}

namespace RexLogic
{

AssetUploader::AssetUploader() : uploadCapability_("")
{
}

AssetUploader::~AssetUploader()
{
}

void AssetUploader::UploadFile(
    const RexTypes::asset_type_t &asset_type,
    const std::string &filename,
    const std::string &name,
    const std::string &description,
    const RexTypes::RexUUID &folder_id)
{
    if (uploadCapability_ == "")
    {
        RexLogicModule::LogError("Upload capability not set! Uploading not possible.");
        return;
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
        return;
    }

    std::vector<Core::u8> response;
    response = request.GetResponseData();

    if (response.size() == 0)
    {
        RexLogicModule::LogError("Size of the response data to \"NewFileAgentInventory\" message was zero.");
        return;
    }

    response.push_back('\0');
    std::string response_str = (char *)&response[0];

    // Parse the upload url from the response.
    std::map<std::string, std::string> llsd_map = RexTypes::ParseLLSDMap(response_str);
    std::string upload_url = llsd_map["uploader"];
    if (upload_url == "")
    {
        RexLogicModule::LogError("Invalid response data for uploading an asset.");
        return;
    }

    // Open the file.
    std::filebuf *pbuf;
    size_t size;
    char *buffer;
    std::vector<Core::u8> buffer_vec;

    // If the file is texture, use QImage and J2k encoding.
    if (asset_type == RexTypes::RexAT_Texture)
    {
        QImage img(filename.c_str());
        buffer = (char *)malloc(img.numBytes());
        bool success = J2kEncode(img, buffer, &size, false);
        if (!success)
        {
            RexLogicModule::LogError("Could not J2k encode the image file.");
            return;
        }
    }
    else
    {
        // Other assets can be uploaded as raw data.
        std::ifstream file(filename.c_str(), std::ios::binary);
        if (!file.is_open())
        {
            RexLogicModule::LogError("Could not open file the file: " + filename + ".");
            return;
        }

        pbuf = file.rdbuf();
        size = pbuf->pubseekoff(0, std::ios::end, std::ios::in);
        buffer_vec.resize(size);
        pbuf->pubseekpos(0, std::ios::in);
        //buffer = new char[size];
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
        RexLogicModule::LogError("HTTP POST asset upload did not succeed. " + request.GetReason());
        return;
    }

    response = request2.GetResponseData();

    if (response.size() == 0)
    {
        RexLogicModule::LogError("Size of the response data to file upload was zero.");
        return;
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
        return;
    }

    RexLogicModule::LogInfo("Upload succesfull. Asset id: " + asset_id + ", inventory id: " + inventory_id + ".");
}

void AssetUploader::UploadFiles(Core::StringList filenames, OpenSimProtocol::InventoryModel *inventory)
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

        RexTypes::RexUUID folder_id = inventory->GetFirstChildFolderByName(cat_name.c_str())->GetID();
        if (folder_id.IsNull())
        {
            RexLogicModule::LogError("Inventory folder for this type of file doesn't exists. File can't be uploaded.");
            continue;
        }

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
            return;
        }

        std::vector<Core::u8> response;
        response = request.GetResponseData();

        if (response.size() == 0)
        {
            RexLogicModule::LogError("Size of the response data to \"NewFileAgentInventory\" message was zero.");
            return;
        }

        // Convert the response data to a string.
        response.push_back('\0');
        std::string response_str = (char *)&response[0];

        // Parse the upload url from the response.
        std::map<std::string, std::string> llsd_map = RexTypes::ParseLLSDMap(response_str);
        std::string upload_url = llsd_map["uploader"];
        if (upload_url == "")
        {
            RexLogicModule::LogError("Invalid response data for uploading an asset.");
            return;
        }

        // Open the file.
        std::filebuf *pbuf;
        size_t size;
        char *buffer;
        std::vector<Core::u8> buffer_vec;

        // If the file is texture, use QImage and J2k encoding.
        if (asset_type == RexTypes::RexAT_Texture)
        {
            QImage img(filename.c_str());
            buffer = (char *)malloc(img.numBytes());
            bool success = J2kEncode(img, buffer, &size, false);
            if (!success)
            {
                RexLogicModule::LogError("Could not J2k encode the image file.");
                continue;
            }
        }
        else
        {
            std::ifstream file(filename.c_str(), std::ios::binary);
            if (!file.is_open())
            {
                RexLogicModule::LogError("Could not open file the file: " + filename + ".");
                continue;
            }

            pbuf = file.rdbuf();
            size = pbuf->pubseekoff(0, std::ios::end, std::ios::in);
            pbuf->pubseekpos(0, std::ios::in);
            //buffer = new char[size];
            buffer_vec.resize(size);
            pbuf->sgetn((char *)&buffer_vec[0], size);
            file.close();
        }

        // Upload the file.
        response.clear();

        HttpUtilities::HttpRequest request2;
        request2.SetUrl(upload_url);
        request2.SetMethod(HttpUtilities::HttpRequest::Post);
        request2.SetRequestData("application/octet-stream", buffer_vec);
        request2.Perform();
        
        if (!request2.GetSuccess())
        {
            RexLogicModule::LogError("HTTP POST asset upload did not succeed. " + request2.GetReason());
            return;
        }

        response = request2.GetResponseData();

        if (response.size() == 0)
        {
            RexLogicModule::LogError("Size of the response data to file upload was zero.");
            return;
        }

        // Convert the response data to a string.
        response.push_back('\0');
        response_str.clear();
        response_str = (char *)&response[0];
        llsd_map.clear();

        llsd_map = RexTypes::ParseLLSDMap(response_str);
        std::string asset_id = llsd_map["new_asset"];
        std::string inventory_id = llsd_map["new_inventory_item"];
        if (asset_id == "" || inventory_id == "")
        {
            RexLogicModule::LogError("Invalid XML response data for uploading an asset.");
            return;
        }

        RexLogicModule::LogInfo("Upload succesfull. Asset id: " + asset_id + ", inventory id: " + inventory_id + ".");
        ++asset_count;

        // Create the asset to inventory.
        OpenSimProtocol::InventoryFolder *folder = inventory->GetChildFolderByID(folder_id);
        OpenSimProtocol::InventoryAsset *asset = inventory->GetOrCreateNewAsset(
            RexTypes::RexUUID(inventory_id),RexTypes::RexUUID(asset_id), *folder);
        asset->SetName(name);
        asset->SetDescription(description);
    }

    RexLogicModule::LogInfo("Multiupload:" + Core::ToString(asset_count) + " assets succesfully uploaded.");
}

std::string AssetUploader::CreateNameFromFilename(std::string filename)
{
    std::string name = "asset";
    bool no_path = false;
    size_t name_start_pos = filename.find_last_of('/');
    // If the filename doens't have the full path, start from index 0.
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
