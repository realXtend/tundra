// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "LocalAssetStorage.h"
#include "LocalAssetProvider.h"
#include "AssetAPI.h"
#include "FileUtils.h"
#include "Profiler.h"
#include "LoggingFunctions.h"

#include <QFileSystemWatcher>
#include <QDir>
#include <utility>

#include "MemoryLeakCheck.h"

LocalAssetStorage::LocalAssetStorage(bool writable_, bool liveUpdate_, bool autoDiscoverable_) :
    recursive(true),
    changeWatcher(0)
{
    // Override the parameters for the base class.
    writable = writable_;
    liveUpdate = liveUpdate_;
    autoDiscoverable = autoDiscoverable_;
}

LocalAssetStorage::~LocalAssetStorage()
{
    RemoveWatcher();
}

void LocalAssetStorage::LoadAllAssetsOfType(AssetAPI *assetAPI, const QString &suffix, const QString &assetType)
{
    foreach(QString str, DirectorySearch(directory, recursive, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks))
        if ((suffix == "" || str.endsWith(suffix)) && !(str.contains(".git") || str.contains(".svn") || str.contains(".hg")))
        {
            int lastSlash = str.lastIndexOf('/');
            if (lastSlash != -1)
                str = str.right(str.length() - lastSlash - 1);
            assetAPI->RequestAsset("local://" + str, assetType);
        }
}

void LocalAssetStorage::RefreshAssetRefs()
{
    foreach(QString str, DirectorySearch(directory, recursive, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks))
        if (!(str.contains(".git") || str.contains(".svn") || str.contains(".hg")))
        {
            QString diskSource = str;
            int lastSlash = str.lastIndexOf('/');
            if (lastSlash != -1)
                str = str.right(str.length() - lastSlash - 1);
            QString localName = str;
            str.prepend("local://");
            if (!assetRefs.contains(str))
            {
                assetRefs.append(str);
                emit AssetChanged(localName, diskSource, IAssetStorage::AssetCreate);
            }
        }
}

void LocalAssetStorage::CacheStorageContents()
{
    cachedFiles.clear();

    foreach(QString str, DirectorySearch(directory, recursive, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks))
        if (!(str.contains(".git") || str.contains(".svn") || str.contains(".hg")))
        {
            QString diskSource = str;
            int lastSlash = str.lastIndexOf('/');
            if (lastSlash != -1)
                str = str.right(str.length() - lastSlash - 1);
            QString localName = str;

///\todo This is an often-received error condition if the user is not aware, but also occurs naturally in built-in Ogre Media storages.
/// Fix this check to occur somehow nicer (without additional constraints to asset load time) without a hardcoded check
/// against the storage name.
            if (Name() != "Ogre Media" && cachedFiles.find(localName) != cachedFiles.end())
                LogWarning("Warning: Asset Storage \"" + Name() + "\" contains ambiguous assets \"" + cachedFiles[localName] + "\" and \"" + diskSource + "\" in two different subdirectories!");

            cachedFiles[localName] = diskSource;
        }
}

QString LocalAssetStorage::GetFullPathForAsset(const QString &assetname, bool recursiveLookup)
{
    QDir dir(GuaranteeTrailingSlash(directory) + assetname);
    if (QFile::exists(dir.absolutePath()))
        return directory;

    std::map<QString, QString, QStringLessThanNoCase>::iterator iter = cachedFiles.find(assetname);
    if (iter == cachedFiles.end())
    {
        if (!recursive || !recursiveLookup)
            return "";
        else
            CacheStorageContents();
    }

    iter = cachedFiles.find(assetname);
    if (iter != cachedFiles.end())
    {
        QFileInfo file(iter->second);
        if (file.exists())
            return file.dir().path();
    }

    return "";
}

/// @todo Make this function handle arbitrary asset refs.
QString LocalAssetStorage::GetFullAssetURL(const QString &localName)
{
    QString filename;
    QString subAssetName;
    AssetAPI::ParseAssetRef(localName, 0, 0, 0, 0, &filename, 0, 0, &subAssetName);
    return BaseURL() + filename + (subAssetName.isEmpty() ? "" : ("#" + subAssetName));
}

QString LocalAssetStorage::Type() const
{
    return "LocalAssetStorage";
}

QString LocalAssetStorage::SerializeToString(bool networkTransfer) const
{
    if (networkTransfer)
        return ""; // Cannot transfer a LocalAssetStorage through network to another computer, since it is local to this system!
    else
        return "type=" + Type() + ";name=" + name + ";src=" + directory + ";recursive=" + BoolToString(recursive) + ";readonly=" + BoolToString(!writable) +
            ";liveupdate=" + BoolToString(liveUpdate) + ";autodiscoverable=" + BoolToString(autoDiscoverable) + ";replicated=" + BoolToString(isReplicated)
            + ";trusted=" + TrustStateToString(GetTrustState());
}

void LocalAssetStorage::EmitAssetChanged(QString absoluteFilename, IAssetStorage::ChangeType change)
{
    QString localName;
    int lastSlash = absoluteFilename.lastIndexOf('/');
    if (lastSlash != -1)
        localName = absoluteFilename.right(absoluteFilename.length() - lastSlash - 1);
    QString assetRef = "local://" + localName;
    if (assetRefs.contains(assetRef) && change == IAssetStorage::AssetCreate)
        LogDebug("LocalAssetStorage::EmitAssetChanged: Emitting AssetCreate signal for already existing asset " + assetRef +
            ", file " + absoluteFilename + ". Asset was probably removed and then added back.");
    emit AssetChanged(localName, absoluteFilename, change);
}

void LocalAssetStorage::SetupWatcher()
{
    if (changeWatcher) // Remove the old watcher if one exists.
        RemoveWatcher();

    changeWatcher = new QFileSystemWatcher();

    // Add directory contents to watch list.
    if (recursive) // Make a visible log message - we may hang here for several seconds, since this step involves recursive iteration.
        LogInfo("LocalAssetStorage::SetupWatcher: recursively adding " + directory + " to file change notification watcher. This may take a while.");
    else
        LogDebug("LocalAssetStorage::SetupWatcher: adding " + directory + " recursive=" + BoolToString(recursive));

    QStringList paths = DirectorySearch(directory, recursive, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
#ifndef Q_WS_MAC
    changeWatcher->addPath(QDir::fromNativeSeparators(directory));
    changeWatcher->addPaths(paths);
#endif

    LogDebug("Total of " + QString::number(paths.count()) + " dirs and files added to watch list.");
}

void LocalAssetStorage::RemoveWatcher()
{
    SAFE_DELETE(changeWatcher);
}
