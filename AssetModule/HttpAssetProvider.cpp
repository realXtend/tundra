// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "HttpAssetProvider.h"
#include "HttpAssetTransfer.h"
#include "LoggingFunctions.h"
#include "IAssetUploadTransfer.h"

#include "AssetAPI.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

DEFINE_POCO_LOGGING_FUNCTIONS("HttpAssetProvider")

HttpAssetProvider::HttpAssetProvider(Foundation::Framework *framework_)
:framework(framework_)
{
    networkAccessManager = new QNetworkAccessManager(this);
    connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), SLOT(OnHttpTransferFinished(QNetworkReply*)));
}

HttpAssetProvider::~HttpAssetProvider()
{
}

QString HttpAssetProvider::Name()
{
    return "HttpAssetProvider";
}

bool HttpAssetProvider::IsValidRef(QString assetRef, QString)
{
    return assetRef.trimmed().startsWith("http://");
}
        
AssetTransferPtr HttpAssetProvider::RequestAsset(QString assetRef, QString assetType)
{
    assetRef = assetRef.trimmed();
    if (!IsValidRef(assetRef))
    {
        LogError("HttpAssetProvider::RequestAsset: Cannot get asset from invalid URL \"" + assetRef.toStdString() + "\"!");
        return AssetTransferPtr();
    }
    QNetworkRequest request;
    request.setUrl(QUrl(assetRef));
    request.setRawHeader("User-Agent", "realXtend Naali");

    QNetworkReply *reply = networkAccessManager->get(request);

    HttpAssetTransferPtr transfer = HttpAssetTransferPtr(new HttpAssetTransfer);
    transfer->source.ref = assetRef;
    transfer->assetType = assetType;
    transfers[reply] = transfer;
    return transfer;
}

AssetUploadTransferPtr HttpAssetProvider::UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName)
{
    QString dstUrl = destination->GetFullAssetURL(assetName);
    QNetworkRequest request;
    request.setUrl(QUrl(dstUrl));
    request.setRawHeader("User-Agent", "realXtend Naali");

    QByteArray dataArray((const char*)data, numBytes);
    QNetworkReply *reply = networkAccessManager->put(request, dataArray);

    AssetUploadTransferPtr transfer = AssetUploadTransferPtr(new IAssetUploadTransfer());
    transfer->destinationStorage = destination;
    transfer->destinationProvider = shared_from_this();
    transfer->destinationName = assetName;

    uploadTransfers[reply] = transfer;

    return transfer;
}

void HttpAssetProvider::OnHttpTransferFinished(QNetworkReply *reply)
{
    switch(reply->operation())
    {
    case QNetworkAccessManager::GetOperation:
    {
        QByteArray data = reply->readAll();
        TransferMap::iterator iter = transfers.find(reply);
        if (iter == transfers.end())
        {
            LogError("Received a finish signal of an unknown Http transfer!");
            return;
        }
        HttpAssetTransferPtr transfer = iter->second;
        assert(transfer.get());

        transfer->rawAssetData.clear();

        if (reply->error() == QNetworkReply::NoError)
        {
            transfer->rawAssetData.insert(transfer->rawAssetData.end(), data.data(), data.data() + data.size());
            framework->Asset()->AssetTransferCompleted(transfer.get());
        }
        else
        {
            LogError("Http get for address \"" + reply->url().toString().toStdString() + "\" returned an error: \"" + reply->errorString().toStdString() + "\"");
            framework->Asset()->AssetTransferFailed(transfer.get());
        }
        transfers.erase(iter);
        break;
    }
    case QNetworkAccessManager::PutOperation:
    case QNetworkAccessManager::PostOperation:
    {
        UploadTransferMap::iterator iter = uploadTransfers.find(reply);
        if (iter == uploadTransfers.end())
        {
            LogError("Received a finish signal of an unknown Http upload transfer!");
            return;
        }
        AssetUploadTransferPtr transfer = iter->second;

        if (reply->error() == QNetworkReply::NoError)
        {
            LogDebug("Http upload to address \"" + reply->url().toString().toStdString() + "\" returned successfully.");
            framework->Asset()->AssetUploadTransferCompleted(transfer.get());
        }
        else
        {
            LogError("Http upload to address \"" + reply->url().toString().toStdString() + "\" failed with an error: \"" + reply->errorString().toStdString() + "\"");
            ///\todo Call the following when implemented:
//            framework->Asset()->AssetUploadTransferFailed(transfer.get());
        }

        uploadTransfers.erase(iter);
        break;
    }
    default:
        LogInfo("Unknown operation for address \"" + reply->url().toString().toStdString() + "\" finished with result: \"" + reply->errorString().toStdString() + "\"");
        break;
    }
}
        
void HttpAssetProvider::AddStorageAddress(const std::string &address, const std::string &storageName)
{
    HttpAssetStoragePtr storage = HttpAssetStoragePtr(new HttpAssetStorage());
    storage->baseAddress = GuaranteeTrailingSlash(QString(address.c_str()).trimmed());
    storage->storageName = storageName.c_str();
    storage->provider = this->shared_from_this();

    storages.push_back(storage);
}

std::vector<AssetStoragePtr> HttpAssetProvider::GetStorages() const
{
    std::vector<AssetStoragePtr> s;
    for(size_t i = 0; i < storages.size(); ++i)
        s.push_back(storages[i]);
    return s;
}
