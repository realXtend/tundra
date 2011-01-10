#include "AssetCache.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "LoggingFunctions.h"

#include <QFileInfo>
#include <boost/filesystem.hpp>

DEFINE_POCO_LOGGING_FUNCTIONS("AssetCache")

QString SanitateAssetRefForCache(QString assetRef)
{ 
    assetRef.replace("/", "_");
    assetRef.replace("\\", "_");
    assetRef.replace(":", "_");
    assetRef.replace("*", "_");
    assetRef.replace("?", "_");
    assetRef.replace("\"", "_");
    assetRef.replace("'", "_");
    assetRef.replace("<", "_");
    assetRef.replace(">", "_");
    assetRef.replace("|", "_");
    return assetRef;
}

AssetCache::AssetCache(QString assetCacheDirectory)
:cacheDirectory(GuaranteeTrailingSlash(assetCacheDirectory))
{
    LogInfo("Using AssetCache in directory \"" + assetCacheDirectory.toStdString() + "\".");
}

QString AssetCache::FindAsset(QString assetRef)
{
    QString fileSource = cacheDirectory + SanitateAssetRefForCache(assetRef);
    if (QFile::exists(fileSource))
        return fileSource;
    else
        return "";
}

QString AssetCache::FindAssetByContentHash(QString contentHash)
{
    ///\todo Implement.
    return "";
}

void AssetCache::DeleteAsset(QString assetRef)
{
    QString fileSource = cacheDirectory + SanitateAssetRefForCache(assetRef);
    boost::filesystem::remove(fileSource.toStdString());
}

void AssetCache::ClearAssetCache()
{
    ///\todo Implement.
}

QString AssetCache::StoreAsset(const u8 *data, size_t numBytes, QString assetName, QString assetContentHash)
{
    QString filename = cacheDirectory + SanitateAssetRefForCache(assetName);

    bool success = SaveAssetFromMemoryToFile(data, numBytes, filename.toStdString().c_str());
    if (success)
    {
        //LogDebug("Saved asset \"" + assetName.toStdString() + "\" to cache into file \"" + filename.toStdString() + "\"");
        return filename;
    }
    else
        return "";
}

void AssetCache::StoreAsset(AssetPtr asset)
{
    std::vector<u8> data;
    asset->SerializeTo(data);
    StoreAsset(&data[0], data.size(), asset->Name(), asset->ContentHash()); ///\todo Content hash can mismatch here.
}
