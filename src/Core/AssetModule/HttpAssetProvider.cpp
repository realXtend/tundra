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
#include "Profiler.h"

#include <QAbstractNetworkCache>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QLocale>

/// @todo Remove the boost::local_time stuff for good when the TUNDRA_NO_BOOST code path is tested thoroughly.
#ifndef TUNDRA_NO_BOOST
// Disable C4245 warning (signed/unsigned mismatch) coming from boost
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4245)
#endif
#include <boost/date_time/local_time/local_time.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // TUNDRA_NO_BOOST

#include "MemoryLeakCheck.h"

static int MonthNameToInt(const char *month)
{
    if (!month) return 0;
    if (!_stricmp(month, "Jan") || !_stricmp(month, "Janunary")) return 1;
    if (!_stricmp(month, "Feb") || !_stricmp(month, "February")) return 2;
    if (!_stricmp(month, "Mar") || !_stricmp(month, "March")) return 3;
    if (!_stricmp(month, "Apr") || !_stricmp(month, "April")) return 4;
    if (!_stricmp(month, "May") || !_stricmp(month, "May")) return 5;
    if (!_stricmp(month, "Jun") || !_stricmp(month, "June")) return 6;
    if (!_stricmp(month, "Jul") || !_stricmp(month, "July")) return 7;
    if (!_stricmp(month, "Aug") || !_stricmp(month, "August")) return 8;
    if (!_stricmp(month, "Sep") || !_stricmp(month, "September")) return 9;
    if (!_stricmp(month, "Oct") || !_stricmp(month, "October")) return 10;
    if (!_stricmp(month, "Nov") || !_stricmp(month, "November")) return 11;
    if (!_stricmp(month, "Dec") || !_stricmp(month, "December")) return 12;
    return 0;
}

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
    return (refType == AssetAPI::AssetRefExternalUrl && (protocol == "http" || protocol == "https"));
}

QDateTime HttpAssetProvider::ParseHttpDate(const QByteArray &value)
{
    if (value.isEmpty())
        return QDateTime();

    int dayNamePos = value.indexOf(',');
    std::string timeFacetFormat;
    if (dayNamePos == -1) // "Sun Nov 6 08:49:37 1994" - ANSI C's asctime() format
        timeFacetFormat = "%a %b %e %H:%M:%S %Y";
    else if (dayNamePos == 3) // "Sun, 06 Nov 1994 08:49:37 GMT" - RFC 822, updated by RFC 1123
        timeFacetFormat = "%a, %d %b %Y %H:%M:%S GMT";
    else if (dayNamePos > 3) // "Sunday, 06-Nov-94 08:49:37 GMT" - RFC 850, obsoleted by RFC 1036
        timeFacetFormat = "%A, %d-%b-%y %H:%M:%S GMT";

    if (timeFacetFormat.empty())
    {
        LogError("HttpAssetProvider::ParseHttpDate: Failed to detect date format from header " + QString(value.data()));
        return QDateTime();
    }

    QDateTime qDateTime;
/// @todo Remove the boost::local_time stuff for good when the TUNDRA_NO_BOOST code path is tested thoroughly.
#ifndef TUNDRA_NO_BOOST
    try
    {
        using namespace boost::local_time;
#include "DisableMemoryLeakCheck.h"
        local_time_input_facet *timeFacet(new local_time_input_facet(timeFacetFormat)); // Not a memory leak, the locale object takes over responsibility of deleting the facet object
#include "EnableMemoryLeakCheck.h"
        std::stringstream stringStream;
        stringStream.exceptions(std::ios_base::failbit);
        stringStream.imbue(std::locale(std::locale::classic(), timeFacet));
        stringStream.str(value.data());

        local_date_time dateTime(local_sec_clock::local_time(time_zone_ptr()));
        stringStream >> dateTime;
        if (dateTime.is_not_a_date_time())
        {
            LogError("HttpAssetProvider::ParseHttpDate: Failed to parse date from header " + QString(value.data()));
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
        LogError("HttpAssetProvider::ParseHttpDate: Exception while parsing date from header " + QString(value.data()) + " - " + e.what());
        return QDateTime();
    }
#else
    int hour = 0, min = 0, sec = 0, day = 0, year = 0;
    char month[4];
    /// @todo regrex validation for the string in order to prevent possible sscanf crashes.
    if (dayNamePos == -1) // "Sun Nov 6 08:49:37 1994" - ANSI C's asctime() format
    {
        sscanf(value.data(), "%*s %[a-z,A-Z] %d %d:%d:%d %d", month, &day, &hour, &min, &sec, &year);
        qDateTime = QDateTime(QDate(year, MonthNameToInt(month), day), QTime(hour, min, sec), Qt::UTC);
    }
    else if (dayNamePos == 3) // "Sun, 06 Nov 1994 08:49:37 GMT" - RFC 822, updated by RFC 1123
    {
        sscanf(value.data(), "%*s %d %[a-z,A-Z] %d %d:%d%:%d", &day, month, &year, &hour, &min, &sec);
        qDateTime = QDateTime(QDate(year, MonthNameToInt(month), day), QTime(hour, min, sec), Qt::UTC);
    }
    else if (dayNamePos > 3) // "Sunday, 06-Nov-94 08:49:37 GMT" - RFC 850, obsoleted by RFC 1036
    {
        sscanf(value.data(), "%*s %d-%[a-z,A-Z]-%d %d:%d%:%d", &day, month, &year, &hour, &min, &sec);
        if (year >= 70 && year <= 99)
            year += 1900;
        else if (year >= 0 && year < 70)
            year += 2000;
        qDateTime = QDateTime(QDate(year, MonthNameToInt(month), day), QTime(hour, min, sec), Qt::UTC);
    }
    if (!qDateTime.isValid())
        LogError("HttpAssetProvider::ParseHttpDate: Failed to parse date from header " + QString(value.data()));
#endif
    return qDateTime;
}

QByteArray HttpAssetProvider::CreateHttpDate(const QDateTime &dateTime)
{
    if (!dateTime.isValid())
        return "";
    // Sun, 06 Nov 1994 08:49:37 GMT - RFC 822.
    return QLocale::c().toString(dateTime, "ddd, dd MMM yyyy hh:mm:ss").toAscii() + QByteArray(" GMT");
}

#ifdef HTTPASSETPROVIDER_NO_HTTP_IF_MODIFIED_SINCE

std::vector<HttpAssetTransferPtr> delayedTransfers;

void HttpAssetProvider::Update(f64 frametime)
{
    for(size_t i = 0; i < delayedTransfers.size(); ++i)
        framework->Asset()->AssetTransferCompleted(delayedTransfers[i].get());
    delayedTransfers.clear();
}

#endif

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

    HttpAssetTransferPtr transfer = HttpAssetTransferPtr(new HttpAssetTransfer);
    transfer->source.ref = originalAssetRef;
    transfer->assetType = assetType;
    transfer->provider = shared_from_this();
    transfer->storage = GetStorageForAssetRef(assetRef);
    transfer->diskSourceType = IAsset::Cached; // The asset's disk source will represent a cached version of the original on the http server

    AssetCache *cache = framework->Asset()->GetAssetCache();
    QString filenameInCache = cache ? cache->FindInCache(assetRef) : QString();
#ifdef HTTPASSETPROVIDER_NO_HTTP_IF_MODIFIED_SINCE
    if (cache && framework->HasCommandLineParameter("--disable_http_ifmodifiedsince") && !filenameInCache.isEmpty())
    {
        PROFILE(HttpAssetProvider_ReadFileFromCache);

        if (QFile::exists(filenameInCache))
        {
            transfer->SetCachingBehavior(false, filenameInCache);
            delayedTransfers.push_back(transfer);
        }
        else
            framework->Asset()->AssetTransferFailed(transfer.get(), "HttpAssetProvider: Failed to read file '" + filenameInCache + "' from cache!");
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
        // Handle 307 Temporary Redirect
        else if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 307)
        {           
            // Handle "Location" header that will have the URL where the resource can be found.
            // Note that the original reply to asset transfer mapping will be removed after this block exists.
            QByteArray redirectUrl = reply->rawHeader("Location");
            if (!redirectUrl.isEmpty())
            {
                LogDebug("HttpAssetProvider: Handling \"307 Temporary Redirect\" from " + reply->url().toString() + " to " + redirectUrl);

                // Add new mapping to the asset transfer for the new redirect URL.
                QNetworkRequest redirectRequest;
                redirectRequest.setUrl(QUrl(redirectUrl));
                redirectRequest.setRawHeader("User-Agent", "realXtend Tundra");

                QNetworkReply *redirectReply = networkAccessManager->get(redirectRequest);
                transfers[QPointer<QNetworkReply>(redirectReply)] = transfer;
            }
            else
            {
                QString error = "Http GET for address \"" + reply->url().toString() + "\" returned 307 Temporary Redirect but the \"Location\" header is empty, cannot request asset from temporary redirect URL.";
                framework->Asset()->AssetTransferFailed(transfer.get(), error);
            }
        }
        // No error, proceed
        else if (reply->error() == QNetworkReply::NoError)
        {            
            AssetCache *cache = framework->Asset()->GetAssetCache();
            QString sourceRef = transfer->source.ref;
            QString error;

            int replyCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            // 304 Not Modified
            if (replyCode == 304)
            {
                // Read cache file to transfer asset data
                if (!cache->FindInCache(sourceRef).isEmpty())
                    transfer->diskSourceType = IAsset::Cached;
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
                        // Setting original source type on the request here will allow later code
                        // to detect if this is a first or update download of this asset.
                        transfer->diskSourceType = IAsset::Original;

                        // If 'Last-Modified' is not present we maybe should set it to a really old date via AssetCache::SetLastModified().
                        // As the metadata is not in a separate file it would mean for replies that did not have 'Last-Modified' header
                        // we would send the next request 'If-Modified-Since' header as the write time of the cache file.
                        // This might result in wonky situations when the server file is updated, though we can/could assume if a
                        // server does not return the 'Last-Modified' header it wont process the 'If-Modified-Since' either.
                        QByteArray header = reply->rawHeader("Last-Modified");
                        if (!header.isEmpty())
                        {
                            QDateTime sourceLastModified = ParseHttpDate(header);
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
            transfer->replyData = reply->readAll();
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
    HttpAssetStoragePtr storage = HttpAssetStoragePtr(new HttpAssetStorage());
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
