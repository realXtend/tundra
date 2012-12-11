// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "IAssetUploadTransfer.h"

#include "MemoryLeakCheck.h"

void IAssetUploadTransfer::EmitTransferCompleted()
{
    emit Completed(this);
}

void IAssetUploadTransfer::EmitTransferFailed()
{
    emit Failed(this);
}
