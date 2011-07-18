// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IAssetTransfer.h"

class HttpAssetTransfer : public IAssetTransfer
{
    Q_OBJECT;
public:

};

typedef boost::shared_ptr<HttpAssetTransfer> HttpAssetTransferPtr;

