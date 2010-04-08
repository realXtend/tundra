// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_QtHttpAssetTransfer_h
#define incl_Asset_QtHttpAssetTransfer_h

#include "RexTypes.h"

#include <QObject>
#include <QNetworkRequest>
#include <QUrl>
#include <QString>

namespace Asset
{
    struct HttpAssetTransferInfo
    {
        HttpAssetTransferInfo();
        HttpAssetTransferInfo(const HttpAssetTransferInfo &info);
        HttpAssetTransferInfo(QUrl asset_url, QString asset_id, asset_type_t asset_type);
        void AddTag(request_tag_t tag);

        QUrl url;
        QString id;
        asset_type_t type;
        QList<request_tag_t> tags;
    };

    class QtHttpAssetTransfer : public QObject, public QNetworkRequest
    {

    Q_OBJECT

    public:
        QtHttpAssetTransfer(QUrl asset_url, QString asset_id, asset_type_t asset_type, request_tag_t tag);
        HttpAssetTransferInfo GetTranferInfo() { return transfer_info_; }

    private:
        HttpAssetTransferInfo transfer_info_;

    };
}

#endif