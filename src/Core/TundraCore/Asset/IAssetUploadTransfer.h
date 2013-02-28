// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreTypes.h"
#include "AssetFwd.h"
#include "IAssetStorage.h"

#include <QObject>
#include <QByteArray>

/// Represents a currently ongoing asset upload operation.
class TUNDRACORE_API IAssetUploadTransfer : public QObject, public enable_shared_from_this<IAssetUploadTransfer>
{
    Q_OBJECT

public:
    virtual ~IAssetUploadTransfer() {}

    /// Returns the current transfer progress in the range [0, 1].
    virtual float Progress() const { return 0.f; }

    /// Specifies the source file of the upload transfer, or none if this upload does not originate from a file in the system.
    QString sourceFilename;

    /// Contains the raw asset data to upload. If sourceFilename=="", the data is taken from this array instead.
    std::vector<u8> assetData;

    /// Contains the reply from the storage if one was provided. Eg. HTTP PUT/POST may give response data in the body.
    QByteArray replyData;
    
    /// Specifies the destination name for the asset.
    QString destinationName;

    weak_ptr<IAssetStorage> destinationStorage;

    AssetProviderWeakPtr destinationProvider;

    void EmitTransferCompleted();
    void EmitTransferFailed();

public slots:
    /// Returns the full assetRef address this asset will have when the upload is complete.
    QString AssetRef()
    { 
        shared_ptr<IAssetStorage> storage = destinationStorage.lock();
        if (!storage)
            return "";
        return storage->GetFullAssetURL(destinationName);
    }

    /// Returns a copy of the raw asset data in this upload.
    QByteArray RawData() const
    { 
        if (assetData.size() == 0)
            return QByteArray();
        else
            return QByteArray((const char*)&assetData[0], assetData.size());
    }

    QByteArray RawReplyData() const { return replyData; }

    QString SourceFilename() const { return sourceFilename; }
    QString DestinationName() const { return destinationName; }

signals:
    void Completed(IAssetUploadTransfer *transfer);

    void Failed(IAssetUploadTransfer *transfer);
};

