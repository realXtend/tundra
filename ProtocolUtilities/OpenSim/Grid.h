// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file Grid.h
 *  @brief Grid parser and other grid related functions if needed in the future
 */

#ifndef incl_Protocol_Grid_h
#define incl_Protocol_Grid_h

namespace ProtocolUtilities
{
    class GridParser
    {

    public:
        /// This function reads the address to connect to for the simulation UDP connection.
        /// @param call Pass in the object to a XMLRPCEPI call that has already been performed. Only the reply part
        ///     will be read by this function.
        /// @return The ip:port to connect to with the UDP socket, or "" if there was an error.
        static std::string ExtractGridAddressFromXMLRPCReply(XmlRpcEpi &call);
    };
}

#endif // incl_Protocol_Grid_h