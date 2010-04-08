// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "QtHttpAssetTransfer.h"

namespace Asset
{
    // ==========================================================
    // HttpAssetTransferInfo

    HttpAssetTransferInfo::HttpAssetTransferInfo()
    {
    }

    HttpAssetTransferInfo::HttpAssetTransferInfo(const HttpAssetTransferInfo &info) :
        url(info.url),
        id(info.id),
        type(info.type),
        tags(info.tags)
    {
    }

    HttpAssetTransferInfo::HttpAssetTransferInfo(QUrl asset_url, QString asset_id, asset_type_t asset_type) :
        url(asset_url),
        id(asset_id),
        type(asset_type)
    {
    }

    void HttpAssetTransferInfo::AddTag(request_tag_t tag)
    {
        tags.append(tag);
    }

    // ==========================================================
    // QtHttpAssetTransfer

    QtHttpAssetTransfer::QtHttpAssetTransfer(QUrl asset_url, QString asset_id, asset_type_t asset_type, request_tag_t tag) :
        QObject(0),
        QNetworkRequest(asset_url),
        transfer_info_(asset_url, asset_id, asset_type)
    {
        transfer_info_.AddTag(tag);
    }
}