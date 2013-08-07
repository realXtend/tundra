// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Application.h"
#include "HighPerfClock.h"
#include "HttpAssetProvider.h"
#include "HttpAssetTransfer.h"
#include "IAssetUploadTransfer.h"
#include "AssetAPI.h"
#include "AssetCache.h"
#include "IAsset.h"
#include "LoggingFunctions.h"
#include "Profiler.h"

#include <QAbstractNetworkCache>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QLocale>
#include <QThreadPool>

#include "MemoryLeakCheck.h"

/** Uncomment to enable a --disable_http_ifmodifiedsince command line parameter.
    This is used to profile the performance effect the HTTP queries have on scene loading times. */
//#define HTTPASSETPROVIDER_NO_HTTP_IF_MODIFIED_SINCE

/** Currently everything is written async. Adjust this to
    force smaller files be written in the main thread. */
int HttpAssetProvider::AsyncCacheWriteThreshold = 0 * 1024;

HttpAssetProvider::HttpAssetProvider(Framework *framework_) :
    framework(framework_),
    networkAccessManager(0)
{
    /** @todo @bug Figure out how to do this cleanly. AssetTransferPtr is used in a signal in this class.
        The Q_DECLARE_METATYPE(AssetTransferPtr) in IAssetTransfer does not do the trick. */
    qRegisterMetaType<AssetTransferPtr>("AssetTransferPtr");

    CreateAccessManager();
    connect(framework->App(), SIGNAL(ExitRequested()), SLOT(AboutToExit()));

    enableRequestsOutsideStorages = (framework_->HasCommandLineParameter("--acceptUnknownHttpSources") ||
        framework_->HasCommandLineParameter("--accept_unknown_http_sources"));  /**< @todo Remove support for the deprecated underscore version at some point. */
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
    return (refType == AssetAPI::AssetRefExternalUrl && (protocol == "http" || protocol == "https"));
}

QByteArray HttpAssetProvider::CreateHttpDate(const QDateTime &dateTime)
{
    if (!dateTime.isValid())
        return "";
    // Sun, 06 Nov 1994 08:49:37 GMT - RFC 822.
    return QLocale::c().toString(dateTime, "ddd, dd MMM yyyy hh:mm:ss").toAscii() + QByteArray(" GMT");
}

void HttpAssetProvider::Update(f64 /*frametime*/)
{
    if (!completedTransfers.isEmpty())
    {
        const int maxLoadMSecs = 16;
        tick_t startTime = GetCurrentClockTime();

        while(completedTransfers.size() > 0)
        {
            PROFILE(HttpAssetProvider_ProcessCompleted);

            AssetTransferPtr transfer = completedTransfers.front();
            completedTransfers.pop_front();

            framework->Asset()->AssetTransferCompleted(transfer.get());

            // Throttle asset loading to at most 16 msecs/frame.
            if (GetCurrentClockTime() - startTime >= GetCurrentClockFreq() * maxLoadMSecs / 1000)
                break;
        }
    }
}

AssetTransferPtr HttpAssetProvider::RequestAsset(QString assetRef, QString assetType)
{
    PROFILE(HttpAssetProvider_RequestAsset);
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
    AssetAPI::AssetRefType refType = AssetAPI::ParseAssetRef(assetRef, 0, 0, 0, 0, 0, 0, 0, 0, 0, &assetRefWithoutSubAssetName);
    
    assert(refType == AssetAPI::AssetRefExternalUrl);
    UNREFERENCED_PARAM(refType);
    
    assetRef = assetRefWithoutSubAssetName;
    if (!IsValidRef(assetRef))
    {
        LogError("HttpAssetProvider::RequestAsset: Cannot get asset from invalid URL \"" + assetRef + "\"!");
        return AssetTransferPtr();
    }

    HttpAssetTransferPtr transfer = MAKE_SHARED(HttpAssetTransfer);
    transfer->source.ref = originalAssetRef;
    transfer->assetType = assetType;
    transfer->provider = shared_from_this();
    transfer->storage = GetStorageForAssetRef(assetRef);
    transfer->diskSourceType = IAsset::Cached; // The asset's disk source will represent a cached version of the original on the http server

#ifdef HTTPASSETPROVIDER_NO_HTTP_IF_MODIFIED_SINCE
    QString cachePath = framework->Asset()->GetAssetCache()->FindInCache(assetRef);
    if (framework->HasCommandLineParameter("--disable_http_ifmodifiedsince") && !cachePath.isEmpty())
    {
        PROFILE(HttpAssetProvider_ReadFileFromCache);
        transfer->SetCachingBehavior(false, cachePath);
        completedTransfers.push_back(transfer);
    }
    else
#endif
    {
        QNetworkRequest request;
        request.setUrl(QUrl(assetRef));
        request.setRawHeader("User-Agent", "realXtend Tundra");
    
        // Fill 'If-Modified-Since' header if we have a valid cache item.
        // Server can then reply with 304 Not Modified.
        QDateTime cacheLastModified = framework->Asset()->GetAssetCache()->LastModified(assetRef);
        if (cacheLastModified.isValid())
            request.setRawHeader("If-Modified-Since", CreateHttpDate(cacheLastModified));
        
        QNetworkReply *reply = networkAccessManager->get(request);
        transfers[QPointer<QNetworkReply>(reply)] = transfer;
    }
    return transfer;
}

bool HttpAssetProvider::AbortTransfer(IAssetTransfer *transfer)
{
    if (!transfer)
        return false;

    for (TransferMap::iterator iter = transfers.begin(); iter != transfers.end(); ++iter)
    {
        AssetTransferPtr ongoingTransfer = iter->second;
        if (ongoingTransfer.get() == transfer)
        {
            // QNetworkReply::abort() will invoke a call to OnHttpTransferFinished. There we continue to 
            // call AssetAPI::AssetTransferAborted and remove the transfer from our map.
            QPointer<QNetworkReply> reply = iter->first;
            if (reply.data())
            {    
                reply->abort();
                return true;
            }
        }
    }
    return false;
}

AssetUploadTransferPtr HttpAssetProvider::UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const QString &assetName)
{
    if (!networkAccessManager)
        CreateAccessManager();

    QString dstUrl = destination->GetFullAssetURL(assetName);
    QNetworkRequest request;
    request.setUrl(QUrl(dstUrl));
    request.setRawHeader("User-Agent", "realXtend Tundra");

    QByteArray dataArray((const char*)data, (int)numBytes);
    QNetworkReply *reply = networkAccessManager->put(request, dataArray);

    AssetUploadTransferPtr transfer = MAKE_SHARED(IAssetUploadTransfer);
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
    bool liveUpload = false;
    if (s.contains("liveupdate"))
        liveUpdate = ParseBool(s["liveupdate"]);
    if (s.contains("liveupload"))
        liveUpdate = ParseBool(s["liveupload"]);
    if (s.contains("autodiscoverable"))
        autoDiscoverable = ParseBool(s["autodiscoverable"]);
    
    HttpAssetStoragePtr newStorage = AddStorageAddress(protocolPath, name, liveUpdate, autoDiscoverable, liveUpload);

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

    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QString replyUrl = reply->url().toString();

    switch(reply->operation())
    {
    case QNetworkAccessManager::GetOperation:
    {
        TransferMap::iterator iter = transfers.find(reply);
        if (iter == transfers.end())
            return;
        HttpAssetTransferPtr transfer = iter->second;
        transfer->rawAssetData.clear();

        // We have called abort() or close() on an ongoing transfer, for example in AbortTransfer.
        if (reply->error() == QNetworkReply::OperationCanceledError)
        {
            framework->Asset()->AssetTransferAborted(transfer.get());
        }
        // Handle '307 Temporary Redirect', '302 Found' and '303 See Other' redirects. All of these should return the Location header.
        else if (httpStatusCode == 307 || httpStatusCode == 302 || httpStatusCode == 303)
        {           
            // Handle "Location" header that will have the URL where the resource can be found.
            // Note that the original reply to asset transfer mapping will be removed after this block exists.
            QByteArray redirectUrl = reply->rawHeader("Location");
            if (!redirectUrl.isEmpty())
            {
                LogDebug(QString("HttpAssetProvider: Handling \"%1 %2\" from %3 to %4")
                    .arg(httpStatusCode).arg(httpStatusCode == 307 ? "Temporary Redirect" : httpStatusCode == 302 ? "Found" : "See Other")
                    .arg(replyUrl).arg(QString(redirectUrl)));

                // Add new mapping to the asset transfer for the new redirect URL.
                QNetworkRequest redirectRequest;
                redirectRequest.setUrl(QUrl(redirectUrl));
                redirectRequest.setRawHeader("User-Agent", "realXtend Tundra");

                QNetworkReply *redirectReply = networkAccessManager->get(redirectRequest);
                transfers[QPointer<QNetworkReply>(redirectReply)] = transfer;
            }
            else
                framework->Asset()->AssetTransferFailed(transfer.get(), QString("Http GET for address \"%1\" returned %2 status code but the \"Location\" header is empty, cannot request asset from redirected URL.")
                    .arg(replyUrl).arg(httpStatusCode));
        }
        // No error, proceed
        else if (reply->error() == QNetworkReply::NoError)
        {            
            AssetCache *cache = framework->Asset()->GetAssetCache();
            QString sourceRef = transfer->source.ref;
            QString error;

            // 304 Not Modified
            if (httpStatusCode == 304)
            {
                // Read cache file to transfer asset data
                if (!cache->FindInCache(sourceRef).isEmpty())
                    transfer->diskSourceType = IAsset::Cached;
                else
                    error = QString("Http GET for address \"%1\" returned '304 Not Modified' but existing cache file could not be opened: \"%2\"").arg(replyUrl).arg(cache->GetDiskSourceByRef(sourceRef));
            }
            // 200 OK
            else if (httpStatusCode == 200)
            {
                // Setting original source type on the request here will allow later code
                // to detect if this is a first or update download of this asset.
                transfer->diskSourceType = IAsset::Original;

                // Read body to transfer asset data
                QByteArray bodyData = reply->readAll();
                if (transfer->CachingAllowed())
                {
                    if (bodyData.size() > AsyncCacheWriteThreshold)
                    {
                        // Store last modified header to be set after the write operation is done.
                        transfer->setProperty("LastModifiedHeader", reply->header(QNetworkRequest::LastModifiedHeader));

                        // Spawn a new cache write operation for this transfer with the global Qt thread pool.
                        TransferCacheWriteOperation *cacheWriteOperation = new TransferCacheWriteOperation(transfer, cache->GetDiskSourceByRef(sourceRef), bodyData);
                        connect(cacheWriteOperation, SIGNAL(Completed(AssetTransferPtr, bool)), SLOT(OnCacheWriteCompleted(AssetTransferPtr, bool)), Qt::QueuedConnection);
                        QThreadPool::globalInstance()->start(cacheWriteOperation);

                        // Erase transfer from internal state and return.
                        transfers.erase(iter);
                        return;
                    }

                    // The data size is below our threshold, write to cache on the main thread.
                    if (!cache->StoreAsset((u8*)bodyData.data(), bodyData.size(), sourceRef).isEmpty())
                    {
                        QVariant lastModifiedVariant = reply->header(QNetworkRequest::LastModifiedHeader);
                        if (lastModifiedVariant.isValid())
                            cache->SetLastModified(sourceRef, lastModifiedVariant.toDateTime());
                    }
                    else
                        LogWarning("HttpAssetProvider: Failed to store asset to cache after completed reply: " + replyUrl);
                }
                // Caching is not allowed. Remove possible cached source from disk.
                else
                    cache->DeleteAsset(sourceRef);

                // Write original source data to the transfer.
                transfer->rawAssetData.insert(transfer->rawAssetData.end(), bodyData.data(), bodyData.data() + bodyData.size());
            }
            else
                error = QString("Http GET for address \"%1\" returned status code %2 that could not be processed.").arg(replyUrl).arg(httpStatusCode);

            // If no errors queue the trasnfer to be sent to AssetAPI::AssetTransferCompleted
            if (error.isEmpty())
            {
                // This tells AssetAPI going forward that storing to cache has been done, otherwise it will rewrite the file.
                transfer->SetCachingBehavior(false, transfer->CachingAllowed() ? cache->GetDiskSourceByRef(sourceRef) : "");
                completedTransfers << transfer;
            }
            else
                framework->Asset()->AssetTransferFailed(transfer.get(), error);
        }
        else
            framework->Asset()->AssetTransferFailed(transfer.get(), QString("Http GET for address \"%1\" returned an error: %2").arg(replyUrl).arg(reply->errorString()));

        // Erase the transfer from internal state.
        transfers.erase(iter);
        break;
    }
    case QNetworkAccessManager::PutOperation:
    case QNetworkAccessManager::PostOperation:
    {
        UploadTransferMap::iterator iter = uploadTransfers.find(reply);
        if (iter == uploadTransfers.end())
        {
            LogError("PostOperation: Received a finish signal of an unknown Http upload transfer: " + replyUrl);
            return;
        }
        AssetUploadTransferPtr transfer = iter->second;

        if (reply->error() == QNetworkReply::NoError)
        {
            LogDebug(QString("Http upload to address \"%1\" returned successfully.").arg(replyUrl));

            // Set reply data.
            transfer->replyData = reply->readAll();

            // Set reply headers.
            foreach(const QNetworkReply::RawHeaderPair &header, reply->rawHeaderPairs())
                transfer->replyHeaders[QString(header.first)] = QString(header.second);

            // Report completion.
            framework->Asset()->AssetUploadTransferCompleted(transfer.get());

            // Add the completed asset ref to matching storage(s)
            AddAssetRefToStorages(replyUrl);
        }
        else
        {
            LogError(QString("Http upload to address \"%1\" failed with an error: %2").arg(replyUrl).arg(reply->errorString()));
            /// @todo Call the AssetAPI::AssetUploadTransferFailed when implemented.
            //framework->Asset()->AssetUploadTransferFailed(transfer);
            transfer->EmitTransferFailed();
        }

        // Erase the transfer from internal state.
        uploadTransfers.erase(iter);
        break;
    }
    case QNetworkAccessManager::DeleteOperation:
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            LogDebug(QString("Http DELETE to address \"%1\" returned successfully.").arg(replyUrl));
            DeleteAssetRefFromStorages(replyUrl);
            framework->Asset()->EmitAssetDeletedFromStorage(replyUrl);
        }
        else
            LogError(QString("Http DELETE to address \"%1\" failed with an error: %2").arg(replyUrl).arg(reply->errorString()));
        break;
    }
    }
}

void HttpAssetProvider::OnCacheWriteCompleted(AssetTransferPtr transfer, bool cacheFileWritten)
{
    if (!transfer.get())
        return;

    const QString sourceRef = transfer->source.ref;
    if (cacheFileWritten)
    {
        // Update the last modified for the cached file if available.
        QVariant lastModifiedVariant = transfer->property("LastModifiedHeader");
        if (lastModifiedVariant.isValid())
            framework->Asset()->Cache()->SetLastModified(sourceRef, lastModifiedVariant.toDateTime());
    }
    else
        LogWarning("HttpAssetProvider: Failed to store asset to cache after completed reply: " + sourceRef);

    // This tells AssetAPI going forward that storing to cache has been done, otherwise it will rewrite the file.
    transfer->SetCachingBehavior(false, cacheFileWritten ? framework->Asset()->Cache()->GetDiskSourceByRef(sourceRef) : "");

    // Push to completed queue.
    completedTransfers << transfer;
}

HttpAssetStoragePtr HttpAssetProvider::AddStorageAddress(const QString &address, const QString &storageName, bool liveUpdate, bool autoDiscoverable, bool liveUpload)
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
    HttpAssetStoragePtr storage = MAKE_SHARED(HttpAssetStorage);
    storage->baseAddress = locationCleaned;
    storage->storageName = storageName;
    storage->liveUpdate = liveUpdate;
    storage->liveUpload = liveUpload;
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
    {
        if (refType == AssetAPI::AssetRefNamedStorage && storages[i]->Name() == namedStorage)
            return storages[i];
        else if (refType == AssetAPI::AssetRefExternalUrl && assetRef.startsWith(storages[i]->baseAddress, Qt::CaseInsensitive))
            return storages[i];
    }

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

// ThreadedCacheWriteOperation

TransferCacheWriteOperation::TransferCacheWriteOperation(AssetTransferPtr transfer, const QString &path, const QByteArray &data) :
    transfer_(transfer),
    path_(path),
    data_(data)
{
    // Make sure this worker object is deleted by QThreadPool once run() completes.
    setAutoDelete(true);
}

void TransferCacheWriteOperation::run()
{
    bool succeeded = false;

    // Write data to the transfer.
    if (transfer_->rawAssetData.empty())
        transfer_->rawAssetData.insert(transfer_->rawAssetData.end(), data_.data(), data_.data() + data_.size());

    // File data to disk.
    QFile file(path_);
    if (file.open(QFile::WriteOnly))
    {
        file.write(data_);
        file.close();
        succeeded = true;
    }

    emit Completed(transfer_, succeeded);
}
