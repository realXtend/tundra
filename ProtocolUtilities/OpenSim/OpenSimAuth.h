// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenSimAuth_h
#define incl_OpenSimAuth_h

#include <string>

namespace ProtocolUtilities
{
    ///\todo Expand & write this functionality into the whole identification/authentication layer.

    /// @return The MAC address of the network adapter, needed for the OpenSim XML-RPC login.
    std::string GetMACaddressString();

    /// @return The serial number of the first logical drive, needed for the OpenSim XML-RPC login.
    std::string GetId0String();

    /// @return Platform: Win, X11 or Mac. Needed for the OpenSim XML-RPC login.
    std::string GetPlatform();
}

#endif
