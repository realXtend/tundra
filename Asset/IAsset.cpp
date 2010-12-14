#include <QCryptographicHash>
#include <QByteArray>
#include <set>

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("IAsset")

#include "IAsset.h"
#include "IAssetTransfer.h"
#include "AssetAPI.h"

IAsset::IAsset(AssetAPI *owner, const QString &type_, const QString &name_)
:assetAPI(owner), type(type_), name(name_)
{
    assert(assetAPI);
}

void IAsset::SetDiskSource(QString diskSource_)
{
    diskSource = diskSource_.trimmed();
}

bool IAsset::LoadFromCache()
{
    bool success = LoadFromFile(DiskSource());
    if (success)
        emit Loaded(shared_from_this()); ///\todo This is not correct. If asset dependencies changed, the new deps won't be requested. Do a new deps request.
    return success;
}

void IAsset::Unload()
{
    DoUnload();
    emit Unloaded(this);
}

bool IAsset::LoadFromFile(QString filename)
{
    filename = filename.trimmed(); ///\todo Sanitate.
    std::vector<u8> fileData;
    bool success = LoadFileToVector(filename.toStdString().c_str(), fileData);
    if (!success)
    {
        LogDebug("LoadFromFile failed for file \"" + filename.toStdString() + "\", could not read file!");
        return false;
    }

    if (fileData.size() == 0)
    {
        LogDebug("LoadFromFile failed for file \"" + filename.toStdString() + "\", file size was 0!");
        return false;
    }

    // Invoke the actual virtual function to load the asset.
    return LoadFromFileInMemory(&fileData[0], fileData.size());
}

bool IAsset::LoadFromFileInMemory(const u8 *data, size_t numBytes)
{
    if (!data || numBytes == 0)
    {
        LogDebug("LoadFromFileInMemory failed for asset \"" + ToString().toStdString() + "\"! No data present!");
        return false;
    }

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

bool IAsset::SerializeTo(std::vector<u8> &data, const QString &serializationParameters)
{
    LogError("IAsset::SerializeTo: Asset serialization not implemented for asset \"" + ToString().toStdString() + "\"!");
    return false;
}

bool IAsset::SaveToFile(const QString &filename, const QString &serializationParameters)
{
    std::vector<u8> data;
    bool success = SerializeTo(data, serializationParameters);
    if (!success || data.size() == 0)
    {
        LogError("IAsset::SaveToFile: SerializeTo returned no data for asset \"" + ToString().toStdString() + "\"!");
        return false;
    }

    return SaveAssetFromMemoryToFile(&data[0], data.size(), filename.toStdString().c_str());
}

bool IAsset::SaveCachedCopyToFile(const QString &filename)
{
    return CopyAssetFile(DiskSource().toStdString().c_str(), filename.toStdString().c_str());
}

void IAsset::SetAssetProvider(AssetProviderPtr provider_)
{
    provider = provider_;
}

void IAsset::SetAssetStorage(AssetStoragePtr storage_)
{
    storage = storage_;
}

void IAsset::SetAssetTransfer(AssetTransferPtr transfer_)
{
    transfer = transfer_;
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

void IAsset::EmitLoaded()
{
    emit Loaded(shared_from_this());

    AssetTransferPtr t = transfer.lock();
    if (t.get())
    {
        assert(t->asset.get() == this);
        t->EmitAssetLoaded();
    }
}
