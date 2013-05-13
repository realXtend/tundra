// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "IAsset.h"
#include "AssetAPI.h"

#include "Profiler.h"
#include "LoggingFunctions.h"

#include <set>

#include "MemoryLeakCheck.h"

IAsset::IAsset(AssetAPI *owner, const QString &type_, const QString &name_)
:assetAPI(owner), type(type_), name(name_), diskSourceType(Programmatic), modified(false)
{
    assert(assetAPI);
}

void IAsset::SetDiskSource(const QString &diskSource_)
{
    diskSource = diskSource_.trimmed();
    emit PropertyStatusChanged(this);
}

void IAsset::SetDiskSourceType(SourceType type)
{
    if (diskSourceType != type)
    {
        diskSourceType = type;
        emit PropertyStatusChanged(this);
    }
}

bool IAsset::LoadFromCache()
{
    // If asset did not have dependencies, this causes Loaded() to be emitted
    bool success = LoadFromFile(DiskSource());
    if (!success)
        return false;

    AssetPtr thisAsset = shared_from_this();

    if (assetAPI->HasPendingDependencies(thisAsset))
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

bool IAsset::IsTrusted() const
{
    AssetStoragePtr storage = GetAssetStorage();
    if (!storage)
    {
        AssetAPI::AssetRefType type = AssetAPI::ParseAssetRef(Name());
        return type == AssetAPI::AssetRefLocalPath || type == AssetAPI::AssetRefLocalUrl;
    }
    return storage->Trusted();
}

void IAsset::MarkModified()
{
    if (!modified)
    {
        modified = true;
        emit PropertyStatusChanged(this);
    }
}

void IAsset::ClearModified()
{
    if (modified)
    {
        modified = false;
        emit PropertyStatusChanged(this);
    }
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

    // Do not allow asynchronous loading due the caller of this 
    // expects the asset to be usable when this function returns.
    success = newAsset->LoadFromFileInMemory(&data[0], data.size(), false);
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
    PROFILE(IAsset_LoadFromFile);
    filename = filename.trimmed(); ///\todo Sanitate.
    if (filename.isEmpty())   
    {
        LogDebug("LoadFromFile failed for asset \"" + Name() + "\", given file path is empty!");
        return false;
    }

    std::vector<u8> fileData;
    bool success = LoadFileToVector(filename, fileData);
    if (!success)
    {
        LogDebug("LoadFromFile failed for file \"" + filename + "\", could not read file!");
        return false;
    }

    if (fileData.size() == 0)
    {
        LogDebug("LoadFromFile failed for file \"" + filename + "\", file size was 0!");
        return false;
    }

    // Invoke the actual virtual function to load the asset.
    // Do not allow asynchronous loading due the caller of this 
    // expects the asset to be usable when this function returns.
    return LoadFromFileInMemory(&fileData[0], fileData.size(), false);
}

bool IAsset::LoadFromFileInMemory(const u8 *data, size_t numBytes, bool allowAsynchronous)
{
    PROFILE(IAsset_LoadFromFileInMemory);
    if (!data || numBytes == 0)
    {
        LogDebug("LoadFromFileInMemory failed for asset \"" + ToString() + "\"! No data present!");
        return false;
    }

    return DeserializeFromData(data, numBytes, allowAsynchronous);
}

void IAsset::DependencyLoaded(AssetPtr dependee)
{
    // If we are loaded, and this was the last dependency, emit Loaded().
    // No need to have exact duplicate code here even if LoadCompleted is not the most 
    // informative name in the world for the situation.
    LoadCompleted();
}

void IAsset::LoadCompleted()
{
    PROFILE(IAsset_LoadCompleted);
    // If asset was loaded successfully, and there are no pending dependencies, emit Loaded() now.
    AssetPtr thisAsset = this->shared_from_this();
    if (IsLoaded() && !assetAPI->HasPendingDependencies(thisAsset))
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

bool IAsset::SerializeTo(std::vector<u8> &/*data*/, const QString &/*serializationParameters*/) const
{
    LogError("IAsset::SerializeTo: Asset serialization not implemented for asset \"" + ToString() + "\"!");
    return false;
}

bool IAsset::SaveToFile(const QString &filename, const QString &serializationParameters) const
{
    std::vector<u8> data;
    bool success = SerializeTo(data, serializationParameters);
    if (!success || data.size() == 0)
    {
        LogError("IAsset::SaveToFile: SerializeTo returned no data for asset \"" + ToString() + "\"!");
        return false;
    }

    return SaveAssetFromMemoryToFile(&data[0], data.size(), filename);
}

bool IAsset::SaveCachedCopyToFile(const QString &filename)
{
    return CopyAssetFile(DiskSource(), filename);
}

void IAsset::SetAssetProvider(AssetProviderPtr provider_)
{
    provider = provider_;
}

void IAsset::SetAssetStorage(AssetStoragePtr storage_) 
{
    storage = storage_;
}

AssetStoragePtr IAsset::AssetStorage() const
{
    return storage.lock();
}

AssetProviderPtr IAsset::AssetProvider() const
{
    return provider.lock();
}

QString IAsset::ToString() const
{ 
    return (Name().isEmpty() ? "(noname)" : Name()) + " (" + (Type().isEmpty() ? "notype" : Type()) + ")";
}

QByteArray IAsset::RawData(const QString serializationParameters) const
{ 
    std::vector<u8> data; 
    if (SerializeTo(data, serializationParameters) && data.size() > 0) 
        return QByteArray((const char*)&data[0], (int)data.size()); 
    else 
        return QByteArray();
}

