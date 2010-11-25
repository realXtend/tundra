#include "AssetRefListener.h"
#include "IAttribute.h"
#include "AssetReference.h"
#include "IComponent.h"
#include "Framework.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"

void AssetRefListener::HandleAssetRefChange(IAttribute *assetRef)
{
    Attribute<AssetReference> *attr = dynamic_cast<Attribute<AssetReference> *>(assetRef);
    if (!attr)
        return; ///\todo Log out warning.

    IAssetTransfer *transfer = attr->GetOwner()->GetFramework()->Asset()->RequestAsset(attr->Get().ref);
    if (!transfer)
        return; ///\todo Log out warning.

    connect(transfer, SIGNAL(Downloaded(IAssetTransfer*)), this, SLOT(EmitDownloaded(IAssetTransfer*)), Qt::UniqueConnection);
    connect(transfer, SIGNAL(Decoded(IAssetTransfer*)), this, SLOT(EmitDecoded(IAssetTransfer*)), Qt::UniqueConnection);
    connect(transfer, SIGNAL(Loaded(IAssetTransfer*)), this, SLOT(EmitLoaded(IAssetTransfer*)), Qt::UniqueConnection);
}

void AssetRefListener::EmitDownloaded(IAssetTransfer *transfer)
{
    emit Downloaded(transfer);
}

void AssetRefListener::EmitDecoded(IAssetTransfer *transfer)
{
    emit Decoded(transfer);
}

void AssetRefListener::EmitLoaded(IAssetTransfer *transfer)
{
    emit Loaded(transfer);
}

//void AssetRefListener::HandleAssetSourceChange(