// For conditions of distribution and use, see copyright notice in LICENSE

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

#include <boost/date_time/local_time/local_time.hpp>

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

QDateTime HttpAssetProvider::FromHttpDate(const QByteArray &value)
{
    if (value.isEmpty())
        return QDateTime();

    int dayNamePos = value.indexOf(',');
    std::string timeFacetFormat;

    // Sun Nov 6 08:49:37 1994 - ANSI C's asctime() format
    if (dayNamePos == -1)
        timeFacetFormat = "%a %b %e %H:%M:%S %Y";
    // Sun, 06 Nov 1994 08:49:37 GMT - RFC 822, updated by RFC 1123
    else if (dayNamePos == 3)
        timeFacetFormat = "%a, %d %b %Y %H:%M:%S GMT";
    // Sunday, 06-Nov-94 08:49:37 GMT - RFC 850, obsoleted by RFC 1036
    else if (dayNamePos > 3)
        timeFacetFormat = "%A, %d-%b-%y %H:%M:%S GMT";
    if (timeFacetFormat.empty())
    {
        LogError("HttpAssetProvider: Failed to detect date format from header " + QString(value.data()));
        return QDateTime();
    }

    QDateTime qDateTime;

    try
    {
        using namespace boost::local_time;
        local_time_input_facet *timeFacet(new local_time_input_facet(timeFacetFormat));

        // This is not a memory leak. 'timeFacet' is destoryed by the std::local object, from a c++ reference guide:
        // "The locale object takes over responsibility of deleting this facet object."
        std::stringstream stringStream;
        stringStream.exceptions(std::ios_base::failbit);
        stringStream.imbue(std::locale(std::locale::classic(), timeFacet));
        stringStream.str(value.data());

        local_date_time dateTime(local_sec_clock::local_time(time_zone_ptr()));
        stringStream >> dateTime;

        if (dateTime.is_not_a_date_time())
        {
            LogError("HttpAssetProvider: Failed to parse date from header " + QString(value.data()));
            return QDateTime();
        }

        local_date_time::date_type boostDate = dateTime.date();
        local_date_time::time_duration_type boostTime = dateTime.time_of_day();

        // Construct QDateTime and ignore milliseconds
        qDateTime = QDateTime(QDate(boostDate.year(), boostDate.month(), boostDate.day()),
                              QTime(boostTime.hours(), boostTime.minutes(), boostTime.seconds(), 0), Qt::UTC);
    }
    catch(std::exception &e)
    {
        LogError("HttpAssetProvider: Exception while parsing date from header " + QString(value.data()) + " - " + e.what());
        return QDateTime();
    }

    return qDateTime;
}

QByteArray HttpAssetProvider::ToHttpDate(const QDateTime &dateTime)
{
    if (!dateTime.isValid())
        return "";
    // Sun, 06 Nov 1994 08:49:37 GMT - RFC 822.
    return QLocale::c().toString(dateTime, "ddd, dd MMM yyyy hh:mm:ss").toAscii() + QByteArray(" GMT");
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
    
    // Fill 'If-Modified-Since' header if we have a valid cache item.
    // Server can then reply with 304 Not Modified.
    QDateTime cacheLastModified = framework->Asset()->GetAssetCache()->LastModified(assetRef);
    if (cacheLastModified.isValid())
        request.setRawHeader("If-Modified-Since", ToHttpDate(cacheLastModified));
    
    QNetworkReply *reply = networkAccessManager->get(request);

    HttpAssetTransferPtr transfer = HttpAssetTransferPtr(new HttpAssetTransfer);
    transfer->source.ref = originalAssetRef;
    transfer->assetType = assetType;
    transfer->provider = shared_from_this();
    transfer->storage = GetStorageForAssetRef(assetRef);
    transfer->diskSourceType = IAsset::Cached; // The asset's disk source will represent a cached version of the original on the http server
    transfers[reply] = transfer;
    return transfer;
}

AssetUploadTransferPtr HttpAssetProvider::UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const QString &assetName)
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
    case QNetworkAccessManager::GetOperation:
    {
        TransferMap::iterator iter = transfers.find(reply);
        if (iter == transfers.end())
        {
            LogError("GetOperation: Received a finish signal of an unknown Http transfer!");
            return;
        }
        HttpAssetTransferPtr transfer = iter->second;
        assert(transfer);
        transfer->rawAssetData.clear();

        // Check for errors
        if (reply->error() == QNetworkReply::NoError)
        {            
            AssetCache *cache = framework->Asset()->GetAssetCache();
            QString sourceRef = transfer->source.ref;
            QString error;

            int replyCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            // 304 Not Modified
            if (replyCode == 304)
            {
                // Read cache file to transfer asset data
                QFile cacheFile(cache->FindInCache(sourceRef));
                if (cacheFile.open(QIODevice::ReadOnly))
                {
                    QByteArray cacheData = cacheFile.readAll();
                    transfer->rawAssetData.insert(transfer->rawAssetData.end(), cacheData.data(), cacheData.data() + cacheData.size());
                    cacheFile.close();
                }
                else
                    error = "Http GET for address \"" + reply->url().toString() + "\" returned '304 Not Modified' but existing cache file could not be opened: \"" + cache->GetDiskSourceByRef(sourceRef) + "\"";
            }
            // 200 OK
            else if (replyCode == 200)
            {
                // Read body to transfer asset data
                QByteArray bodyData = reply->readAll();
                transfer->rawAssetData.insert(transfer->rawAssetData.end(), bodyData.data(), bodyData.data() + bodyData.size());

                if (transfer->CachingAllowed())
                {
                    // Store to cache
                    if (!cache->StoreAsset((u8*)bodyData.data(), bodyData.size(), sourceRef).isEmpty())
                    {
                        // If 'Last-Modified' is not present we maybe should set it to a really old date via AssetCache::SetLastModified().
                        // As the metadata is not in a separate file it would mean for replies that did not have 'Last-Modified' header
                        // we would send the next request 'If-Modified-Since' header as the write time of the cache file.
                        // This might result in wonky situations when the server file is updated, though we can/could assume if a
                        // server does not return the 'Last-Modified' header it wont process the 'If-Modified-Since' either.
                        QByteArray header = reply->rawHeader("Last-Modified");
                        if (!header.isEmpty())
                        {
                            QDateTime sourceLastModified = FromHttpDate(header);
                            if (sourceLastModified.isValid())
                                cache->SetLastModified(sourceRef, sourceLastModified);
                        }
                    }
                    else
                        LogWarning("HttpAssetProvider: Failed to store asset to cache after completed reply: " + sourceRef);
                }
                else
                {
                    // Remove possible cache file if caching is disabled for the transfer.
                    if (!cache->FindInCache(sourceRef).isEmpty())
                        cache->DeleteAsset(sourceRef);
                }
            }
            else
                error = "Http GET for address \"" + reply->url().toString() + "\" returned code " + QString::number(replyCode) + " that could not be processed.";

            // Send AssetTransferCompleted or AssetTransferFailed to AssetAPI.
            if (error.isEmpty())
            {
                transfer->SetCachingBehavior(false, cache->GetDiskSourceByRef(sourceRef));
                framework->Asset()->AssetTransferCompleted(transfer.get());
            }
            else
                framework->Asset()->AssetTransferFailed(transfer.get(), error);
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
