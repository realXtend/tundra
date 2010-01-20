// For conditions of distribution and use, see copyright notice in license.txt

/// @file AssetUploader.cpp
/// @brief Asset HTTP uploading functionality for OS & reX servers.

#include "StableHeaders.h"
#include "AssetUploader.h"
#include "InventoryModule.h"
#include "OpenSimInventoryDataModel.h"
#include "HttpRequest.h"
#include "LLSDUtilities.h"
#include "RexUUID.h"
#include "Inventory/InventoryEvents.h"
#include "InventoryFolder.h"
#include "J2kEncoder.h"

#include <QStringList>
#include <QVector>
#include <QTime>
#include <QDir>

namespace Inventory
{

AssetUploader::AssetUploader(Foundation::Framework* framework, OpenSimInventoryDataModel *data_model) :
    framework_(framework), dataModel_(data_model), uploadCapability_("")
{
}

AssetUploader::~AssetUploader()
{
}

void AssetUploader::SetWorldStream(ProtocolUtilities::WorldStreamPtr world_stream)
{
    currentWorldStream_ = world_stream;
}

void AssetUploader::UploadFiles(StringList &filenames, StringList &item_names)
{
    CreateRexInventoryFolders();

    if (!HasUploadCapability())
    {
        std::string upload_url = currentWorldStream_->GetCapability("NewFileAgentInventory");
        if (upload_url == "")
        {
            InventoryModule::LogError("Could not get upload capability for asset uploader. Uploading not possible");
            return;
        }

        SetUploadCapability(upload_url);
    }

    Thread thread(boost::bind(&AssetUploader::ThreadedUploadFiles, this, filenames, item_names));
}

void AssetUploader::UploadFiles(QStringList &filenames, QStringList &item_names)
{
    StringList files, names;
    for(QStringList::iterator it = filenames.begin(); it != filenames.end(); ++it)
        files.push_back(it->toStdString());

    for(QStringList::iterator it = item_names.begin(); it != item_names.end(); ++it)
        names.push_back(it->toStdString());

    UploadFiles(files, names);
}

void AssetUploader::UploadBuffers(QStringList &filenames, QVector<QVector<uchar> > &buffers)
{
    CreateRexInventoryFolders();

    if (!HasUploadCapability())
    {
        std::string upload_url = currentWorldStream_->GetCapability("NewFileAgentInventory");
        if (upload_url == "")
        {
            InventoryModule::LogError("Could not get upload capability for asset uploader. Uploading not possible");
            return;
        }

        SetUploadCapability(upload_url);
    }

    Thread thread(boost::bind(&AssetUploader::ThreadedUploadBuffers, this, filenames, buffers));
}

/*
bool AssetUploader::UploadFile(
    const RexTypes::asset_type_t &asset_type,
    const std::string &filename,
    const std::string &name,
    const std::string &description,
    const RexUUID &folder_id)
{
    using namespace OpenSimProtocol;

    if (!HasUploadCapability())
    {
        std::string upload_url = currentWorldStream_ ->GetCapability("NewFileAgentInventory");
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
        currentWorldStream_ ->SendCreateInventoryFolderPacket(parentFolder->id, folder_id, asset_type, cat_name);
    }
    else
        folder_id = folder->id;

    Thread thread(boost::bind(&AssetUploader::ThreadedUploadFile, this, asset_type, filename, name, description, folder_id));
    return true;
}
*/

bool AssetUploader::UploadBuffer(
    const RexTypes::asset_type_t &asset_type,
    const std::string& filename,
    const std::string& name,
    const std::string& description,
    const RexUUID& folder_id,
    const QVector<uchar>& buffer)
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

    std::vector<u8> response;
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

    HttpUtilities::HttpRequest request2;
    request2.SetUrl(upload_url);
    request2.SetMethod(HttpUtilities::HttpRequest::Post);

    // If the file is texture, use Ogre image and J2k encoding.
    if (asset_type == RexTypes::RexAT_Texture)
    {
        Ogre::Image image;

        try
        {
            Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)&buffer[0], buffer.size(), false));
            image.load(stream);
        }
        catch (Ogre::Exception &e)
        {
            InventoryModule::LogError("Error loading image: " + std::string(e.what()));
            return false;
        }

        std::vector<u8> encoded_buffer;
        
        bool success = J2k::J2kEncode(image, encoded_buffer, false);
        if (!success)
        {
            InventoryModule::LogError("Could not J2k encode the image file.");
            return false;
        }
        
        request2.SetRequestData("application/octet-stream", encoded_buffer);
    }
    else
    {
        // Other assets can be uploaded as raw data.
        request2.SetRequestData("application/octet-stream", buffer.toStdVector());
    }

    response.clear();
    response_str.clear();

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
    // Note: sent as delayed, to be thread-safe
    
    Foundation::EventManagerPtr event_mgr = framework_->GetEventManager();
    event_category_id_t event_category = event_mgr->QueryEventCategory("Inventory");
    if (event_category != 0)
    {
        boost::shared_ptr<Inventory::InventoryItemEventData> asset_data(new Inventory::InventoryItemEventData(Inventory::IIT_Asset));
        
        asset_data->id = RexUUID(inventory_id);
        asset_data->parentId = folder_id;
        asset_data->assetId = RexUUID(asset_id);
        asset_data->assetType = asset_type;
        asset_data->inventoryType = RexTypes::GetInventoryTypeFromAssetType(asset_type);
        asset_data->name = name;
        asset_data->description = description;
        asset_data->fileName = filename;

        event_mgr->SendDelayedEvent<Inventory::InventoryItemEventData>(event_category, Inventory::Events::EVENT_INVENTORY_DESCENDENT, asset_data);
    }
    
    InventoryModule::LogInfo("Upload succesfull. Asset id: " + asset_id + ", inventory id: " + inventory_id + ".");
    return true;
}

bool AssetUploader::UploadFile(
    const RexTypes::asset_type_t &asset_type,
    const std::string &filename,
    const std::string &name,
    const std::string &description,
    const RexUUID &folder_id)
{
    if (!HasUploadCapability())
    {
        std::string upload_url = currentWorldStream_->GetCapability("NewFileAgentInventory");
        if (upload_url == "")
        {
            InventoryModule::LogError("Could not get upload capability for asset uploader. Uploading not possible");
            return false;
        }

        SetUploadCapability(upload_url);
    }

    // Open the file.
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file.is_open())
    {
        InventoryModule::LogError("Could not open file the file: " + filename + ".");
        return false;
    }

    QVector<uchar> buffer;

    std::filebuf *pbuf = file.rdbuf();
    size_t size = pbuf->pubseekoff(0, std::ios::end, std::ios::in);
    buffer.resize(size);
    pbuf->pubseekpos(0, std::ios::in);
    pbuf->sgetn((char *)&buffer[0], size);
    file.close();

    return UploadBuffer(asset_type, filename, name, description, folder_id, buffer);
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

void AssetUploader::ThreadedUploadFiles(StringList filenames, StringList item_names)
{
    // For notifications
    QTime timer;

    // Iterate trought every asset.
    int asset_count = 0;
    StringList::iterator name_it = item_names.begin();
    for(StringList::iterator it = filenames.begin(); it != filenames.end(); ++it)
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
        std::string name;
        if (name_it != item_names.end())
        {
            name = *name_it;
            ++name_it;
        }
        else
            name = CreateNameFromFilename(filename);

        std::string description = "(No Description)";

        RexUUID folder_id(dataModel_->GetFirstChildFolderByName(cat_name.c_str())->GetID().toStdString());
        if (folder_id.IsNull())
        {
            InventoryModule::LogError("Inventory folder for this type of file doesn't exists. File can't be uploaded.");
            continue;
        }
 
        // Start notification
        QString real_filename = QString::fromStdString(filename);
        real_filename = real_filename.midRef(real_filename.lastIndexOf(QDir::separator())+1).toString();
        emit NewNotification(QString("Uploading %1").arg(QString::fromStdString(filename)));
        timer.start();

        if (UploadFile(asset_type, filename, name, description, folder_id))
        {
            // Send succesfull notification
            qreal elapsed(timer.elapsed());
            emit NewNotification(QString("%1 uploaded succesfully to %2 in %3 seconds").arg(real_filename, QString::fromStdString(cat_name), QString::number(elapsed/1000, 102, 2)));
            ++asset_count;
        }
        else
        {
            // Send fail notification
            emit NewNotification(QString("Upload failed for %1").arg(real_filename));
        }
    }

    InventoryModule::LogInfo("Multiupload:" + ToString(asset_count) + " assets succesfully uploaded.");
}

void AssetUploader::ThreadedUploadBuffers(QStringList filenames, QVector<QVector<uchar> > buffers)
{
    if (filenames.size() != buffers.size())
    {
        InventoryModule::LogError("Not as many data buffers as filenames!");
        return;
    }

    // Iterate trought every asset.
    int asset_count = 0;
    QVector<QVector<uchar> >::iterator it2 = buffers.begin();

    QStringListIterator it(filenames);
    while(it.hasNext())
    {
        std::string filename = it.next().toStdString();
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

        RexUUID folder_id(dataModel_->GetFirstChildFolderByName(cat_name.c_str())->GetID().toStdString());
        if (folder_id.IsNull())
        {
            InventoryModule::LogError("Inventory folder for this type of file doesn't exists. File can't be uploaded.");
            continue;
        }

        //if (UploadBuffer(asset_type, filename, name, description, folder_id, (*it2)))
        if (UploadBuffer(asset_type, filename, name, description, folder_id, *it2))
            ++asset_count;

        ++it2;
    }

    InventoryModule::LogInfo("Multiupload:" + ToString(asset_count) + " assets succesfully uploaded.");
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
    if (!dataModel_)
    {
        InventoryModule::LogError("Inventory doens't exist yet! Can't create folder to it.");
        return;
    }

    using namespace RexTypes;

    const char *asset_types[] = { "Texture", "Mesh", "Skeleton", "MaterialScript", "ParticleScript", "FlashAnimation", "GenericAvatarXml" };
    asset_type_t asset_type;
    for(int i = 0; i < NUMELEMS(asset_types); ++i)
    {
        asset_type = GetAssetTypeFromTypeName(asset_types[i]);
        std::string cat_name = GetCategoryNameForAssetType(asset_type);

        // Check out if this inventory category exists.
        AbstractInventoryItem *existing = dataModel_->GetFirstChildFolderByName(STD_TO_QSTR(cat_name));
        if (!existing)
        {
            // I doesn't. Create new inventory folder.
            InventoryFolder *myInventory = dataModel_->GetMyInventoryFolder();
            AbstractInventoryItem *newFolder = dataModel_->GetOrCreateNewFolder(
                QString(RexUUID::CreateRandom().ToString().c_str()), *myInventory, STD_TO_QSTR(cat_name));
        }
    }
}

}
