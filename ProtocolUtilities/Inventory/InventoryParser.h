// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   InventoryParser.h
 *  @brief  Helper utility which parses inventory structure from the login xmlrpc call.
 */

#ifndef incl_Protocol_InventoryParser_h
#define incl_Protocol_InventoryParser_h

namespace ProtocolUtilities
{
    class InventoryFolderSkeleton;

    class InventoryParser
    {
    public:
        /// This function reads the inventory tree that was stored in the XMLRPC login_to_simulator reply.
        /// @param call Pass in the object to a XMLRPCEPI call that has already been performed. Only the reply part will be read by this function.
        /// @return The inventory object, or null pointer if an error occurred.
        static boost::shared_ptr<ProtocolUtilities::InventorySkeleton> ExtractInventoryFromXMLRPCReply(XmlRpcEpi &call);

        static void SetErrorFolder(ProtocolUtilities::InventoryFolderSkeleton *root);

    private:
        /// Checks if the name of the folder belongs to the harcoded OpenSim folders.
        /// @param name name of the folder.
        /// @return True if one of the harcoded folders, false if not.
        static bool IsHardcodedOpenSimFolder(const char *name);
    };
}

#endif // incl_Protocol_InventoryParser_h
