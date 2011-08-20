// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_HttpAssetTransfer_h
#define incl_Asset_HttpAssetTransfer_h

#include "IAssetTransfer.h"

class HttpAssetTransfer : public IAssetTransfer
{
    Q_OBJECT;
public:

};

typedef boost::shared_ptr<HttpAssetTransfer> HttpAssetTransferPtr;

#endif
