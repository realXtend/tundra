
#include "StableHeaders.h"
#include "XmlRpcEpi.h"

#include "Grid.h"

namespace ProtocolUtilities
{
    // STATIC
    std::string GridParser::ExtractGridAddressFromXMLRPCReply(XmlRpcEpi &call)
    {
        std::string gridUrl = call.GetReply<std::string>("sim_ip");
        if (gridUrl.size() == 0)
            return "";

        int port = call.GetReply<int>("sim_port");
        if (port <= 0 || port >= 65536)
            return "";

        std::stringstream out;
        out << gridUrl << ":" << port;

        return out.str();
    }
}