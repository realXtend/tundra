// For conditions of distribution and use, see copyright notice in LICENSE

#include "DebugOperatorNew.h"

#include "AssetRefListener.h"
#include "IAttribute.h"
#include "AssetReference.h"
#include "IComponent.h"
#include "Framework.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "IAssetTransfer.h"
#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

#include <QTimer>

AssetRefListener::AssetRefListener() : 
    myAssetAPI(0), 
    currentWaitingRef("")
{
}

AssetPtr AssetRefListener::Asset() const
{
    return asset.lock();
}

void AssetRefListener::HandleAssetRefChange(IAttribute *assetRef, const QString& assetType)
{
    Attribute<AssetReference> *attr = dynamic_cast<Attribute<AssetReference> *>(assetRef);
    if (!attr)
    {
        LogWarning("AssetRefListener::HandleAssetRefChange: Attribute's type not AssetReference (was " +
            (assetRef == 0 ? "null" : assetRef->TypeName()) + " instead).");
        return;
    }
    HandleAssetRefChange(attr->Owner()->GetFramework()->Asset(), attr->Get().ref, assetType);
}

void AssetRefListener::HandleAssetRefChange(AssetAPI *assetApi, QString assetRef, const QString& assetType)
{
    // Disconnect from any previous transfer we might be listening to
    if (!currentTransfer.expired())
    {
        IAssetTransfer* current = currentTransfer.lock().get();
        current->disconnect(this, SLOT(OnTransferSucceeded(AssetPtr)));
        current->disconnect(this, SLOT(OnTransferFailed(IAssetTransfer*, QString)));
        currentTransfer.reset();
    }
    
    assert(assetApi);

    // Store AssetAPI ptr for later signal hooking.
    if (!myAssetAPI)
        myAssetAPI = assetApi;

    // If the ref is empty, don't go any further as it will just trigger the LogWarning below.
    assetRef = assetRef.trimmed();
    if (assetRef.isEmpty())
        return;
    currentWaitingRef = "";

    // Resolve the protocol for generated:// assets. These assets are never meant to be
    // requested from AssetAPI, they cannot be fetched from anywhere. They can only be either
    // loaded or we must wait for something to load/create them.
    QString protocolPart = "";
    assetApi->ParseAssetRef(assetRef, &protocolPart);
    if (protocolPart.toLower() == "generated")
    {
        AssetPtr loadedAsset = assetApi->GetAsset(assetRef);
        if (loadedAsset.get() && loadedAsset->IsLoaded())
        {
            // Asset is loaded, emit Loaded with 1 msec delay to preserve the logic
            // that HandleAssetRefChange won't emit anything itself as before.
            // Otherwise existing connection can break/be too late after calling this function.
            asset = loadedAsset;
            QTimer::singleShot(1, this, SLOT(EmitLoaded()));
            return;
        }
        else
        {
            // Wait for it to be created.
            currentWaitingRef = assetRef;
            connect(myAssetAPI, SIGNAL(AssetCreated(AssetPtr)), this, SLOT(OnAssetCreated(AssetPtr)), Qt::UniqueConnection);
        }
    }
    else
    {
        // This is not a generated asset, request normally from asset api.
        AssetTransferPtr transfer = assetApi->RequestAsset(assetRef, assetType);
        if (!transfer)
        {
            LogWarning("AssetRefListener::HandleAssetRefChange: Asset request for asset \"" + assetRef + "\" failed.");
            return;
        }
        currentWaitingRef = assetRef;

        connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), this, SLOT(OnTransferSucceeded(AssetPtr)), Qt::UniqueConnection);
        connect(transfer.get(), SIGNAL(Failed(IAssetTransfer*, QString)), this, SLOT(OnTransferFailed(IAssetTransfer*, QString)), Qt::UniqueConnection);
        currentTransfer = transfer;
    }
    
    // Disconnect from the old asset's load signal
    AssetPtr assetData = asset.lock();
    if (assetData)
        disconnect(assetData.get(), SIGNAL(Loaded(AssetPtr)), this, SIGNAL(Loaded(AssetPtr)));
    asset = AssetPtr();
}

void AssetRefListener::OnTransferSucceeded(AssetPtr assetData)
{
    if (!assetData)
        return;
    
    // Connect to further reloads of the asset to be able to notify of them.
    asset = assetData;
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
    /// @todo Remove this logic once a EC_Material + EC_Mesh behaves correctly without failed requests, see generated:// logic in HandleAssetRefChange.
    if (myAssetAPI)
        connect(myAssetAPI, SIGNAL(AssetCreated(AssetPtr)), this, SLOT(OnAssetCreated(AssetPtr)), Qt::UniqueConnection);       
    emit TransferFailed(transfer, reason);
}

void AssetRefListener::OnAssetCreated(AssetPtr assetData)
{
    if (assetData.get() && !currentWaitingRef.isEmpty() && currentWaitingRef == assetData->Name())
    {
        /// @todo Remove this logic once a EC_Material + EC_Mesh behaves correctly without failed requests, see generated:// logic in HandleAssetRefChange.
        /** Log the same message as before for non generated:// refs. This is good to do
            because AssetAPI has now said the request failed, so user might be confused when it still works. */
        if (!currentWaitingRef.toLower().startsWith("generated://"))
            LogInfo("AssetRefListener: Asset \"" + assetData->Name() + "\" was created, applying after it loads.");

        // The asset we are waiting for has been created, hook to the IAsset::Loaded signal.
        currentWaitingRef = "";
        asset = assetData;
        connect(assetData.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnAssetLoaded(AssetPtr)), Qt::UniqueConnection);
        if (myAssetAPI)
            disconnect(myAssetAPI, SIGNAL(AssetCreated(AssetPtr)), this, SLOT(OnAssetCreated(AssetPtr)));
    }
}

void AssetRefListener::EmitLoaded()
{
    AssetPtr currentAsset = asset.lock();
    if (currentAsset.get())
        emit Loaded(currentAsset);
}
