// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "LocalAssetStorage.h"

namespace Asset
{

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

} // ~Asset

