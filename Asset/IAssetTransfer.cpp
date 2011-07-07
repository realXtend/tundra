#include "IAssetTransfer.h"
#include "IAsset.h"

void IAssetTransfer::EmitAssetDownloaded()
{
    emit Downloaded(this);
}

void IAssetTransfer::EmitTransferSucceeded()
{
    emit Succeeded(this->asset);
}

void IAssetTransfer::EmitAssetFailed(QString reason)
{
    emit Failed(this, reason);
}
