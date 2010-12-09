#include <QCryptographicHash>
#include <QByteArray>
#include <set>

#include "IAsset.h"
#include "AssetAPI.h"

IAsset::IAsset(AssetAPI *owner, const QString &type_, const QString &name_)
:assetAPI(owner), type(type_), name(name_)
{
    assert(assetAPI);
}

void IAsset::SetCacheFile(QString cacheFile_)
{
    cacheFile = cacheFile_.trimmed();
}

bool IAsset::LoadFromCache()
{
    return LoadFromFile(CacheFile());
}

bool IAsset::LoadFromFile(QString filename)
{
    filename = filename.trimmed(); ///\todo Sanitate.
    std::vector<u8> fileData;
    bool success = LoadFileToVector(filename.toStdString().c_str(), fileData);
    if (!success)
        return false; ///\todo Log warning.

    if (fileData.size() == 0)
        return false; ///\todo Log warning.

    // Invoke the actual virtual function to load the asset.
    return LoadFromFileInMemory(&fileData[0], fileData.size());
}

bool IAsset::LoadFromFileInMemory(const u8 *data, size_t numBytes)
{
    if (!data || numBytes == 0)
        return false; ///\todo Log out warning.

    // Before loading the asset, recompute the content hash for the asset data.
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData((const char*)data, numBytes);
    QByteArray result = hash.result().toHex();

    contentHash = QString(result);

    return DeserializeFromData(data, numBytes);
}

std::vector<AssetReference> IAsset::FindReferencesRecursive() const
{
    std::set<AssetReference> refs;

    std::vector<AssetReference> unwalkedRefs = FindReferences();
    while(unwalkedRefs.size() > 0)
    {
        AssetReference ref = unwalkedRefs.back();
        unwalkedRefs.pop_back();
        if (refs.find(ref) == refs.end())
        {
            refs.insert(ref);
            AssetPtr asset = assetAPI->GetAsset(ref.ref);
            if (asset.get())
            {
                std::vector<AssetReference> newRefs = asset->FindReferences();
                unwalkedRefs.insert(unwalkedRefs.end(), newRefs.begin(), newRefs.end());                
            }
        }
    }

    std::vector<AssetReference> finalRefs(refs.begin(), refs.end());
    return finalRefs;
}

bool IAsset::SaveToFile(const QString &filename)
{
    ///\todo Log out warning.
    return false;
}

bool IAsset::SaveCachedCopyToFile(const QString &filename)
{
    return CopyAssetFile(CacheFile().toStdString().c_str(), filename.toStdString().c_str());
}

void IAsset::SetAssetProvider(AssetProviderPtr provider_)
{
    provider = provider_;
}

void IAsset::SetAssetStorage(AssetStoragePtr storage_)
{
    storage = storage_;
}

AssetStoragePtr IAsset::GetAssetStorage()
{
    return storage.lock();
}

AssetProviderPtr IAsset::GetAssetProvider()
{
    return provider.lock();
}

QString IAsset::ToString() const
{ 
    return (Name().isEmpty() ? "(noname)" : Name()) + " (" + (Type().isEmpty() ? "notype" : Type()) + ")";
}
