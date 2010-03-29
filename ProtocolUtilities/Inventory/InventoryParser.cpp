// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   InventoryParser.cpp
 *  @brief  Helper utility which parses inventory structure from the login xmlrpc call.
 */

#include "StableHeaders.h"
#include "XmlRpcEpi.h"

#include "Inventory/InventorySkeleton.h"
#include "InventoryParser.h"

namespace ProtocolUtilities
{

// static
bool InventoryParser::IsHardcodedOpenSimFolder(const char *name)
{
    const char *folders[] = { "My Inventory", "Animations", "Body Parts", "Calling Cards", "Clothing", "Gestures",
        "Landmarks", "Lost And Found", "Notecards", "Objects", "Photo Album", "Scripts", "Sounds", "Textures", "Trash",
        "OpenSim Library", "Animations Library", "BodyParts Library", "Clothing Library", "Gestures Library", "Landmarks Library",
        "Notecards Library", "Objects Library", "Photos Library", "Scripts Library", "Sounds Library", "Texture Library" };

    for(int i = 0; i < NUMELEMS(folders); ++i)
    {
#ifdef _MSC_VER
        if (!_strcmpi(folders[i], name))
#else
        if (!strcasecmp(folders[i], name))
#endif
            return true;
    }
    return false;
}

// static
boost::shared_ptr<ProtocolUtilities::InventorySkeleton> InventoryParser::ExtractInventoryFromXMLRPCReply(XmlRpcEpi &call)
{
    boost::shared_ptr<ProtocolUtilities::InventorySkeleton> inventory =
        boost::shared_ptr<ProtocolUtilities::InventorySkeleton>(new ProtocolUtilities::InventorySkeleton);

    XmlRpcCall *xmlrpcCall = call.GetXMLRPCCall();
    if (!xmlrpcCall)
        throw XmlRpcException("Failed to read inventory, no XMLRPC Reply to read!");

    XMLRPC_REQUEST request = xmlrpcCall->GetReply();
    if (!request)
        throw XmlRpcException("Failed to read inventory, no XMLRPC Reply to read!");

    XMLRPC_VALUE result = XMLRPC_RequestGetData(request);
    if (!result)
        throw XmlRpcException("Failed to read inventory, the XMLRPC Reply did not contain any data!");

    /********** My Inventory **********/
    XMLRPC_VALUE inventoryNode = XMLRPC_VectorGetValueWithID(result, "inventory-skeleton");

    if (!inventoryNode || XMLRPC_GetValueType(inventoryNode) != xmlrpc_vector)
        throw XmlRpcException("Failed to read inventory, inventory-skeleton in the reply was not properly formed!");

    typedef std::pair<RexUUID, ProtocolUtilities::InventoryFolderSkeleton> DetachedInventoryFolder;
    typedef std::list<DetachedInventoryFolder> DetachedInventoryFolderList;
    DetachedInventoryFolderList folders;

    XMLRPC_VALUE item = XMLRPC_VectorRewind(inventoryNode);
    while(item)
    {
        XMLRPC_VALUE_TYPE type = XMLRPC_GetValueType(item);
        if (type == xmlrpc_vector) // xmlrpc-epi handles structs as arrays.
        {
            DetachedInventoryFolder folder;

            XMLRPC_VALUE val = XMLRPC_VectorGetValueWithID(item, "name");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_string)
                folder.second.name = XMLRPC_GetValueString(val);

            val = XMLRPC_VectorGetValueWithID(item, "parent_id");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_string)
                folder.first.FromString(XMLRPC_GetValueString(val));

            val = XMLRPC_VectorGetValueWithID(item, "version");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_int)
                folder.second.version = XMLRPC_GetValueInt(val);

            val = XMLRPC_VectorGetValueWithID(item, "type_default");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_int)
                folder.second.type_default = XMLRPC_GetValueInt(val);

            val = XMLRPC_VectorGetValueWithID(item, "folder_id");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_string)
                folder.second.id.FromString(XMLRPC_GetValueString(val));

            folders.push_back(folder);
        }

        item = XMLRPC_VectorNext(inventoryNode);
    }

    // Find and set the inventory root folder.
    XMLRPC_VALUE inventoryRootNode = XMLRPC_VectorGetValueWithID(result, "inventory-root");
    if (!inventoryRootNode)
        throw XmlRpcException("Failed to read inventory, inventory-root in the reply was not present!");

    if (!inventoryRootNode || XMLRPC_GetValueType(inventoryRootNode) != xmlrpc_vector)
        throw XmlRpcException("Failed to read inventory, inventory-root in the reply was not properly formed!");

    XMLRPC_VALUE inventoryRootNodeFirstElem = XMLRPC_VectorRewind(inventoryRootNode);
    if (!inventoryRootNodeFirstElem || XMLRPC_GetValueType(inventoryRootNodeFirstElem) != xmlrpc_vector)
        throw XmlRpcException("Failed to read inventory, inventory-root in the reply was not properly formed!");

    XMLRPC_VALUE val = XMLRPC_VectorGetValueWithID(inventoryRootNodeFirstElem, "folder_id");
    if (!val || XMLRPC_GetValueType(val) != xmlrpc_string)
        throw XmlRpcException("Failed to read inventory, inventory-root struct value folder_id not present!");

    RexUUID inventoryRootFolderID(XMLRPC_GetValueString(val));
    if (inventoryRootFolderID.IsNull())
        throw XmlRpcException("Failed to read inventory, inventory-root value folder_id was null or unparseable!");

    // Find the root folder from the list of detached folders, and set it as the root folder to start with.
    for(DetachedInventoryFolderList::iterator iter = folders.begin(); iter != folders.end(); ++iter)
    {
        if (iter->second.id == inventoryRootFolderID)
        {
            ProtocolUtilities::InventoryFolderSkeleton *root = inventory->GetRoot();
            iter->second.editable = false;
            root->AddChildFolder(iter->second);
            folders.erase(iter);
            break;
        }
    }

    if (inventory->GetFirstChildFolderByName("My Inventory")->id != inventoryRootFolderID)
        throw XmlRpcException("Failed to read inventory, inventory-root value folder_id pointed to a nonexisting folder!");

    // Insert the detached folders onto the tree view until all folders have been added or there are orphans left
    // that cannot be added, and quit.
    bool progress = true;
    while(folders.size() > 0 && progress)
    {
        progress = false;
        DetachedInventoryFolderList::iterator iter = folders.begin();
        while(iter != folders.end())
        {
            DetachedInventoryFolderList::iterator next = iter;
            ++next;

            ProtocolUtilities::InventoryFolderSkeleton *parent = inventory->GetChildFolderById(iter->first);
            if (parent)
            {
                // Mark harcoded OpenSim Library folders non-editable.
                if (parent->id == inventoryRootFolderID &&
                    IsHardcodedOpenSimFolder(iter->second.name.c_str()))
                    iter->second.editable = false;

                parent->AddChildFolder(iter->second);
                progress = true;
                folders.erase(iter);
            }

            iter = next;
        }
    }

    /********** World Library **********/

    // Find and set the inventory-lib-owner uuid.
    XMLRPC_VALUE inventoryLibOwnerNode = XMLRPC_VectorGetValueWithID(result, "inventory-lib-owner");
    if (!inventoryLibOwnerNode)
        throw XmlRpcException("Failed to read inventory, inventory-lib-owner in the reply was not present!");

    RexUUID inventoryLibOwnerId;
    if (XMLRPC_GetValueType(inventoryLibOwnerNode) != xmlrpc_vector)
    {
        // In Taiga inventory-lib-owner isn't array, just single value.
        if (XMLRPC_GetValueType(inventoryLibOwnerNode) == xmlrpc_string)
            inventoryLibOwnerId.FromString(XMLRPC_GetValueString(inventoryLibOwnerNode));
        else
            throw XmlRpcException("Failed to read inventory, inventory-lib-owner in the reply was not properly formed!");
    }
    else
    {
        // In legacy servers inventory-lib-owner is array.
        XMLRPC_VALUE inventoryLibOwnerNodeFirstElem = XMLRPC_VectorRewind(inventoryLibOwnerNode);
        if (!inventoryLibOwnerNodeFirstElem || XMLRPC_GetValueType(inventoryLibOwnerNodeFirstElem) != xmlrpc_vector)
            throw XmlRpcException("Failed to read inventory,inventory-lib-owner in the reply was not properly formed!");

        val = XMLRPC_VectorGetValueWithID(inventoryLibOwnerNodeFirstElem, "agent_id");
        if (!val || XMLRPC_GetValueType(val) != xmlrpc_string)
            throw XmlRpcException("Failed to read inventory, inventory-lib-owner struct value agent_id not present!");

        inventoryLibOwnerId.FromString(XMLRPC_GetValueString(val));
    }

    if (inventoryLibOwnerId.IsNull())
    {
        throw XmlRpcException("Failed to read inventory, inventory-lib-owner value agent_id was null or unparseable!");
        return inventory;
    }

    inventory->worldLibraryOwnerId = inventoryLibOwnerId;

    // Find and set the inventory root folder.
    XMLRPC_VALUE inventoryLibraryNode = XMLRPC_VectorGetValueWithID(result, "inventory-skel-lib");

    if (!inventoryLibraryNode || XMLRPC_GetValueType(inventoryLibraryNode) != xmlrpc_vector)
    {
        // Note: E.g. ScienceSim doens't have have World Library. Don't throw exception, just return here.
        //throw XmlRpcException("Failed to read world inventory, inventory in the reply was not properly formed!");
        return inventory;
    }

    DetachedInventoryFolderList library_folders;

    item = XMLRPC_VectorRewind(inventoryLibraryNode);
    while(item)
    {
        XMLRPC_VALUE_TYPE type = XMLRPC_GetValueType(item);
        if (type == xmlrpc_vector) // xmlrpc-epi handles structs as arrays.
        {
            DetachedInventoryFolder folder;

            XMLRPC_VALUE val = XMLRPC_VectorGetValueWithID(item, "name");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_string)
                folder.second.name = XMLRPC_GetValueString(val);

            val = XMLRPC_VectorGetValueWithID(item, "parent_id");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_string)
                folder.first.FromString(XMLRPC_GetValueString(val));

            val = XMLRPC_VectorGetValueWithID(item, "version");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_int)
                folder.second.version = XMLRPC_GetValueInt(val);

            val = XMLRPC_VectorGetValueWithID(item, "type_default");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_int)
                folder.second.type_default = XMLRPC_GetValueInt(val);

            val = XMLRPC_VectorGetValueWithID(item, "folder_id");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_string)
                folder.second.id.FromString(XMLRPC_GetValueString(val));

            library_folders.push_back(folder);
        }

        item = XMLRPC_VectorNext(inventoryLibraryNode);
    }

    // Find and set the world library root folder.
    XMLRPC_VALUE inventoryLibraryRootNode = XMLRPC_VectorGetValueWithID(result, "inventory-lib-root");
    if (!inventoryLibraryRootNode)
        throw XmlRpcException("Failed to read inventory, inventory-lib-root in the reply was not present!");

    if (!inventoryLibraryRootNode || XMLRPC_GetValueType(inventoryLibraryRootNode) != xmlrpc_vector)
        throw XmlRpcException("Failed to read inventory, inventory-lib--root in the reply was not properly formed!");

    XMLRPC_VALUE inventoryLibraryRootNodeFirstElem = XMLRPC_VectorRewind(inventoryLibraryRootNode);
    if (!inventoryLibraryRootNodeFirstElem || XMLRPC_GetValueType(inventoryRootNodeFirstElem) != xmlrpc_vector)
        throw XmlRpcException("Failed to read inventory, inventory-lib-root in the reply was not properly formed!");

    val = XMLRPC_VectorGetValueWithID(inventoryLibraryRootNodeFirstElem, "folder_id");
    if (!val || XMLRPC_GetValueType(val) != xmlrpc_string)
        throw XmlRpcException("Failed to read inventory, inventory-lib-root struct value folder_id not present!");

    RexUUID inventoryLibraryRootFolderID(XMLRPC_GetValueString(val));
    if (inventoryLibraryRootFolderID.IsNull())
        throw XmlRpcException("Failed to read inventory, inventory-lib-root value folder_id was null or unparseable!");

    // Find the root folder from the list of detached folders, and set it as the root folder to start with.
    for(DetachedInventoryFolderList::iterator iter = library_folders.begin(); iter != library_folders.end(); ++iter)
    {
        if (iter->second.id == inventoryLibraryRootFolderID)
        {
            ProtocolUtilities::InventoryFolderSkeleton *root = inventory->GetRoot();
            iter->second.editable = false;
            root->AddChildFolder(iter->second);
            library_folders.erase(iter);
            break;
        }
    }

    ProtocolUtilities::InventoryFolderSkeleton *worldLibrary = inventory->GetChildFolderById(inventoryLibraryRootFolderID);
    if (!worldLibrary)
        throw XmlRpcException("Failed to read inventory, inventory-lib-root value folder_id pointed to a nonexisting folder!");

    // Insert the detached folders onto the tree view until all folders have been added or there are orphans left
    // that cannot be added, and quit.
    progress = true;
    while(library_folders.size() > 0 && progress)
    {
        progress = false;
        DetachedInventoryFolderList::iterator iter = library_folders.begin();
        while(iter != library_folders.end())
        {
            DetachedInventoryFolderList::iterator next = iter;
            ++next;

            ProtocolUtilities::InventoryFolderSkeleton *parent = inventory->GetChildFolderById(iter->first);
            if (parent)
            {
                // Mark all World Libary folder descendents non-editable.
                iter->second.editable = false;
                parent->AddChildFolder(iter->second);
                progress = true;
                library_folders.erase(iter);
            }

            iter = next;
        }
    }

    return inventory;
}

// STATIC
void InventoryParser::SetErrorFolder(ProtocolUtilities::InventoryFolderSkeleton *root)
{
    ProtocolUtilities::InventoryFolderSkeleton errorFolder;
    errorFolder.name = "Inventory parsing failed";
    root->AddChildFolder(errorFolder);
}

}
