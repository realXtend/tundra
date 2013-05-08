// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "IAssetTransfer.h"
#include "IAssetProvider.h"
#include "IAsset.h"

#include "Profiler.h"
#include "LoggingFunctions.h"

IAssetTransfer::IAssetTransfer() : 
    cachingAllowed(true),
    diskSourceType(IAsset::Original)
{
}

IAssetTransfer::~IAssetTransfer()
{
}

void IAssetTransfer::EmitAssetDownloaded()
{
    emit Downloaded(this);
}

void IAssetTransfer::EmitTransferSucceeded()
{
    PROFILE(IAssetTransfer_AssetDependenciesCompleted);
    emit Succeeded(this->asset);
}

void IAssetTransfer::EmitAssetFailed(QString reason)
{
    emit Failed(this, reason);
}

bool IAssetTransfer::Abort()
{
    if (provider.lock().get())
        return provider.lock()->AbortTransfer(this);
    else
    {    
        LogWarning("IAssetTransfer::Abort() Provider is null, cannot call IAssetProvider::AbortTransfer()."); 
        return false;
    }
}

void IAssetTransfer::SetCachingBehavior(bool cachingAllowed, QString diskSource)
{
    this->cachingAllowed = cachingAllowed; 
    this->diskSource = diskSource;
}

QString IAssetTransfer::DiskSource() const
{
    return diskSource;
}

IAsset::SourceType IAssetTransfer::DiskSourceType() const
{
    return diskSourceType;
}

bool IAssetTransfer::CachingAllowed() const
{
    return cachingAllowed;
}

QByteArray IAssetTransfer::RawData() const
{
    if (rawAssetData.size() == 0) 
        return QByteArray(); 
    else 
        return QByteArray((const char*)&rawAssetData[0], (int)rawAssetData.size());
}

QString IAssetTransfer::SourceUrl() const
{
    return source.ref;
}

QString IAssetTransfer::AssetType() const
{
    return assetType;
}

AssetPtr IAssetTransfer::Asset() const
{
    return asset;
}
