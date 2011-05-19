// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "LocalAssetStorage.h"
#include "LocalAssetProvider.h"
#include "AssetAPI.h"

#include <boost/filesystem.hpp>
#include <QFileSystemWatcher>
#include <QDir>
#include <utility>

namespace Asset
{

LocalAssetStorage::LocalAssetStorage()
//:changeWatcher(0)
{
}

LocalAssetStorage::~LocalAssetStorage()
{
    RemoveWatcher();
}

void LocalAssetStorage::LoadAllAssetsOfType(AssetAPI *assetAPI, const QString &suffix, const QString &assetType)
{
    try
    {
        if (recursive)
        {
            boost::filesystem::recursive_directory_iterator iter(directory.toStdString());
            boost::filesystem::recursive_directory_iterator end_iter;
            // Check the subdir
            for(; iter != end_iter; ++iter)
            {
                if (boost::filesystem::is_regular_file(iter->status()))
                {
                    QString str = iter->path().string().c_str();
                    if (suffix == "" || str.endsWith(suffix))
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
            boost::filesystem::directory_iterator iter(directory.toStdString());
            boost::filesystem::directory_iterator end_iter;
            // Check the subdir
            for(; iter != end_iter; ++iter)
            {
                if (boost::filesystem::is_regular_file(iter->status()))
                {
                    QString str = iter->path().string().c_str();
                    if (suffix == "" || str.endsWith(suffix))
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
}

void LocalAssetStorage::RefreshAssetRefs()
{
    assetRefs.clear();
    
    try
    {
        if (recursive)
        {
            boost::filesystem::recursive_directory_iterator iter(directory.toStdString());
            boost::filesystem::recursive_directory_iterator end_iter;
            // Check the subdir
            for(; iter != end_iter; ++iter)
            {
                if (boost::filesystem::is_regular_file(iter->status()))
                {
                    QString str = iter->path().string().c_str();
                    int lastSlash = str.lastIndexOf('/');
                    if (lastSlash != -1)
                        str = str.right(str.length() - lastSlash - 1);
                    assetRefs.append("local://" + str);
                }
            }
        }
        else
        {
            boost::filesystem::directory_iterator iter(directory.toStdString());
            boost::filesystem::directory_iterator end_iter;
            // Check the subdir
            for(; iter != end_iter; ++iter)
            {
                if (boost::filesystem::is_regular_file(iter->status()))
                {
                    QString str = iter->path().string().c_str();
                    int lastSlash = str.lastIndexOf('/');
                    if (lastSlash != -1)
                        str = str.right(str.length() - lastSlash - 1);
                    assetRefs.append("local://" + str);
                }
            }
        }
    }
    catch (...)
    {
    }
    
    emit AssetRefsChanged(this->shared_from_this());
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
        boost::filesystem::recursive_directory_iterator iter(directory.toStdString());
        boost::filesystem::recursive_directory_iterator end_iter;
        // Check the subdir
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

QString LocalAssetStorage::SerializeToString() const
{
    return "type=LocalAssetStorage;name=" + name + ";src=" + directory + ";recursive=" + (recursive ? "true" : "false");
}

void LocalAssetStorage::SetupWatcher()
{
    /* This watcher is not used for now. -jj. will be removed.
    if (changeWatcher) // Remove the old watcher if one exists.
        RemoveWatcher();

    changeWatcher = new QFileSystemWatcher();

    // Add a watcher to listen to if the directory contents change.
    changeWatcher->addPath(directory);

    // Add a watcher to each file in the directory.
    QDir dir(directory);
    QFileInfoList files = dir.entryInfoList(QDir::Files);
    foreach(QFileInfo i, files)
        changeWatcher->addPath(i.absoluteFilePath());

    ///\todo The QFileSystemWatcher is severely lacking in functionality. Replace the above with some custom method that can tell
    /// which files change.
    */
}

void LocalAssetStorage::RemoveWatcher()
{
    /* This watcher is not used for now. -jj. will be removed.
    delete changeWatcher;
    changeWatcher = 0;
    */
}

} // ~Asset

