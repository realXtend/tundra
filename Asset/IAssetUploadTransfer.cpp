// For conditions of distribution and use, see copyright notice in license.txt

#include "DebugOperatorNew.h"
#include <QList>
#include "MemoryLeakCheck.h"
#include "IAssetUploadTransfer.h"

void IAssetUploadTransfer::EmitTransferCompleted()
{
    emit Completed(this);
}

void IAssetUploadTransfer::EmitTransferFailed()
{
    emit Failed(this);
}
