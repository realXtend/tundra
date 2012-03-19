#include "IAssetTransfer.h"
#include "IAsset.h"
#include "Profiler.h"

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
