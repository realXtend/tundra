#include "IAssetTransfer.h"
#include "IAsset.h"

void IAssetTransfer::EmitAssetDownloaded()
{
    emit Downloaded(this);
}

void IAssetTransfer::EmitAssetDecoded()
{
//    if (this->asset.get())
//        emit Decoded(this->asset);
}

void IAssetTransfer::EmitAssetLoaded()
{
    emit Loaded(this->asset);
    /*
    if (this->asset.get())
    {
        this->asset->EmitLoaded();
    }
    */
}

void IAssetTransfer::EmitAssetFailed()
{
    emit Failed(this);
}
