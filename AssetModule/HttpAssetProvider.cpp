// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "HttpAssetProvider.h"
#include "HttpAssetTransfer.h"
#include "LoggingFunctions.h"
#include "IAssetUploadTransfer.h"

#include "AssetAPI.h"
#include "AssetCache.h"
#include "IAsset.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

HttpAssetProvider::HttpAssetProvider(Framework *framework_)
:framework(framework_)
{
    // Http access manager
    networkAccessManager = new QNetworkAccessManager(this);
    networkAccessManager->setCache(framework_->Asset()->GetAssetCache());
    connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), SLOT(OnHttpTransferFinished(QNetworkReply*)));
    
    // Connect to asset discovery & deletion signals, to refresh assetrefs of storages without having to do a full PROPFIND refresh
    connect(framework_->Asset(), SIGNAL(AssetDiscovered(const QString &, const QString &)), this, SLOT(OnAssetDiscovered(const QString &, const QString &)));
    connect(framework_->Asset(), SIGNAL(AssetDeleted(const QString &)), this, SLOT(OnAssetDeleted(const QString &)));
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
    QString protocol;
    AssetAPI::AssetRefType refType = AssetAPI::ParseAssetRef(assetRef.trimmed(), &protocol);
    if (refType == AssetAPI::AssetRefExternalUrl && 
        (protocol == "http" || protocol == "https"))
        return true;
    else
        return false;
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
    request.setRawHeader("User-Agent", "realXtend");

    QNetworkReply *reply = networkAccessManager->get(request);

    HttpAssetTransferPtr transfer = HttpAssetTransferPtr(new HttpAssetTransfer);
    transfer->source.ref = assetRef;
    transfer->assetType = assetType;
    transfer->provider = shared_from_this();
    transfer->storage = GetStorageForAssetRef(assetRef);
    transfers[reply] = transfer;
    return transfer;
}

AssetUploadTransferPtr HttpAssetProvider::UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName)
{
    QString dstUrl = destination->GetFullAssetURL(assetName);
    QNetworkRequest request;
    request.setUrl(QUrl(dstUrl));
    request.setRawHeader("User-Agent", "realXtend");

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
    request.setRawHeader("User-Agent", "realXtend");

    networkAccessManager->deleteResource(request);
}

bool HttpAssetProvider::RemoveAssetStorage(QString storageName)
{
    for(size_t i = 0; i < storages.size(); ++i)
        if (storages[i]->storageName.compare(storageName, Qt::CaseInsensitive) == 0)
        {
            storages.erase(storages.begin() + i);
            return true;
        }

    return false;
}

AssetStoragePtr HttpAssetProvider::TryDeserializeStorageFromString(const QString &storage)
{
    QMap<QString, QString> s = AssetAPI::ParseAssetStorageString(storage);
    if (s.contains("type") && s["type"].compare("HttpAssetStorage", Qt::CaseInsensitive) != 0)
        return AssetStoragePtr();
    if (!s.contains("src"))
        return AssetStoragePtr();

    QString path;
    QString protocolPath;
    AssetAPI::AssetRefType refType = AssetAPI::ParseAssetRef(s["src"], 0, 0, &protocolPath, 0, 0, &path);

    if (refType != AssetAPI::AssetRefExternalUrl)
        return AssetStoragePtr();

    QString name = (s.contains("name") ? s["name"] : GenerateUniqueStorageName());

    return AddStorageAddress(protocolPath, name);
}

QString HttpAssetProvider::GenerateUniqueStorageName() const
{
    QString name = "Web";
    int counter = 2;
    while(GetStorageByName(name) != 0)
        name = "Web" + counter++;
    return name;
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
            // If asset request creator has not allowed caching, remove it now
            AssetCache *cache = framework->Asset()->GetAssetCache();
            if (!transfer->CachingAllowed())
                cache->remove(reply->url());

            // Setting cache allowed as false is very important! The items are already in our cache via the 
            // QAccessManagers QAbstractNetworkCache (same as our AssetAPI::AssetCache). Network replies will already call them
            // so the AssetAPI::AssetTransferCompletes doesn't have to.
            // \note GetDiskSource() will return empty string if above cache remove was performed, this is wanted behaviour.
            transfer->SetCachingBehavior(false, cache->FindInCache(reply->url().toString()));

            // Copy raw data to transfer
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
            // Add the assetref to matching storage(s)
            AddAssetRefToStorages(reply->url().toString());
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
        {
            LogInfo("Http DELETE to address \"" + reply->url().toString().toStdString() + "\" returned successfully.");
            framework->Asset()->EmitAssetDeleted(reply->url().toString());
        }
        else
            LogError("Http DELETE to address \"" + reply->url().toString().toStdString() + "\" failed with an error: \"" + reply->errorString().toStdString() + "\"");
        break;
        /*
    default:
        LogInfo("Unknown operation for address \"" + reply->url().toString().toStdString() + "\" finished with result: \"" + reply->errorString().toStdString() + "\"");
        break;
        */
    }
}

void HttpAssetProvider::OnAssetDiscovered(const QString& ref, const QString& assetType)
{
    // Add to our storages if the base matches
    AddAssetRefToStorages(ref);
}

void HttpAssetProvider::OnAssetDeleted(const QString& ref)
{
    DeleteAssetRefFromStorages(ref);
}

HttpAssetStoragePtr HttpAssetProvider::AddStorageAddress(const QString &address, const QString &storageName)
{    QString locationCleaned = GuaranteeTrailingSlash(address.trimmed());

    // Check if a storage with this name already exists.
    for(size_t i = 0; i < storages.size(); ++i)
        if (storages[i]->storageName.compare(storageName, Qt::CaseInsensitive) == 0)
        {
            if (storages[i]->baseAddress != address)
                LogError("HttpAssetProvider::AddStorageAddress failed: A storage by name \"" + storageName + "\" already exists, but points to address \"" + storages[i]->baseAddress + "\" instead of \"" + address + "\"!");
            return HttpAssetStoragePtr();
        }

    // Add new if not found
    HttpAssetStoragePtr storage = HttpAssetStoragePtr(new HttpAssetStorage());
    storage->baseAddress = locationCleaned;
    storage->storageName = storageName;
    storage->provider = this->shared_from_this();
    storages.push_back(storage);
    return storage;
}

std::vector<AssetStoragePtr> HttpAssetProvider::GetStorages() const
{
    std::vector<AssetStoragePtr> s;
    for(size_t i = 0; i < storages.size(); ++i)
        s.push_back(storages[i]);

    return s;
}

AssetStoragePtr HttpAssetProvider::GetStorageByName(const QString &name) const
{
    for(size_t i = 0; i < storages.size(); ++i)
        if (storages[i]->storageName.compare(name, Qt::CaseInsensitive) == 0)
            return storages[i];

    return AssetStoragePtr();
}

HttpAssetStoragePtr HttpAssetProvider::GetStorageForAssetRef(QString assetRef) const
{
    QString namedStorage;
    QString protocolPath;
    AssetAPI::AssetRefType refType = AssetAPI::ParseAssetRef(assetRef, 0, &namedStorage, &protocolPath);
    for (size_t i = 0; i < storages.size(); ++i)
        if (refType == AssetAPI::AssetRefNamedStorage && storages[i]->Name() == namedStorage)
    return storages[i];
        else if (refType == AssetAPI::AssetRefExternalUrl && assetRef.startsWith(storages[i]->baseAddress, Qt::CaseInsensitive))
    return storages[i];

    return HttpAssetStoragePtr();
}

void HttpAssetProvider::AddAssetRefToStorages(const QString& ref)
{
    for (size_t i = 0; i < storages.size(); ++i)
        if (ref.indexOf(storages[i]->baseAddress) == 0)
            storages[i]->AddAssetRef(ref);
}

void HttpAssetProvider::DeleteAssetRefFromStorages(const QString& ref)
{
    for (size_t i = 0; i < storages.size(); ++i)
        storages[i]->DeleteAssetRef(ref);
}
