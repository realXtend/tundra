// For conditions of distribution and use, see copyright notice in license.txt

/// @file AssetUploader.cpp
/// @brief Asset HTTP uploading functionality for OS & reX servers.

#include "StableHeaders.h"
#include "AssetUploader.h"
#include "RexLogicModule.h"
#include "InventoryModule.h"
#include "HttpRequest.h"
#include "LLSDUtilities.h"
#include "RexUUID.h"
#include "Inventory/InventoryEvents.h"
#include "Inventory/InventorySkeleton.h"
#include "J2kEncoder.h"

namespace Inventory
{

AssetUploader::AssetUploader(Foundation::Framework* framework, RexLogic::RexLogicModule *rexlogic) :
    framework_(framework), rexLogicModule_(rexlogic), uploadCapability_("")
{
}

AssetUploader::~AssetUploader()
{
}

void AssetUploader::UploadFiles(Core::StringList filenames)
{
    CreateRexInventoryFolders();

    if (!HasUploadCapability())
    {
        std::string upload_url = rexLogicModule_->GetServerConnection()->GetCapability("NewFileAgentInventory");
        if (upload_url == "")
        {
            InventoryModule::LogError("Could not get upload capability for asset uploader. Uploading not possible");
            return;
        }

        SetUploadCapability(upload_url);
    }

    Core::Thread thread(boost::bind(&AssetUploader::ThreadedUploadFiles, this, filenames));
}

/*
bool AssetUploader::UploadFile(
    const RexTypes::asset_type_t &asset_type,
    const std::string &filename,
    const std::string &name,
    const std::string &description,
    const RexTypes::RexUUID &folder_id)
{
    using namespace OpenSimProtocol;

    if (!HasUploadCapability())
    {
        std::string upload_url = rexLogicModule_->GetServerConnection()->GetCapability("NewFileAgentInventory");
        if (upload_url == "")
        {
            InventoryModule::LogError("Could not get upload capability for asset uploader. Uploading not possible");
            return false;
        }

        SetUploadCapability(upload_url);
    }

    RexLogic::InventoryPtr inventory = rexLogicModule_->GetInventory();

    // Get the category name for this asset type.
    std::string cat_name = GetCategoryNameForAssetType(asset_type);
    RexUUID folder_id;

    // Check out if this inventory category exists.
    InventoryFolderSkeleton *folder = inventory->GetFirstChildFolderByName(cat_name.c_str());
    if (!folder)
    {
        // I doesn't. Create new inventory folder.
        InventoryFolderSkeleton *parentFolder = inventory->GetMyInventoryFolder();
        folder_id.Random();

        // Add folder to inventory skeleton.
        InventoryFolderSkeleton newFolder(folder_id, cat_name);
        parentFolder->AddChildFolder(newFolder);

        // Notify the server about the new inventory folder.
        rexLogicModule_->GetServerConnection()->SendCreateInventoryFolderPacket(parentFolder->id, folder_id, asset_type, cat_name);
    }
    else
        folder_id = folder->id;

    Core::Thread thread(boost::bind(&AssetUploader::ThreadedUploadFile, this, asset_type, filename, name, description, folder_id));
    return true;
}
*/

bool AssetUploader::UploadFile(
    const RexTypes::asset_type_t &asset_type,
    const std::string &filename,
    const std::string &name,
    const std::string &description,
    const RexTypes::RexUUID &folder_id)
{
    if (uploadCapability_ == "")
    {
        InventoryModule::LogError("Upload capability not set! Uploading not possible.");
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
        InventoryModule::LogError(request.GetReason());
        return false;
    }

    std::vector<Core::u8> response;
    response = request.GetResponseData();

    if (response.size() == 0)
    {
        InventoryModule::LogError("Size of the response data to \"NewFileAgentInventory\" message was zero.");
        return false;
    }

    response.push_back('\0');
    std::string response_str = (char *)&response[0];

    // Parse the upload url from the response.
    std::map<std::string, std::string> llsd_map = RexTypes::ParseLLSDMap(response_str);
    std::string upload_url = llsd_map["uploader"];
    if (upload_url == "")
    {
        InventoryModule::LogError("Invalid response data for uploading an asset.");
        return false;
    }

    // Open the file.
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file.is_open())
    {
        InventoryModule::LogError("Could not open file the file: " + filename + ".");
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
        catch (Ogre::Exception &e)
        {
            InventoryModule::LogError("Error loading image: " + std::string(e.what()));
            return false;
        }

        bool success = J2k::J2kEncode(image, buffer_vec, false);
        if (!success)
        {
            InventoryModule::LogError("Could not J2k encode the image file.");
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
        InventoryModule::LogError("HTTP POST asset upload did not succeed: " + request.GetReason());
        return false;
    }

    response = request2.GetResponseData();

    if (response.size() == 0)
    {
        InventoryModule::LogError("Size of the response data to file upload was zero.");
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
        InventoryModule::LogError("Invalid XML response data for uploading an asset.");
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

    InventoryModule::LogInfo("Upload succesfull. Asset id: " + asset_id + ", inventory id: " + inventory_id + ".");
    return true;
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

void AssetUploader::ThreadedUploadFiles(Core::StringList filenames)
{
    // Iterate trought every asset.
    int asset_count = 0;
    for(Core::StringList::iterator it = filenames.begin(); it != filenames.end(); ++it)
    {
        std::string filename = *it;
        RexTypes::asset_type_t asset_type = RexTypes::GetAssetTypeFromFilename(filename);
        if (asset_type == RexAT_None)
        {
            InventoryModule::LogError("Invalid file extension. File can't be uploaded: " + filename);
            continue;
        }

        // Create the asset uploading info XML message.
        std::string it_str = RexTypes::GetInventoryTypeString(asset_type);
        std::string at_str = RexTypes::GetAssetTypeString(asset_type);
        std::string cat_name = RexTypes::GetCategoryNameForAssetType(asset_type);

        ///\todo User-defined name and desc when we got the UI.
        std::string name = CreateNameFromFilename(filename);
        std::string description = "(No Description)";

        RexTypes::RexUUID folder_id = rexLogicModule_->GetInventory()->GetFirstChildFolderByName(cat_name.c_str())->id;
        if (folder_id.IsNull())
        {
            InventoryModule::LogError("Inventory folder for this type of file doesn't exists. File can't be uploaded.");
            continue;
        }

        if (UploadFile(asset_type, filename, name, description, folder_id))
            ++asset_count;
    }

    InventoryModule::LogInfo("Multiupload:" + Core::ToString(asset_count) + " assets succesfully uploaded.");
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

void AssetUploader::CreateRexInventoryFolders()
{
    RexLogic::InventoryPtr inventory = rexLogicModule_->GetInventory();
    if (!inventory.get())
    {
        InventoryModule::LogError("Inventory doens't exist yet! Can't create folder to it.");
        return;
    }

    using namespace RexTypes;

    const char *asset_types[] = { "Texture", "Mesh", "Skeleton", "MaterialScript", "ParticleScript", "FlashAnimation" };
    asset_type_t asset_type;
    for(int i = 0; i < NUMELEMS(asset_types); ++i)
    {
        asset_type = GetAssetTypeFromTypeName(asset_types[i]);
        std::string cat_name = GetCategoryNameForAssetType(asset_type);

        // Check out if this inventory category exists.
        ProtocolUtilities::InventoryFolderSkeleton *folder = inventory->GetFirstChildFolderByName(cat_name.c_str());
        if (!folder)
        {
            // I doesn't. Create new inventory folder.
            ProtocolUtilities::InventoryFolderSkeleton *parentFolder = inventory->GetMyInventoryFolder();
            RexUUID folder_id = RexUUID::CreateRandom();

            // Add folder to inventory skeleton.
            ProtocolUtilities::InventoryFolderSkeleton newFolder(folder_id, cat_name);
            parentFolder->AddChildFolder(newFolder);

            // Notify the server about the new inventory folder.
            rexLogicModule_->GetServerConnection()->SendCreateInventoryFolderPacket(parentFolder->id, folder_id, asset_type, cat_name);

            // Send event to inventory module.
            Foundation::EventManagerPtr event_mgr = framework_->GetEventManager();
            Core::event_category_id_t event_category = event_mgr->QueryEventCategory("Inventory");
            if (event_category != 0)
            {
                Inventory::InventoryItemEventData folder_data(Inventory::IIT_Folder);
                folder_data.id = folder_id;
                folder_data.parentId = parentFolder->id;
                folder_data.inventoryType = GetInventoryTypeFromAssetType(asset_type);
                folder_data.assetType = asset_type;
                folder_data.name = cat_name;
                event_mgr->SendEvent(event_category, Inventory::Events::EVENT_INVENTORY_DESCENDENT, &folder_data);
            }
        }
    }
}

}
