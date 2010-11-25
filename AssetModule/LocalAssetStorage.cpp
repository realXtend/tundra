// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "LocalAssetStorage.h"
#include "LocalAssetProvider.h"

#include <QFileSystemWatcher>
#include <QDir>
#include <utility>

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

QString LocalAssetStorage::GetFullPathForAsset(const QString &assetname, bool recursiveLookup)
{
    if (boost::filesystem::exists((GuaranteeTrailingSlash(directory) + assetname).toStdString()))
        return directory;

    if (!recursive || !recursiveLookup)
        return "";

    try
    {
        boost::filesystem::recursive_directory_iterator iter(directory.toStdString());
        boost::filesystem::recursive_directory_iterator end_iter;
        // Check the subdir
        for(; iter != end_iter; ++iter)
            if (!fs::is_regular_file(iter->status()) && boost::filesystem::exists((GuaranteeTrailingSlash(iter->path().string().c_str()) + assetname).toStdString()))
                return iter->path().string().c_str();
    }
    catch (...)
    {
    }

    return "";
}

QString LocalAssetStorage::GetFullAssetURL(const QString &localName)
{    
    using namespace std;

    QString s = localName.trimmed();
    int end = 0;
    end = max(end, s.lastIndexOf('/')+1);
    end = max(end, s.lastIndexOf('\\')+1);
    return BaseURL() + localName.mid(end);
}

void LocalAssetStorage::SetupWatcher()
{
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
}

void LocalAssetStorage::RemoveWatcher()
{
    delete changeWatcher;
    changeWatcher = 0;
}

} // ~Asset

