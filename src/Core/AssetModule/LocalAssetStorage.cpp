// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "LocalAssetStorage.h"
#include "LocalAssetProvider.h"
#include "AssetAPI.h"
#include "QtUtils.h"
#include "Profiler.h"

#include <QFileSystemWatcher>
#include <QDir>
#include <utility>

#include "MemoryLeakCheck.h"

LocalAssetStorage::LocalAssetStorage() :
    recursive(true),
    writable(true),
    liveUpdate(true),
    autoDiscoverable(true),
    changeWatcher(0)
{
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

QString LocalAssetStorage::GetFullPathForAsset(const QString &assetname, bool recursiveLookup)
{
    QDir dir(GuaranteeTrailingSlash(directory) + assetname);
    if (QFile::exists(dir.absolutePath()))
        return directory;

    if (!recursive || !recursiveLookup)
        return "";

    foreach(const QString &str, DirectorySearch(directory, recursive, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks))
    {
        QFileInfo file(GuaranteeTrailingSlash(str) + assetname);
        if (file.exists())
            return file.dir().path();
    }
    return "";
}

QString LocalAssetStorage::GetFullAssetURL(const QString &localName)
{
    QString filename;
    QString subAssetName;
    AssetAPI::ParseAssetRef(localName, 0, 0, 0, 0, &filename, 0, 0, &subAssetName);
    return BaseURL() + filename + (subAssetName.isEmpty() ? "" : ("," + subAssetName));
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
        return "type=" + Type() + ";name=" + name + ";src=" + directory + ";recursive=" + (recursive ? "true" : "false") + ";readonly=" + (!writable ? "true" : "false") +
            ";liveupdate=" + (liveUpdate ? "true" : "false") + ";autodiscoverable=" + (autoDiscoverable ? "true" : "false");
}

void LocalAssetStorage::EmitAssetChanged(QString absoluteFilename, IAssetStorage::ChangeType change)
{
    QString localName;
    int lastSlash = absoluteFilename.lastIndexOf('/');
    if (lastSlash != -1)
        localName = absoluteFilename.right(absoluteFilename.length() - lastSlash - 1);
    QString assetRef = "local://" + localName;
    if (assetRefs.contains(assetRef) && change == IAssetStorage::AssetCreate)
        LogWarning("LocalAssetStorage::EmitAssetChanged: AssetCreate signaled for already existing asset.");
    else
        emit AssetChanged(localName, absoluteFilename, change);
}

void LocalAssetStorage::SetupWatcher()
{
    if (changeWatcher) // Remove the old watcher if one exists.
        RemoveWatcher();

    changeWatcher = new QFileSystemWatcher();

    // Add directory contents to watch list.
    LogDebug("LocalAssetStorage::SetupWatcher: adding " + directory + " recursive=" + (recursive ? "true" : "false"));

    QStringList paths = DirectorySearch(directory, recursive, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    changeWatcher->addPath(QDir::fromNativeSeparators(directory));
    changeWatcher->addPaths(paths);

    LogDebug("Total of " + QString::number(paths.count()) + " dirs and files added to watch list.");
}

void LocalAssetStorage::RemoveWatcher()
{
    SAFE_DELETE(changeWatcher);
}
