// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "LocalAssetStorage.h"

#include <QFileSystemWatcher>
#include <QDir>

namespace Asset
{

LocalAssetStorage::LocalAssetStorage()
:changeWatcher(0)
{
}

LocalAssetStorage::~LocalAssetStorage()
{
    RemoveWatcher();
}

std::string LocalAssetStorage::GetFullPathForAsset(const std::string &assetname, bool recursiveLookup)
{
    if (boost::filesystem::exists(directory + "/" + assetname))
        return directory;

    if (!recursive || !recursiveLookup)
        return "";

    try
    {
        boost::filesystem::recursive_directory_iterator iter(directory);
        boost::filesystem::recursive_directory_iterator end_iter;
        // Check the subdir
        for(; iter != end_iter; ++iter)
            if (!fs::is_regular_file(iter->status()) && boost::filesystem::exists(iter->path().string() + "/" + assetname))
                return iter->path().string();
    }
    catch (...)
    {
    }

    return "";
}

void LocalAssetStorage::SetupWatcher()
{
    if (changeWatcher) // Remove the old watcher if one exists.
        RemoveWatcher();

    changeWatcher = new QFileSystemWatcher();

    // Add a watcher to listen to if the directory contents change.
    changeWatcher->addPath(directory.c_str());

    // Add a watcher to each file in the directory.
    QDir dir(directory.c_str());
    QFileInfoList files = dir.entryInfoList(QDir::Files);
    foreach(QFileInfo i, files)
        changeWatcher->addPath(i.absoluteFilePath());

    ///\todo The QFileSystemWatcher is severely lacking in functionality. Replace the above with some custom method that can tell
    /// which files change.
}

void LocalAssetStorage::RemoveWatcher()
{
    delete changeWatcher;
    changeWatcher = 0;
}

} // ~Asset

