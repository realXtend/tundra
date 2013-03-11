// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IAssetTransfer.h"

/// Utility class for identifying HTTP asset transfers for another types of asset transfers.
class HttpAssetTransfer : public IAssetTransfer
{
Q_OBJECT

public:

};

typedef shared_ptr<HttpAssetTransfer> HttpAssetTransferPtr;
