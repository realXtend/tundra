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

namespace Asset
{
    QtHttpAssetProvider::QtHttpAssetProvider(Foundation::Framework *framework) :
        QObject(),
        framework_(framework),
        event_manager_(framework->GetEventManager().get()),
        name_("QtHttpAssetProvider"),
        network_manager_(new QNetworkAccessManager())
    {
		asset_timeout_ = framework_->GetDefaultConfig().DeclareSetting("AssetSystem", "http_timeout", 120.0);
        if (event_manager_)
            asset_event_category_ = event_manager_->QueryEventCategory("Asset");
        if (!asset_event_category_)
            AssetModule::LogWarning("QtHttpAssetProvider >> Could not get event category for Asset events");

        connect(network_manager_, SIGNAL(finished(QNetworkReply*)), SLOT(TranferCompleted(QNetworkReply*)));
    }

    QtHttpAssetProvider::~QtHttpAssetProvider()
    {
        SAFE_DELETE(network_manager_);
    }

    // Interface implementation

    void QtHttpAssetProvider::Update(f64 frametime)
    {

    }

    const std::string& QtHttpAssetProvider::Name()
    {
        return name_;
    }

    bool QtHttpAssetProvider::IsValidId(const std::string& asset_id)
    {
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
        if (!IsValidId(asset_id))
            return false;

        QString asset_id_qstring = QString::fromStdString(asset_id);
        if (assetid_to_transfer_map_.contains(asset_id_qstring))
        {
            assetid_to_transfer_map_[asset_id_qstring]->GetTranferInfo().AddTag(tag);
        }
        else
        {
            QUrl asset_url = CreateUrl(asset_id_qstring);
            asset_type_t asset_type_int = RexTypes::GetAssetTypeFromTypeName(asset_type);
            if (asset_type_int < 0 || !asset_url.isValid())
                return false;
            if (!asset_url.path().endsWith("/data"))
                asset_url.setPath(asset_url.path() + "/data");

            QtHttpAssetTransfer *transfer = new QtHttpAssetTransfer(asset_url, asset_id_qstring, asset_type_int, tag);
            transfer->setOriginatingObject(transfer);

            if (assetid_to_transfer_map_.count() <= 20)
            {
                assetid_to_transfer_map_[asset_id_qstring] = transfer;
                network_manager_->get(*transfer);

                //QStringList debug_parts = transfer->GetTranferInfo().id.split("/");
                //qDebug() << "     <HTTP-GET-DATA> " << debug_parts.at(debug_parts.length()-2);
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
        QtHttpAssetTransfer *transfer = dynamic_cast<QtHttpAssetTransfer*>(reply->request().originatingObject());

        /**** THIS IS A /data REQUEST REPLY AND IT FAILED****/
        if (reply->error() != QNetworkReply::NoError && transfer)
        {
            // Send asset canceled events
            HttpAssetTransferInfo error_transfer_data = transfer->GetTranferInfo();
            Events::AssetCanceled cancel_data(error_transfer_data.id.toStdString(), RexTypes::GetAssetTypeString(error_transfer_data.type));
            event_manager_->SendEvent(asset_event_category_, Events::ASSET_CANCELED, &cancel_data);

            // Clean up
            RemoveFinishedTransfers(error_transfer_data.id, reply->url());
            StartTransferFromQueue();

            //QStringList debug_parts = error_transfer_data.id.split("/");
            //qDebug() << "    <ASSET-CANCELED> " << debug_parts.at(debug_parts.length()-2);
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

            // Get metadata
            QUrl metadata_url = tranfer_info.url;
            QString url_path = tranfer_info.url.path();
            int clip_count;
            if (url_path.endsWith("/data"))
                clip_count = 5;
            else if (url_path.endsWith("/data/"))
                clip_count = 6;
            else 
                return;
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
            network_manager_->get(*metada_request);

            //QStringList debug_parts = tranfer_info.id.split("/");
            //qDebug() << " <HTTP-GET-METADATA> " << debug_parts.at(debug_parts.length()-2);
        }
        /**** THIS IS A /metadata REQUEST REPLY ****/
        else if (metadata_to_assetptr_.contains(reply->url()))
        {
            // Pull out transfer data and asset pointer assosiated with this reply url
            QUrl metadata_transfer_url = reply->url();
            HttpAssetTransferInfo transfer_data = metadata_to_assetptr_[metadata_transfer_url].first;
            Foundation::AssetPtr ready_asset_ptr = metadata_to_assetptr_[metadata_transfer_url].second;
            if (!ready_asset_ptr)
                return;

            // Fill metadata
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

            //QStringList debug_parts = transfer_data.id.split("/");
            //qDebug() << "       <ASSET-READY> " << debug_parts.at(debug_parts.length()-2);
        }
    }
    
    bool QtHttpAssetProvider::CheckRequestQueue(QString assed_id)
    {
        foreach (QtHttpAssetTransfer *transfer, pending_request_queue_)
            if (transfer->GetTranferInfo().id == assed_id)
                return true;
        return false;
    }

    void QtHttpAssetProvider::RemoveFinishedTransfers(QString asset_transfer_key, QUrl metadata_transfer_key)
    {
        QtHttpAssetTransfer *remove_transfer = assetid_to_transfer_map_[asset_transfer_key];
        assetid_to_transfer_map_.remove(asset_transfer_key);
        metadata_to_assetptr_.remove(metadata_transfer_key);
        SAFE_DELETE(remove_transfer);
    }

    void QtHttpAssetProvider::StartTransferFromQueue()
    {
        if (assetid_to_transfer_map_.count() <= 20 && pending_request_queue_.count() > 0)
        {
            QtHttpAssetTransfer *new_transfer = pending_request_queue_.takeAt(0);
            assetid_to_transfer_map_[new_transfer->GetTranferInfo().id] = new_transfer;
            network_manager_->get(*new_transfer);

            //QStringList debug_parts = new_transfer->GetTranferInfo().id.split("/");
            //qDebug() << "     <HTTP-GET-DATA> " << debug_parts.at(debug_parts.length()-2) << " FROM QUEUE";
        }
    }
}