// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Application.h"
#include "HttpAssetProvider.h"
#include "HttpAssetTransfer.h"
#include "IAssetUploadTransfer.h"
#include "AssetAPI.h"
#include "AssetCache.h"
#include "IAsset.h"
#include "LoggingFunctions.h"

#include <QAbstractNetworkCache>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QLocale>

#include "MemoryLeakCheck.h"

HttpAssetProvider::HttpAssetProvider(Framework *framework_) :
    framework(framework_),
    networkAccessManager(0)
{
    CreateAccessManager();
    connect(framework->App(), SIGNAL(ExitRequested()), SLOT(AboutToExit()));

    enableRequestsOutsideStorages = framework_->HasCommandLineParameter("--accept_unknown_http_sources");
}

HttpAssetProvider::~HttpAssetProvider()
{
}

void HttpAssetProvider::CreateAccessManager()
{
    if (!networkAccessManager)
    {
        networkAccessManager = new QNetworkAccessManager();
        connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), SLOT(OnHttpTransferFinished(QNetworkReply*)));
    }
}

void HttpAssetProvider::AboutToExit()
{
    // Check if someone has canceled the exit command.
    if (!framework->IsExiting())
        return;

    if (networkAccessManager)
        SAFE_DELETE(networkAccessManager);
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

// Copied from QNetworkHeadersPrivate used in HttpAssetProvider::FromHttpDate. 
// Can be rewritten at some point if this looks too ugly. Original comments from qt:
// Fast month string to int conversion. This code
// assumes that the Month name is correct and that
// the string is at least three chars long.
static int ShortMonthNameToNum(const char* month_str)
{
    switch (month_str[0]) 
    {
    case 'J':
        switch (month_str[1]) 
        {
        case 'a':
            return 1;
            break;
        case 'u':
            switch (month_str[2] ) 
            {
            case 'n':
                return 6;
                break;
            case 'l':
                return 7;
                break;
            }
        }
        break;
    case 'F':
        return 2;
        break;
    case 'M':
        switch (month_str[2] ) 
        {
        case 'r':
            return 3;
            break;
        case 'y':
            return 5;
            break;
        }
        break;
    case 'A':
        switch (month_str[1]) 
        {
        case 'p':
            return 4;
            break;
        case 'u':
            return 8;
            break;
        }
        break;
    case 'O':
        return 10;
        break;
    case 'S':
        return 9;
        break;
    case 'N':
        return 11;
        break;
    case 'D':
        return 12;
        break;
    }

    return 0;
}

// This function has been copied with small modifications from Qts QNetworkHeadersPrivate::fromHttpDate().
// Which is the same code that QNetworkAccessManager would use if we would use a proper qt cache for networking.
QDateTime HttpAssetProvider::FromHttpDate(const QByteArray &value)
{
    // HTTP dates have three possible formats. If something else we return a null QDateTime.
    // - RFC 1123/822      -   ddd, dd MMM yyyy hh:mm:ss "GMT"
    // - RFC 850           -   dddd, dd-MMM-yy hh:mm:ss "GMT"
    // - ANSI C's asctime  -   ddd MMM d hh:mm:ss yyyy

    int pos = value.indexOf(',');
    QDateTime dt;
    if (pos == -1) 
    {
        // no comma -> asctime(3) format
        dt = QDateTime::fromString(QString::fromLatin1(value), Qt::TextDate);
    } 
    else 
    {
        // Use sscanf over QLocal/QDateTimeParser for speed reasons. See the
        // QtWebKit performance benchmarks to get an idea.
        if (pos == 3) 
        {
            char month_name[4];
            int day, year, hour, minute, second;
            if (sscanf(value.constData(), "%*3s, %d %3s %d %d:%d:%d 'GMT'", &day, month_name, &year, &hour, &minute, &second) == 6)
                dt = QDateTime(QDate(year, ShortMonthNameToNum(month_name), day), QTime(hour, minute, second));
        } 
        else 
        {
            QLocale c = QLocale::c();
            // eat the weekday, the comma and the space following it
            QString sansWeekday = QString::fromLatin1(value.constData() + pos + 2);
            // must be RFC 850 date
            dt = c.toDateTime(sansWeekday, QLatin1String("dd-MMM-yy hh:mm:ss 'GMT'"));
        }
    }

    if (dt.isValid())
        dt.setTimeSpec(Qt::UTC);
    return dt;
}

bool HttpAssetProvider::IsValidDiskSource(const QString assetRef, const QString &diskSource)
{
    // For the http provider we always return false. We never want AssetAPI to use a cache file
    // before doing a RequestAsset first. This is when a last modified header check is performed
    // to determine if the source asset is never than what we have in cache.
    return false;
}
        
AssetTransferPtr HttpAssetProvider::RequestAsset(QString assetRef, QString assetType)
{
    if (!networkAccessManager)
        CreateAccessManager();

    if (!enableRequestsOutsideStorages)
    {
        AssetStoragePtr storage = GetStorageForAssetRef(assetRef);
        if (!storage)
        {
            LogError("HttpAssetProvider::RequestAsset: Discarding asset request to URL \"" + assetRef + "\" because requests to sources outside HttpAssetStorages have been forbidden. (See --accept_unknown_http_sources).");
            return AssetTransferPtr();
        }
    }

    QString originalAssetRef = assetRef;
    assetRef = assetRef.trimmed();
    QString assetRefWithoutSubAssetName;
    AssetAPI::ParseAssetRef(assetRef, 0, 0, 0, 0, 0, 0, 0, 0, 0, &assetRefWithoutSubAssetName);
    assetRef = assetRefWithoutSubAssetName;
    if (!IsValidRef(assetRef))
    {
        LogError("HttpAssetProvider::RequestAsset: Cannot get asset from invalid URL \"" + assetRef + "\"!");
        return AssetTransferPtr();
    }

    QNetworkRequest request;
    request.setUrl(QUrl(assetRef));
    request.setRawHeader("User-Agent", "realXtend Tundra");

    QNetworkReply *reply = 0;

    // HEAD request if we have a cache file for the asset ref.
    // Meaning we get last modified from server and compare if 
    // the source file is newer than our cache file.
    if (!framework->Asset()->GetAssetCache()->GetDiskSourceByRef(assetRef).isEmpty())
        reply = networkAccessManager->head(request);
    else
        reply = networkAccessManager->get(request);

    HttpAssetTransferPtr transfer = HttpAssetTransferPtr(new HttpAssetTransfer);
    transfer->source.ref = originalAssetRef;
    transfer->assetType = assetType;
    transfer->provider = shared_from_this();
    transfer->storage = GetStorageForAssetRef(assetRef);
    transfer->diskSourceType = IAsset::Cached; // The asset's disk source will represent a cached version of the original on the http server
    transfers[reply] = transfer;
    return transfer;
}

AssetUploadTransferPtr HttpAssetProvider::UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName)
{
    if (!networkAccessManager)
        CreateAccessManager();

    QString dstUrl = destination->GetFullAssetURL(assetName);
    QNetworkRequest request;
    request.setUrl(QUrl(dstUrl));
    request.setRawHeader("User-Agent", "realXtend Tundra");

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
    if (!networkAccessManager)
        CreateAccessManager();

    assetRef = assetRef.trimmed();
    if (!IsValidRef(assetRef))
    {
        LogError("HttpAssetProvider::DeleteAssetFromStorage: Cannot delete asset from invalid URL \"" + assetRef + "\"!");
        return;
    }
    QUrl assetUrl(assetRef);
    QNetworkRequest request;
    request.setUrl(QUrl(assetRef));
    request.setRawHeader("User-Agent", "realXtend Tundra");

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

AssetStoragePtr HttpAssetProvider::TryDeserializeStorageFromString(const QString &storage, bool fromNetwork)
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

    bool liveUpdate = true;
    bool autoDiscoverable = false;
    if (s.contains("liveupdate"))
        liveUpdate = ParseBool(s["liveupdate"]);
    if (s.contains("autodiscoverable"))
        autoDiscoverable = ParseBool(s["autodiscoverable"]);
    
    HttpAssetStoragePtr newStorage = AddStorageAddress(protocolPath, name, liveUpdate, autoDiscoverable);

    // Set local dir if specified
    ///\bug Refactor these sets to occur inside AddStorageAddress so that when the NewStorageAdded signal is emitted, these values are up to date.
    if (newStorage)
    {
        if (!fromNetwork && s.contains("localdir")) // If we get a storage from a remote computer, discard the localDir parameter if it had been set.
            newStorage->localDir = GuaranteeTrailingSlash(s["localdir"]);
        if (s.contains("readonly"))
            newStorage->writable = !ParseBool(s["readonly"]);
        if (s.contains("replicated"))
            newStorage->SetReplicated(ParseBool(s["replicated"]));
        if (s.contains("trusted"))
            newStorage->trustState = IAssetStorage::TrustStateFromString(s["trusted"]);
    }
    
    return newStorage;
}

QString HttpAssetProvider::GenerateUniqueStorageName() const
{
    QString name = "Web";
    int counter = 2;
    while(GetStorageByName(name) != 0)
        name = "Web" + QString::number(counter++);
    return name;
}

void HttpAssetProvider::OnHttpTransferFinished(QNetworkReply *reply)
{
    // QNetworkAccessManager requires us to delete the QNetworkReply, or it will leak.
    reply->deleteLater();

    switch(reply->operation())
    {
    case QNetworkAccessManager::HeadOperation:
    {
        TransferMap::iterator iter = transfers.find(reply);
        if (iter == transfers.end())
        {
            LogError("HeadOperation: Received a finish signal of an unknown Http transfer!");
            return;
        }
        HttpAssetTransferPtr transfer = iter->second;
        assert(transfer);
        transfer->rawAssetData.clear();

        bool requestBody = true;
        QString sourceRef = transfer->source.ref;

        if (reply->error() == QNetworkReply::NoError)
        {
            AssetCache *cache = framework->Asset()->GetAssetCache();   

            // Compare cache and header time stamps.
            // If Last-Modified is not found or the date parsing fails, request full body as fall back.
            QByteArray header = reply->rawHeader("Last-Modified");
            if (!header.isEmpty())
            {
                QDateTime cacheLastModified = cache->LastModified(sourceRef);
                QDateTime sourceLastModified = FromHttpDate(header); // Converts to UTC time spec
#ifdef Q_WS_WIN
                // Set same time spec for comparison to work on windows. Spesific to FromHttpDate and
                // and this provider time spec conversion. So this cannot be really resolved in AssetCache::LastModified().
                cacheLastModified.setTimeSpec(sourceLastModified.timeSpec());
#endif
                if (!sourceLastModified.isNull() && !cacheLastModified.isNull())
                {
                    if (cacheLastModified.toMSecsSinceEpoch() >= sourceLastModified.toMSecsSinceEpoch())
                        requestBody = false;
                }
            }

            // Create transfer raw data from cache
            if (!requestBody)
            {
                QFile cacheFile(cache->GetDiskSourceByRef(sourceRef));
                if (cacheFile.open(QIODevice::ReadOnly))
                {
                    QByteArray cacheData = cacheFile.readAll();
                    cacheFile.close();
                    transfer->rawAssetData.insert(transfer->rawAssetData.end(), cacheData.data(), cacheData.data() + cacheData.size());

                    // Mark as already cached so AssetAPI wont overwrite the cache file.
                    transfer->SetCachingBehavior(false, cache->GetDiskSourceByRef(sourceRef));
                    framework->Asset()->AssetTransferCompleted(transfer.get());
                }
                // If cache file open fails, request the full body as fall back.
                else
                    requestBody = true;
            }
        }

        // If HEAD operation failed requestBody == true, this does not mean that GET will fail. Some servers eg. REST APIs
        // do not implement HEAD but will implement GET. HEAD might have also succeeded but our cache is out of date
        // in this case requestBody is also true.
        if (requestBody)
        {
            QNetworkRequest request;
            request.setUrl(QUrl(sourceRef));
            request.setRawHeader("User-Agent", "realXtend Tundra");

            QNetworkReply *bodyReply = networkAccessManager->get(request);
            transfers[bodyReply] = transfer;
        }

        transfers.erase(iter);
        break;
    }
    case QNetworkAccessManager::GetOperation:
    {
        QByteArray data = reply->readAll();
        TransferMap::iterator iter = transfers.find(reply);
        if (iter == transfers.end())
        {
            LogError("GetOperation: Received a finish signal of an unknown Http transfer!");
            return;
        }
        HttpAssetTransferPtr transfer = iter->second;
        assert(transfer);
        transfer->rawAssetData.clear();

        if (reply->error() == QNetworkReply::NoError)
        {
            QString sourceRef = transfer->source.ref;
            AssetCache *cache = framework->Asset()->GetAssetCache();

            if (transfer->CachingAllowed())
            {
                // Save file to cache so we can update its last modified right here.
                if (!cache->StoreAsset((u8*)data.data(), data.size(), sourceRef).isEmpty())
                {
                    QByteArray header = reply->rawHeader("Last-Modified");
                    if (!header.isEmpty())
                    {
                        QDateTime sourceLastModified = FromHttpDate(header); // Converts to UTC time spec
                        if (sourceLastModified.isValid())
                            cache->SetLastModified(sourceRef, sourceLastModified);
                    }
                }
                else
                    LogError("Failed to store asset to cache: " + sourceRef);
            }
            else
                cache->DeleteAsset(sourceRef);

            // Mark as already cached so AssetAPI wont do it again. If caching was not allowed this sets a empty string as the disk source.
            transfer->SetCachingBehavior(false, cache->GetDiskSourceByRef(sourceRef));

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
            LogError("PostOperation: Received a finish signal of an unknown Http upload transfer!");
            return;
        }
        AssetUploadTransferPtr transfer = iter->second;

        if (reply->error() == QNetworkReply::NoError)
        {
            QString ref = reply->url().toString();
            LogDebug("Http upload to address \"" + ref + "\" returned successfully.");
            framework->Asset()->AssetUploadTransferCompleted(transfer.get());
            // Add the assetref to matching storage(s)
            AddAssetRefToStorages(ref);
        }
        else
        {
            LogError("Http upload to address \"" + reply->url().toString() + "\" failed with an error: \"" + reply->errorString() + "\"");
            ///\todo Call the following when implemented:
//            framework->Asset()->AssetUploadTransferFailed(transfer);
        }
        uploadTransfers.erase(iter);
        break;
    }
    case QNetworkAccessManager::DeleteOperation:
        if (reply->error() == QNetworkReply::NoError)
        {
            QString ref = reply->url().toString();
            LogInfo("Http DELETE to address \"" + ref + "\" returned successfully.");
            DeleteAssetRefFromStorages(ref);
            framework->Asset()->EmitAssetDeletedFromStorage(ref);
        }
        else
            LogError("Http DELETE to address \"" + reply->url().toString() + "\" failed with an error: \"" + reply->errorString() + "\"");
        break;
        /*
    default:
        LogInfo("Unknown operation for address \"" + reply->url().toString() + "\" finished with result: \"" + reply->errorString() + "\"");
        break;
        */
    }
}

HttpAssetStoragePtr HttpAssetProvider::AddStorageAddress(const QString &address, const QString &storageName, bool liveUpdate, bool autoDiscoverable)
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
    storage->liveUpdate = liveUpdate;
    storage->autoDiscoverable = autoDiscoverable;
    storage->provider = this->shared_from_this();
    storages.push_back(storage);
    
    // Tell the Asset API that we have created a new storage.
    framework->Asset()->EmitAssetStorageAdded(storage);

    if (storage->AutoDiscoverable())
        storage->RefreshAssetRefs(); // Initiate PROPFIND
    
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

AssetStoragePtr HttpAssetProvider::GetStorageForAssetRef(const QString &assetRef) const
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
