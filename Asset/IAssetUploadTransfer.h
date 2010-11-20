// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_IAssetUploadTransfer_h
#define incl_Asset_IAssetUploadTransfer_h

#include <QObject>
#include "CoreTypes.h"
#include "AssetFwd.h"
#include <boost/weak_ptr.hpp>

class IAssetUploadTransfer : public QObject
{
    Q_OBJECT

public:
    virtual ~IAssetUploadTransfer() {}

    /// Returns the current transfer progress in the range [0, 1].
    virtual float Progress() const { return 0.f; }

    /// Specifies the source file of the upload transfer, or none if this upload does not originate from a file in the system.
    QString sourceFilename;

    /// Specifies the destination name for the asset.
    QString destinationName;

    /// Contains the raw asset data to upload.
    std::vector<u8> assetData;

    boost::weak_ptr<IAssetStorage> destinationStorage;

    boost::weak_ptr<IAssetProvider> destinationProvider;

    void EmitTransferCompleted();
    void EmitTransferFailed();

signals:
    void Completed(IAssetUploadTransfer *transfer);

    void Failed(IAssetUploadTransfer *transfer);
};

#endif
