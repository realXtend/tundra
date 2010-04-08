
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

        int region_udp_port = call.GetReply<int>("sim_port");
        if (region_udp_port <= 0 || region_udp_port >= 65536)
            return "";

        std::stringstream out;
        out << gridUrl << ":" << region_udp_port;

        return out.str();
    }
}