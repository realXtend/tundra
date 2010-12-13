#include "IAssetTransfer.h"

void IAssetTransfer::EmitAssetDownloaded()
{
    emit Downloaded(this);
}

void IAssetTransfer::EmitAssetDecoded()
{
    emit Decoded(this);
}

void IAssetTransfer::EmitAssetLoaded()
{
    emit Loaded(this);
}

void IAssetTransfer::EmitAssetFailed()
{
    emit Failed(this);
}
