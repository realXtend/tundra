// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "LocalAssetStorage.h"
#include "LocalAssetProvider.h"
#include "AssetAPI.h"
#include "QtUtils.h"
#include "Profiler.h"

#include <boost/filesystem.hpp>
#include <QFileSystemWatcher>
#include <QDir>
#include <utility>

#include "MemoryLeakCheck.h"

LocalAssetStorage::LocalAssetStorage() :
    recursive(true),
    writable(true),
    liveUpdate(true),
    autoDiscoverable(true)
//    changeWatcher(0)
{
}

LocalAssetStorage::~LocalAssetStorage()
{
//    RemoveWatcher();
}

void LocalAssetStorage::LoadAllAssetsOfType(AssetAPI *assetAPI, const QString &suffix, const QString &assetType)
{
    ///\todo Profile which ones are better: Boost's or Qt's directory iterators. Preliminary tests by me (Stinkfist) show that Qt's are better.
    foreach(QString str, DirectorySearch(directory, recursive, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks))
        if ((suffix == "" || str.endsWith(suffix)) && !(str.contains(".git") || str.contains(".svn") || str.contains(".hg")))
        {
            int lastSlash = str.lastIndexOf('/');
            if (lastSlash != -1)
                str = str.right(str.length() - lastSlash - 1);
            assetAPI->RequestAsset("local://" + str, assetType);
        }
/*
    try
    {
        if (recursive)
        {
            boost::filesystem::recursive_directory_iterator iter(directory.toStdString()), end_iter;
            for(; iter != end_iter; ++iter)
            {
                if (boost::filesystem::is_regular_file(iter->status()))
                {
                    QString str = iter->path().string().c_str();
                    if ((suffix == "" || str.endsWith(suffix)) && !(str.contains(".git") || str.contains(".svn") || str.contains(".hg")))
                    {
                        int lastSlash = str.lastIndexOf('/');
                        if (lastSlash != -1)
                            str = str.right(str.length() - lastSlash - 1);
                        assetAPI->RequestAsset("local://" + str, assetType);
                    }
                }
            }
        }
        else
        {
            boost::filesystem::directory_iterator iter(directory.toStdString()), end_iter;
            for(; iter != end_iter; ++iter)
            {
                if (boost::filesystem::is_regular_file(iter->status()))
                {
                    QString str = iter->path().string().c_str();
                    if ((suffix == "" || str.endsWith(suffix)) && !(str.contains(".git") || str.contains(".svn") || str.contains(".hg")))
                    {
                        int lastSlash = str.lastIndexOf('/');
                        if (lastSlash != -1)
                            str = str.right(str.length() - lastSlash - 1);
                        assetAPI->RequestAsset("local://" + str, assetType);
                    }
                }
            }
        }
    }
    catch (...)
    {
    }
*/
}

void LocalAssetStorage::RefreshAssetRefs()
{
    ///\todo Profile which ones are better: Boost's or Qt's directory iterators. Preliminary tests by me (Stinkfist) show that Qt's are better.
    PROFILE(LocalAssetStorage_RefreshAssetRefs)
    {
        PROFILE(LocalAssetStorage_RefreshAssetRefs_QtDirectorySearch)
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

/*
    assetRefs.clear();
    PROFILE(LocalAssetStorage_RefreshAssetRefs_boost)
    {
        try
        {
            if (recursive)
            {
                boost::filesystem::recursive_directory_iterator iter(directory.toStdString()), end_iter;
                for(; iter != end_iter; ++iter)
                {
                    if (boost::filesystem::is_regular_file(iter->status()))
                    {
                        QString str = iter->path().string().c_str();
                        if (!(str.contains(".git") || str.contains(".svn") || str.contains(".hg")))
                        {
                            int lastSlash = str.lastIndexOf('/');
                            if (lastSlash != -1)
                                str = str.right(str.length() - lastSlash - 1);
                            assetRefs.append("local://" + str);
                        }
                    }
                }
            }
            else
            {
                boost::filesystem::directory_iterator iter(directory.toStdString()), end_iter;
                for(; iter != end_iter; ++iter)
                {
                    if (boost::filesystem::is_regular_file(iter->status()))
                    {
                        QString str = iter->path().string().c_str();
                        if (!(str.contains(".git") || str.contains(".svn") || str.contains(".hg")))
                        {
                            int lastSlash = str.lastIndexOf('/');
                            if (lastSlash != -1)
                                str = str.right(str.length() - lastSlash - 1);
                            assetRefs.append("local://" + str);
                        }
                    }
                }
            }
        }
        catch (...)
        {
        }
    }

    emit AssetRefsChanged(this->shared_from_this());
*/
}

QString LocalAssetStorage::GetFullPathForAsset(const QString &assetname, bool recursiveLookup)
{
    QDir dir(GuaranteeTrailingSlash(directory) + assetname);
    if (boost::filesystem::exists(dir.absolutePath().toStdString()))
        return directory;

    if (!recursive || !recursiveLookup)
        return "";

    try
    {
        boost::filesystem::recursive_directory_iterator iter(directory.toStdString()), end_iter;
        for(; iter != end_iter; ++iter)
        {
            QDir dir(GuaranteeTrailingSlash(iter->path().string().c_str()) + assetname);
            if (!boost::filesystem::is_regular_file(iter->status()) && boost::filesystem::exists(dir.absolutePath().toStdString()))
                return iter->path().string().c_str();
        }
    }
    catch(...)
    {
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

QString LocalAssetStorage::SerializeToString() const
{
    return "type=" + Type() + ";name=" + name + ";src=" + directory + ";recursive=" + (recursive ? "true" : "false") + ";readonly=" + (!writable ? "true" : "false") +
        ";liveupdate=" + (liveUpdate ? "true" : "false") + ";autodiscoverable=" + (autoDiscoverable ? "true" : "false");
}

/*
void LocalAssetStorage::SetupWatcher()
{
    if (changeWatcher) // Remove the old watcher if one exists.
        RemoveWatcher();

    changeWatcher = new QFileSystemWatcher();

    // Add directory contents to watch list..
    LogDebug("LocalAssetStorage::SetupWatcher: adding " + directory + " recursive=" + (recursive ? "true" : "false"));

    QStringList paths = DirectorySearch(directory, recursive, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    changeWatcher->addPaths(paths);

    LogDebug("Total of " + QString::number(paths.count()) + " dirs and files added to watch list.");
}

void LocalAssetStorage::RemoveWatcher()
{
    SAFE_DELETE(changeWatcher);
}
*/
