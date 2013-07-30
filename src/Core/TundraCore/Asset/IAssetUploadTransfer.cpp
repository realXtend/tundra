// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "IAssetUploadTransfer.h"

#include "MemoryLeakCheck.h"

IAssetUploadTransfer::~IAssetUploadTransfer()
{
}

void IAssetUploadTransfer::EmitTransferCompleted()
{
    emit Completed(this);
}

void IAssetUploadTransfer::EmitTransferFailed()
{
    emit Failed(this);
}

QString IAssetUploadTransfer::AssetRef()
{
    shared_ptr<IAssetStorage> storage = destinationStorage.lock();
    if (!storage)
        return "";
    return storage->GetFullAssetURL(destinationName);
}

QByteArray IAssetUploadTransfer::RawData() const
{
    if (assetData.size() == 0)
        return QByteArray();
    else
        return QByteArray((const char*)&assetData[0], (int)assetData.size());
}

QString IAssetUploadTransfer::ReplyHeader(const QString &header) const
{
    foreach(const QString &name, replyHeaders.keys())
        if (name.compare(header, Qt::CaseInsensitive) == 0)
            return replyHeaders[name];
    return "";
}

QString IAssetUploadTransfer::SourceFilename() const
{
    return sourceFilename;
}

QString IAssetUploadTransfer::DestinationName() const
{
    return destinationName;
}
