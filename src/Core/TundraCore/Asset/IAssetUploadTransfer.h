// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreTypes.h"
#include "AssetFwd.h"
#include "IAssetStorage.h"

#include <QObject>
#include <QByteArray>
#include <QHash>

/// Represents a currently ongoing asset upload operation.
class TUNDRACORE_API IAssetUploadTransfer : public QObject, public enable_shared_from_this<IAssetUploadTransfer>
{
    Q_OBJECT

public:
    virtual ~IAssetUploadTransfer();

    /// Returns the current transfer progress in the range [0, 1].
    virtual float Progress() const { return 0.f; }

    /// Specifies the source file of the upload transfer, or none if this upload does not originate from a file in the system.
    QString sourceFilename;

    /// Contains the raw asset data to upload. If sourceFilename=="", the data is taken from this array instead.
    std::vector<u8> assetData;

    /// Contains the reply from the storage if one was provided. Eg. HTTP PUT/POST may give response data in the body.
    QByteArray replyData;

    /// Headers returned by the upload.
    QHash<QString, QString> replyHeaders;

    /// Specifies the destination name for the asset.
    QString destinationName;

    /// Destination storage.
    weak_ptr<IAssetStorage> destinationStorage;

    /// Destination provider.
    AssetProviderWeakPtr destinationProvider;

    /// Emits Completed signal.
    void EmitTransferCompleted();

    /// Emits Failed signal.
    void EmitTransferFailed();

public slots:
    /// Returns the full assetRef address this asset will have when the upload is complete.
    QString AssetRef();

    /// Returns a copy of the raw asset data in this upload.
    QByteArray RawData() const;

    /// Returns the raw reply data returned by this upload.
    QByteArray RawReplyData() const { return replyData; }

    /// Returns a header value for the passed in header name. The name check is case insensitive.
    /** @note Headers can be empty or can have values for certain providers like HTTP. 
        @param Header name.
        @return Header value or empty string if not found. */
    QString ReplyHeader(const QString &header) const;

    /// @copydoc sourceFilename
    QString SourceFilename() const;

    /// @copydoc destinationName
    QString DestinationName() const;

signals:
    /// Emitted when upload completes successfully.
    void Completed(IAssetUploadTransfer *transfer);

    /// Emitted when upload fails.
    void Failed(IAssetUploadTransfer *transfer);
};
