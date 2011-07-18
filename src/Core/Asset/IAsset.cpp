#include "DebugOperatorNew.h"
#include <QList>
#include <boost/thread.hpp>
#include <QByteArray>
#include <set>

#include "LoggingFunctions.h"

#include "IAsset.h"
#include "AssetAPI.h"
#include "MemoryLeakCheck.h"

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
    // If asset did not have dependencies, this causes Loaded() to be emitted
    bool success = LoadFromFile(DiskSource());
    if (!success)
        return false;

    AssetPtr thisAsset = shared_from_this();

    if (assetAPI->NumPendingDependencies(thisAsset) > 0)
        assetAPI->RequestAssetDependencies(thisAsset);

    return success;
}

void IAsset::Unload()
{
//    LogDebug("IAsset::Unload called for asset \"" + name.toStdString() + "\".");
    DoUnload();
    emit Unloaded(this);
}

bool IAsset::IsEmpty() const
{
    return !IsLoaded() && diskSource.isEmpty();
}

AssetPtr IAsset::Clone(QString newAssetName) const
{
    assert(assetAPI);
    if (!IsLoaded())
        return AssetPtr();

    AssetPtr existing = assetAPI->GetAsset(newAssetName);
    if (existing)
    {
        LogError("Cannot Clone() asset \"" + Name() + "\" to a new asset \"" + newAssetName + "\": An asset with that name already exists!");
        return AssetPtr();
    }

    std::vector<u8> data;
    bool success = SerializeTo(data);
    if (!success)
    {
        LogError("Cannot Clone() asset \"" + Name() + "\" to a new asset \"" + newAssetName + "\": Serializing the asset failed!");
        return AssetPtr();
    }
    if (data.size() == 0)
    {
        LogError("Cannot Clone() asset \"" + Name() + "\" to a new asset \"" + newAssetName + "\": Asset serialization succeeded with zero size!");
        return AssetPtr();
    }

    AssetPtr newAsset = assetAPI->CreateNewAsset(this->Type(), newAssetName);
    if (!newAsset)
    {
        LogError("Cannot Clone() asset \"" + Name() + "\" to a new asset \"" + newAssetName + "\": AssetAPI::CreateNewAsset failed!");
        return AssetPtr();
    }

    success = newAsset->LoadFromFileInMemory(&data[0], data.size());
    if (!success)
    {
        LogError("Cannot Clone() asset \"" + Name() + "\" to a new asset \"" + newAssetName + "\": Deserializing the new asset from bytes failed!");
        assetAPI->ForgetAsset(newAsset, false);
        return AssetPtr();
    }

    return newAsset;
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

    bool success = DeserializeFromData(data, numBytes);
    
    AssetPtr thisAsset = this->shared_from_this();
    // If asset was loaded successfully, and there are no pending dependencies, emit Loaded() now
    if (success && assetAPI->NumPendingDependencies(thisAsset) == 0)
        emit Loaded(thisAsset);
    return success;
}

void IAsset::DependencyLoaded(AssetPtr dependee)
{
    AssetPtr thisAsset = this->shared_from_this();
    // If we are loaded, and this was the last dependency, emit Loaded()
    if (IsLoaded() && assetAPI->NumPendingDependencies(thisAsset) == 0)
        emit Loaded(thisAsset);
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
            if (asset)
            {
                std::vector<AssetReference> newRefs = asset->FindReferences();
                unwalkedRefs.insert(unwalkedRefs.end(), newRefs.begin(), newRefs.end());                
            }
        }
    }

    std::vector<AssetReference> finalRefs(refs.begin(), refs.end());
    return finalRefs;
}

bool IAsset::SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const
{
    LogError("IAsset::SerializeTo: Asset serialization not implemented for asset \"" + ToString().toStdString() + "\"!");
    return false;
}

bool IAsset::SaveToFile(const QString &filename, const QString &serializationParameters) const
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
