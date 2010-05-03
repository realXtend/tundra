// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "QtHttpAssetProvider.h"
#include "ConfigurationManager.h"
#include "EventManager.h"
#include "AssetModule.h"

#include "RexAsset.h"
#include "AssetMetadataInterface.h"
#include "AssetServiceInterface.h"
#include "AssetEvents.h"

#include <QUuid>
#include <QNetworkReply>
#include <QByteArray>

#include <QDebug>
#include <QStringList>

#define MAX_HTTP_CONNECTIONS 10000 // Basically means no limit to parallel connections

namespace Asset
{
    QtHttpAssetProvider::QtHttpAssetProvider(Foundation::Framework *framework) :
        QObject(),
        framework_(framework),
        event_manager_(framework->GetEventManager().get()),
        name_("QtHttpAssetProvider"),
        network_manager_(new QNetworkAccessManager()),
        filling_stack_(false),
        get_texture_cap_(QUrl())
    {
		asset_timeout_ = framework_->GetDefaultConfig().DeclareSetting("AssetSystem", "http_timeout", 120.0);
        if (event_manager_)
            asset_event_category_ = event_manager_->QueryEventCategory("Asset");
        if (!asset_event_category_)
            AssetModule::LogWarning("QtHttpAssetProvider >> Could not get event category for Asset events");

        connect(network_manager_, SIGNAL(finished(QNetworkReply*)), SLOT(TranferCompleted(QNetworkReply*)));

        AssetModule::LogWarning(QString("QtHttpAssetProvider >> Initialized with max %1 parallel HTTP connections").arg(QString::number(MAX_HTTP_CONNECTIONS)).toStdString());
    }

    QtHttpAssetProvider::~QtHttpAssetProvider()
    {
        SAFE_DELETE(network_manager_);
    }

    void QtHttpAssetProvider::SetGetTextureCap(std::string url)
    {
        get_texture_cap_ = QUrl(QString::fromStdString(url));
        if (get_texture_cap_.isValid())
            AssetModule::LogInfo("Server supports HTTP assets: using HTTP to fetch textures and meshes.");
    }

    // Interface implementation

    void QtHttpAssetProvider::Update(f64 frametime)
    {
        StartTransferFromQueue();
    }

    const std::string& QtHttpAssetProvider::Name()
    {
        return name_;
    }

    bool QtHttpAssetProvider::IsValidId(const std::string& asset_id, const std::string& asset_type)
    {
        // Textures over http with the GetTexture cap
        if (IsAcceptableAssetType(asset_type))
            if (RexUUID::IsValid(asset_id) && get_texture_cap_.isValid())
                return true;

        QString id(asset_id.c_str());
        if (!id.startsWith("http://"))
            return false;

        QUrl asset_url(id);
        if (asset_url.isValid())
            return true;
        else
            return false;
    }
    
    bool QtHttpAssetProvider::RequestAsset(const std::string& asset_id, const std::string& asset_type, request_tag_t tag)
    {
        if (!IsValidId(asset_id, asset_type))
            return false;

        QString asset_id_qstring = QString::fromStdString(asset_id);
        if (assetid_to_transfer_map_.contains(asset_id_qstring))
        {
            assetid_to_transfer_map_[asset_id_qstring]->GetTranferInfo().AddTag(tag);
        }
        else
        {
            asset_type_t asset_type_int = RexTypes::GetAssetTypeFromTypeName(asset_type);
            QtHttpAssetTransfer *transfer = 0;
    
            if (IsAcceptableAssetType(asset_type) && RexUUID::IsValid(asset_id) && get_texture_cap_.isValid())
            {
                // Http texture/meshes via cap url
                QString texture_url_string = get_texture_cap_.toString() + "?texture_id=" + asset_id_qstring;
                QUrl texture_url(texture_url_string);
                transfer = new QtHttpAssetTransfer(texture_url, asset_id_qstring, asset_type_int, tag);
            }
            else
            {
                // Normal http get
                QUrl asset_url = CreateUrl(asset_id_qstring);
                transfer = new QtHttpAssetTransfer(asset_url, asset_id_qstring, asset_type_int, tag);
            }

            if (!transfer)
                return false;

            transfer->setOriginatingObject(transfer);
            if (assetid_to_transfer_map_.count() <= MAX_HTTP_CONNECTIONS)
            {
                assetid_to_transfer_map_[asset_id_qstring] = transfer;
                network_manager_->get(*transfer);
                AssetModule::LogDebug("New HTTP asset request: " + asset_id + " type: " + asset_type);
            }
            else
                pending_request_queue_.append(transfer);
        }
        return true;
    }

    bool QtHttpAssetProvider::InProgress(const std::string& asset_id)
    {
        QString qt_asset_id = QString::fromStdString(asset_id);
        if (assetid_to_transfer_map_.contains(qt_asset_id) || CheckRequestQueue(qt_asset_id))
            return true;
        else
            return false;
    }

    bool QtHttpAssetProvider::QueryAssetStatus(const std::string& asset_id, uint& size, uint& received, uint& received_continuous)
    {
        if (InProgress(asset_id))
        {
            // We dont know, QNetworkAccessManager handles these
            size = 0;
            received = 0;
            received_continuous = 0;
            return true;
        }
        else
            return false;
    }

    Foundation::AssetPtr QtHttpAssetProvider::GetIncompleteAsset(const std::string& asset_id, const std::string& asset_type, uint received)       
    {
        return Foundation::AssetPtr();
    }

    Foundation::AssetTransferInfoVector QtHttpAssetProvider::GetTransferInfo()
    {
        Foundation::AssetTransferInfoVector info_vector;
        foreach (QtHttpAssetTransfer *transfer, assetid_to_transfer_map_.values())
        {
            HttpAssetTransferInfo iter_info = transfer->GetTranferInfo();
            // What we know
            Foundation::AssetTransferInfo info;
            info.id_ = iter_info.id.toStdString();
            info.type_ = RexTypes::GetAssetTypeString(iter_info.type);
            info.provider_ = Name();
            // The following we dont know, QNetworkAccessManager handles these
            info.size_ = 0;
            info.received_ = 0;
            info.received_continuous_ = 0;
            info_vector.push_back(info);
        }
        return info_vector;
    }

    // Private

    QUrl QtHttpAssetProvider::CreateUrl(QString assed_id)
    {
        if (!assed_id.startsWith("http://") && !assed_id.startsWith("https://"))
            assed_id = "http://" + assed_id;
        return QUrl(assed_id);
    }

    void QtHttpAssetProvider::TranferCompleted(QNetworkReply *reply)
    {
        fake_metadata_fetch_ = false;
        QtHttpAssetTransfer *transfer = dynamic_cast<QtHttpAssetTransfer*>(reply->request().originatingObject());

        /**** THIS IS A DATA REQUEST REPLY AND IT FAILED ****/
        if (reply->error() != QNetworkReply::NoError && transfer)
        {
            // Send asset canceled events
            HttpAssetTransferInfo error_transfer_data = transfer->GetTranferInfo();
            Events::AssetCanceled *data = new Events::AssetCanceled(error_transfer_data.id.toStdString(), RexTypes::GetAssetTypeString(error_transfer_data.type));
            Foundation::EventDataPtr data_ptr(data);
            event_manager_->SendDelayedEvent(asset_event_category_, Events::ASSET_CANCELED, data_ptr, 0);

            // Clean up
            RemoveFinishedTransfers(error_transfer_data.id, reply->url());
            StartTransferFromQueue();

            AssetModule::LogDebug("HTTP asset " + error_transfer_data.id.toStdString() + " canceled");
            reply->deleteLater();
            return;
        }

        /**** THIS IS A /data REQUEST REPLY ****/
        if (transfer)
        {
            // Create asset pointer
            HttpAssetTransferInfo tranfer_info = transfer->GetTranferInfo();
            std::string id = tranfer_info.id.toStdString();
            std::string type = RexTypes::GetTypeNameFromAssetType(tranfer_info.type);
            Foundation::AssetPtr asset_ptr = Foundation::AssetPtr(new RexAsset(id, type));

            // Fill asset data with reply data
            RexAsset::AssetDataVector& data_vector = checked_static_cast<RexAsset*>(asset_ptr.get())->GetDataInternal();
            QByteArray data_array = reply->readAll();
            for (int index = 0; index < data_array.count(); ++index)
                data_vector.push_back(data_array.at(index));

            // Get metadata if available
            QString url_path = tranfer_info.url.path();
            if (url_path.endsWith("/data") || url_path.endsWith("/data/"))
            {
                // Generate metada url
                int clip_count;
                if (url_path.endsWith("/data"))
                    clip_count = 5;
                else if (url_path.endsWith("/data/"))
                    clip_count = 6;
                else
                {
                    reply->deleteLater();
                    return;
                }

                QUrl metadata_url = tranfer_info.url;
                url_path = url_path.left(url_path.count()-clip_count);
                url_path = url_path + "/metadata";
                metadata_url.setPath(url_path);
                tranfer_info.url = metadata_url;

                QNetworkRequest *metada_request = new QNetworkRequest(metadata_url);

                // Store tranfer data and asset data pointer internally
                QPair<HttpAssetTransferInfo, Foundation::AssetPtr> data_pair;
                data_pair.first = tranfer_info;
                data_pair.second = asset_ptr;
                metadata_to_assetptr_[metada_request->url()] = data_pair;
                
                // Send metadata network request
                //network_manager_->get(*metada_request);
                
                // HACK to avoid metadata fetch for now
                fake_metadata_url_ = metada_request->url();
                fake_metadata_fetch_ = true;
            }
            // Asset data feched, lets store
            else
            {
                // Store asset
                boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = framework_->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
                if (asset_service)
                    asset_service->StoreAsset(asset_ptr);

                // Send asset ready events
                foreach (request_tag_t tag, tranfer_info.tags)
                {
                    Events::AssetReady event_data(asset_ptr.get()->GetId(), asset_ptr.get()->GetType(), asset_ptr, tag);
                    event_manager_->SendEvent(asset_event_category_, Events::ASSET_READY, &event_data);
                }

                RemoveFinishedTransfers(tranfer_info.id, QUrl());
                StartTransferFromQueue();
                AssetModule::LogDebug("HTTP asset " + tranfer_info.id.toStdString() + " completed");
            }
        }

        // Complete /data and /metadata sequence, fake is here as long as we dont have xml parser for metadata
        // or actually we dont use metadata in naali so thats the main reason its not fetched
        if (fake_metadata_fetch_)
        {
            /**** THIS IS A /metadata REQUEST REPLY ****/
            if (metadata_to_assetptr_.contains(fake_metadata_url_))
            {
                // Pull out transfer data and asset pointer assosiated with this reply url
                QUrl metadata_transfer_url = fake_metadata_url_;
                HttpAssetTransferInfo transfer_data = metadata_to_assetptr_[metadata_transfer_url].first;
                Foundation::AssetPtr ready_asset_ptr = metadata_to_assetptr_[metadata_transfer_url].second;
                if (!ready_asset_ptr)
                {
                    reply->deleteLater();
                    return;
                }

                // Fill metadata
                /*
                const QByteArray &inbound_metadata = reply->readAll();
                QString decoded_metadata = QString::fromUtf8(inbound_metadata.data());
                #if defined(__GNUC__)
                RexAssetMetadata *m = dynamic_cast<RexAssetMetadata*>(ready_asset_ptr.get()->GetMetadata());
                #else	   
                Foundation::AssetMetadataInterface *metadata = ready_asset_ptr.get()->GetMetadata();
                RexAssetMetadata *m = static_cast<RexAssetMetadata*>(metadata);
                #endif
                std::string std_md(decoded_metadata.toStdString());
                
                m->DesesrializeFromJSON(std_md); // TODO: implement a xml based metadata parser.
                */

                // Store asset
                boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = framework_->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
                if (asset_service)
                    asset_service->StoreAsset(ready_asset_ptr);

                // Send asset ready events
                foreach (request_tag_t tag, transfer_data.tags)
                {
                    Events::AssetReady event_data(ready_asset_ptr.get()->GetId(), ready_asset_ptr.get()->GetType(), ready_asset_ptr, tag);
                    event_manager_->SendEvent(asset_event_category_, Events::ASSET_READY, &event_data);
                }

                RemoveFinishedTransfers(transfer_data.id, metadata_transfer_url);
                StartTransferFromQueue();
                AssetModule::LogDebug("HTTP asset " + transfer_data.id.toStdString() + " completed with metadata");
            }
        }
        reply->deleteLater();
    }

    void QtHttpAssetProvider::RemoveFinishedTransfers(QString asset_transfer_key, QUrl metadata_transfer_key)
    {
        QtHttpAssetTransfer *remove_transfer = assetid_to_transfer_map_[asset_transfer_key];
        assetid_to_transfer_map_.remove(asset_transfer_key);
        if (metadata_transfer_key.isValid())
            metadata_to_assetptr_.remove(metadata_transfer_key);
        SAFE_DELETE(remove_transfer);
    }

    void QtHttpAssetProvider::ClearAllTransfers()
    {
        foreach (QtHttpAssetTransfer *transfer, assetid_to_transfer_map_.values())
            SAFE_DELETE(transfer);
        assetid_to_transfer_map_.clear();

        foreach (QtHttpAssetTransfer *transfer, pending_request_queue_)
            SAFE_DELETE(transfer);
        pending_request_queue_.clear();
    }

    void QtHttpAssetProvider::StartTransferFromQueue()
    {
        if (pending_request_queue_.count() == 0)
            return;

        // If the whole map is empty then go and start new MAX_HTTP_CONNECTIONS amount of http gets
        if (assetid_to_transfer_map_.count() == 0)
            filling_stack_ = true;

        if (!filling_stack_)
            return;

        if (assetid_to_transfer_map_.count() <= MAX_HTTP_CONNECTIONS && pending_request_queue_.count() > 0)
        {
            if (assetid_to_transfer_map_.count() == MAX_HTTP_CONNECTIONS || pending_request_queue_.count() == 0)
                filling_stack_ = false; 
            else
            {
                QtHttpAssetTransfer *new_transfer = pending_request_queue_.takeAt(0);
                assetid_to_transfer_map_[new_transfer->GetTranferInfo().id] = new_transfer;
                network_manager_->get(*new_transfer);

                AssetModule::LogDebug("New HTTP asset request from queue: " + new_transfer->GetTranferInfo().id.toStdString() + " type: " + RexTypes::GetAssetTypeString(new_transfer->GetTranferInfo().type));
                StartTransferFromQueue(); // Recursivly fill the transfer map
            }
        }
    }

    bool QtHttpAssetProvider::CheckRequestQueue(QString assed_id)
    {
        foreach (QtHttpAssetTransfer *transfer, pending_request_queue_)
            if (transfer->GetTranferInfo().id == assed_id)
                return true;
        return false;
    }

    bool QtHttpAssetProvider::IsAcceptableAssetType(const std::string& asset_type)
    {
        using namespace RexTypes;
        
        bool accepted = false;
        switch (GetAssetTypeFromTypeName(asset_type))
        {
            case RexAT_Texture:
            case RexAT_Mesh:
                accepted = true;
                break;
            default:
                accepted = false;
                break;
        }
        return accepted;
    }
}