#include "DebugOperatorNew.h"
#include <QList>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>

#include "MemoryLeakCheck.h"
#include "AssetRefListener.h"
#include "IAttribute.h"
#include "AssetReference.h"
#include "IComponent.h"
#include "Framework.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "IAssetTransfer.h"

AssetPtr AssetRefListener::Asset()
{
    return asset.lock();
}

void AssetRefListener::HandleAssetRefChange(IAttribute *assetRef, const QString& assetType)
{
    Attribute<AssetReference> *attr = dynamic_cast<Attribute<AssetReference> *>(assetRef);
    if (!attr)
        return; ///\todo Log out warning.

    HandleAssetRefChange(attr->GetOwner()->GetFramework()->Asset(), attr->Get().ref, assetType);
}

void AssetRefListener::HandleAssetRefChange(AssetAPI *assetApi, QString assetRef, const QString& assetType)
{
    assert(assetApi);

    assetRef = assetRef.trimmed();

    AssetTransferPtr transfer = assetApi->RequestAsset(assetRef, assetType);
    if (!transfer)
        return; ///\todo Log out warning.

    connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), this, SLOT(OnTransferSucceeded(AssetPtr)), Qt::UniqueConnection);
    connect(transfer.get(), SIGNAL(Failed(IAssetTransfer*, QString)), this, SLOT(OnTransferFailed(IAssetTransfer*, QString)), Qt::UniqueConnection);

    // Disconnect from the old asset's load signal
    AssetPtr assetData = asset.lock();
    if (assetData)
        disconnect(assetData.get(), SIGNAL(Loaded(AssetPtr)), this, SIGNAL(Loaded(AssetPtr)));
    asset = AssetPtr();
}

void AssetRefListener::OnTransferSucceeded(AssetPtr assetData)
{
    assert(assetData);
    if (!assetData)
        return;
    
    asset = assetData;
    
    // Connect to further reloads of the asset to be able to notify of them.
    connect(assetData.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnAssetLoaded(AssetPtr)), Qt::UniqueConnection);
    
    emit Loaded(assetData);
}

void AssetRefListener::OnAssetLoaded(AssetPtr assetData)
{
    if (assetData == asset.lock())
        emit Loaded(assetData);
}

void AssetRefListener::OnTransferFailed(IAssetTransfer* transfer, QString reason)
{
    emit TransferFailed(transfer, reason);
}
