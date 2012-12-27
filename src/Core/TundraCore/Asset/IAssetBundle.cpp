// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "IAssetBundle.h"
#include "IAssetTransfer.h"
#include "LoggingFunctions.h"

IAssetBundle::IAssetBundle(AssetAPI *assetAPI, const QString &type, const QString &name) :
    assetAPI_(assetAPI),
    type_(type),
    name_(name)
{
}

bool IAssetBundle::IsEmpty() const
{
    return !IsLoaded() && diskSource_.isEmpty();
}

QString IAssetBundle::Type() const
{
    return type_;
}

QString IAssetBundle::Name() const
{
    return name_;
}

void IAssetBundle::SetDiskSource(QString diskSource)
{
    diskSource_ = diskSource;
}

QString IAssetBundle::DiskSource() const
{
    return diskSource_;
}

void IAssetBundle::Unload()
{
    DoUnload();
    emit Unloaded(this);
}

// AssetBundleMonitor

AssetBundleMonitor::AssetBundleMonitor(AssetAPI *owner, AssetTransferPtr bundleTransfer) :
    assetAPI_(owner),
    bundleTransfer_(bundleTransfer),
    bundleRef_(bundleTransfer->source.ref)
{
    connect(bundleTransfer_.get(), SIGNAL(Failed(IAssetTransfer*, QString)), this, SLOT(BundleFailed()), Qt::UniqueConnection);
}

AssetBundleMonitor::~AssetBundleMonitor()
{
    childTransfers_.clear();
    bundleTransfer_.reset();
}

void AssetBundleMonitor::AddSubAssetTransfer(AssetTransferPtr transfer)
{
    childTransfers_.push_back(transfer);
}

AssetTransferPtr AssetBundleMonitor::SubAssetTransfer(const QString &fullSubAssetRef)
{
    for(std::vector<AssetTransferPtr>::iterator subTransferIter=childTransfers_.begin(); subTransferIter!=childTransfers_.end(); ++subTransferIter)
        if ((*subTransferIter)->source.ref == fullSubAssetRef)
            return (*subTransferIter);
    return AssetTransferPtr();
}

std::vector<AssetTransferPtr> AssetBundleMonitor::SubAssetTransfers()
{
    return childTransfers_;
}

AssetTransferPtr AssetBundleMonitor::BundleTransfer()
{
    return bundleTransfer_;
}

QString AssetBundleMonitor::BundleAssetRef()
{
    return bundleRef_;
}

void AssetBundleMonitor::BundleFailed()
{
    for(std::vector<AssetTransferPtr>::iterator subTransferIter=childTransfers_.begin(); subTransferIter!=childTransfers_.end(); ++subTransferIter)
    {
        AssetTransferPtr transfer = (*subTransferIter);
        if (transfer.get())
        {
            QString error = "Failed to load parent asset bundle \"" + bundleRef_ + "\" for sub asset \"" + transfer->source.ref + "\".";
            transfer->EmitAssetFailed(error);
            LogError("AssetBundleMonitor: " + error);
        }
    }
}
