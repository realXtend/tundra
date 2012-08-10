// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "IAssetTransfer.h"
#include "IAsset.h"
#include "Profiler.h"

#include "MemoryLeakCheck.h"

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
