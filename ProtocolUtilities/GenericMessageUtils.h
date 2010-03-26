// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Protocol_GenericMessageUtils_h
#define incl_Protocol_GenericMessageUtils_h

#include "NetworkMessages/NetInMessage.h"

namespace ProtocolUtilities
{
    //! parse method from generic message packet
    std::string ParseGenericMessageMethod(NetInMessage& message);
    
    //! parse parameter list from generic message packet
    StringVector ParseGenericMessageParameters(NetInMessage& message);
}

#endif 