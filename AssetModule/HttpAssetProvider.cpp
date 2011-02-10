// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "HttpAssetProvider.h"
#include "HttpAssetTransfer.h"
#include "LoggingFunctions.h"
#include "IAssetUploadTransfer.h"

#include "AssetAPI.h"
#include "IAsset.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkDiskCache>

DEFINE_POCO_LOGGING_FUNCTIONS("HttpAssetProvider")

HttpAssetProvider::HttpAssetProvider(Foundation::Framework *framework_)
:framework(framework_)
{
    // Http access manager
    networkAccessManager = new QNetworkAccessManager(this);
    connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), SLOT(OnHttpTransferFinished(QNetworkReply*)));

    // Http disk cache
    QString diskCachePath = QString::fromStdString(framework->GetPlatform()->GetApplicationDataDirectory()) + "/assetcache";
    QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory(diskCachePath);
    qint64 cacheSize = diskCache->maximumCacheSize(); // default is 50mb
    diskCache->setMaximumCacheSize(cacheSize * 10 * 4); // go up to 2000mb
    networkAccessManager->setCache(diskCache);

    connect(framework_->Asset(), SIGNAL(DiskSourceAboutToBeRemoved(AssetPtr)), SLOT(ForgetAsset(AssetPtr)));
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
    assetRef = assetRef.trimmed();

    return assetRef.startsWith("http://") || assetRef.startsWith("https://");
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

void HttpAssetProvider::DeleteAssetFromStorage(QString assetRef)
{
    assetRef = assetRef.trimmed();
    if (!IsValidRef(assetRef))
    {
        LogError("HttpAssetProvider::DeleteAssetFromStorage: Cannot delete asset from invalid URL \"" + assetRef.toStdString() + "\"!");
        return;
    }
    QUrl assetUrl(assetRef);
    QNetworkRequest request;
    request.setUrl(QUrl(assetRef));
    request.setRawHeader("User-Agent", "realXtend Naali");

    networkAccessManager->deleteResource(request);
}

void HttpAssetProvider::ForgetAsset(AssetPtr asset)
{
    ForgetAsset(asset->Name());
}

void HttpAssetProvider::ForgetAsset(QString assetRef)
{
    QAbstractNetworkCache *internalCache = networkAccessManager->cache();
    if (!internalCache)
        return;
    QUrl url(assetRef);
    QString urlString = url.toString();
    if (url.isValid() && (urlString.startsWith("http://") || urlString.startsWith("https://"))) /// \todo better checks
        if (!internalCache->remove(url))
            LogError("Could not remove \"" + urlString.toStdString() + "\" from QNetworkDiskCache");
}

void HttpAssetProvider::OnHttpTransferFinished(QNetworkReply *reply)
{
    // QNetworkAccessManager requires us to delete the QNetworkReply, or it will leak.
    reply->deleteLater();

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
        assert(transfer);
        transfer->rawAssetData.clear();

        if (reply->error() == QNetworkReply::NoError)
        {
            if (!transfer->CachingAllowed())
                ForgetAsset(transfer->source.ref);
            transfer->SetCachingBehavior(false, ""); // Don't duplicate store to normal asset cache
            transfer->rawAssetData.insert(transfer->rawAssetData.end(), data.data(), data.data() + data.size());
            framework->Asset()->AssetTransferCompleted(transfer.get());
        }
        else
        {
            QString error = "Http GET for address \"" + reply->url().toString() + "\" returned an error: \"" + reply->errorString() + "\"";
            framework->Asset()->AssetTransferFailed(transfer.get(), error);
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
//            framework->Asset()->AssetUploadTransferFailed(transfer);
        }

        uploadTransfers.erase(iter);
        break;
    }
    case QNetworkAccessManager::DeleteOperation:
        if (reply->error() == QNetworkReply::NoError)
            LogInfo("Http DELETE to address \"" + reply->url().toString().toStdString() + "\" returned successfully.");
        else
            LogError("Http DELETE to address \"" + reply->url().toString().toStdString() + "\" failed with an error: \"" + reply->errorString().toStdString() + "\"");
        break;
    default:
        LogInfo("Unknown operation for address \"" + reply->url().toString().toStdString() + "\" finished with result: \"" + reply->errorString().toStdString() + "\"");
        break;
    }
}

AssetStoragePtr HttpAssetProvider::AddStorage(const QString &location, const QString &name)
{
    QString locationCleaned = GuaranteeTrailingSlash(location.trimmed());

    // Check if same location and name combination already exists
    for(size_t i=0; i<storages.size(); ++i)
    {
        HttpAssetStoragePtr checkStorage = storages[i];
        if (!checkStorage.get())
            continue;
        if (checkStorage->baseAddress == locationCleaned && checkStorage->storageName == name)
            return checkStorage;
    }

    // Add new if not found
    HttpAssetStoragePtr storage = HttpAssetStoragePtr(new HttpAssetStorage());
    storage->baseAddress = locationCleaned;
    storage->storageName = name;
    storage->provider = this->shared_from_this();

    storages.push_back(storage);
    return storage;
}

std::vector<AssetStoragePtr> HttpAssetProvider::GetStorages() const
{
    std::vector<AssetStoragePtr> s;
    for(size_t i = 0; i < storages.size(); ++i)
    {
        if (!storages[i].get())
            continue;
        s.push_back(storages[i]);
    }
    return s;
}
