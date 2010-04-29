// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_QtHttpAssetProvider_h
#define incl_Asset_QtHttpAssetProvider_h

#include "Foundation.h"
#include "AssetProviderInterface.h"
#include "QtHttpAssetTransfer.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QMap>
#include <QPair>
#include <QUrl>

namespace Asset
{
    class QtHttpAssetProvider : public QObject, public Foundation::AssetProviderInterface
    {

    Q_OBJECT

    public:
        QtHttpAssetProvider(Foundation::Framework *framework);
        virtual ~QtHttpAssetProvider();

        void SetGetTextureCap(std::string url);

        //! Interface implementation
        void Update(f64 frametime);

        const std::string& Name();
        bool IsValidId(const std::string& asset_id, const std::string& asset_type);
        
        bool RequestAsset(const std::string& asset_id, const std::string& asset_type, request_tag_t tag);
        bool InProgress(const std::string& asset_id);
        bool QueryAssetStatus(const std::string& asset_id, uint& size, uint& received, uint& received_continuous);

        Foundation::AssetPtr GetIncompleteAsset(const std::string& asset_id, const std::string& asset_type, uint received);
        Foundation::AssetTransferInfoVector GetTransferInfo();
    
    private slots:
        QUrl CreateUrl(QString assed_id);
        void TranferCompleted(QNetworkReply *reply);
        bool CheckRequestQueue(QString assed_id);
        void RemoveFinishedTransfers(QString asset_transfer_key, QUrl metadata_transfer_key);
        void StartTransferFromQueue();

    private:
        Foundation::Framework *framework_;
        Foundation::EventManager *event_manager_;
        const std::string name_;
        QNetworkAccessManager *network_manager_;
        
        event_category_id_t asset_event_category_;
        f64 asset_timeout_;

        QMap<QString, QtHttpAssetTransfer *> assetid_to_transfer_map_;
        QMap<QUrl, QPair<HttpAssetTransferInfo, Foundation::AssetPtr> > metadata_to_assetptr_;
        QList<QtHttpAssetTransfer *> pending_request_queue_;

        bool filling_stack_;
        bool fake_metadata_fetch_;
        QUrl fake_metadata_url_;

        QUrl get_texture_cap_;

    };
}

#endif