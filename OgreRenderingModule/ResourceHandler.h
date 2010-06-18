// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_ResourceHandler_h
#define incl_OgreRenderer_ResourceHandler_h

#include "ResourceInterface.h"
#include "AssetInterface.h"
#include "OgreModuleApi.h"

namespace OgreRenderer
{
    //! Manages Ogre resources & requests for their data from the asset system. Used internally by Renderer.
    class OGRE_MODULE_API ResourceHandler
    {
    public:
        //! Constructor
        explicit ResourceHandler(Foundation::Framework* framework);
        
        //! Destructor
        ~ResourceHandler();
    
        //! Postinitialization. Queries event categories
        void PostInitialize();
        
        //! Get a renderer-specific resource. Called by Renderer
        Foundation::ResourcePtr GetResource(const std::string& id, const std::string& type);   
        
        //! Request a renderer-specific resource. Called by Renderer
        request_tag_t RequestResource(const std::string& id, const std::string& type);   
        
        //! Remove a renderer-specific resource. Called by Renderer
        void RemoveResource(const std::string& id, const std::string& type);
        
        //! Get all loaded resources of certain type
        std::vector<Foundation::ResourcePtr> GetResources(const std::string& type);
        
        //! Handles an asset system event. Called by OgreRenderingModule
        bool HandleAssetEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        //! Handles a resource event. Called by OgreRenderingModule
        bool HandleResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data);
        
        //! Internal method to parse braces from an Ogre script. Returns true if line contained open/close brace
        static bool ProcessBraces(const std::string& line, int& brace_level);
        
    private:
        //! Get a renderer-specific resource, without caring if it is valid
        Foundation::ResourcePtr GetResourceInternal(const std::string& id, const std::string& type); 
        
        //! Requests a texture to be downloaded & decoded
        /*! A resource event (with the returned request tag) is sent as each quality level is decoded.
            \param id Resource ID, same as asset ID
            \return Request tag, 0 if asset ID invalid or asset system fatally non-existent
         */
        request_tag_t RequestTexture(const std::string& id);
    
        //! Requests other asset than texture to be downloaded & decoded
        /*! A resource event (with the returned request tag) will be sent once download is finished
            \param id Resource ID, same as asset ID
            \return Request tag, 0 if asset ID invalid or asset system fatally non-existent
         */
        request_tag_t RequestOtherResource(const std::string& id, const std::string& type);

        //! Creates or updates a texture, based on a source raw texture resource
        /*! \param source Raw texture 
            \param tag Request tag from raw texture resource event
            \return true if successful
         */
        bool UpdateTexture(Foundation::ResourcePtr source, request_tag_t tag);

        //! Creates or updates a mesh, based on source asset data
        /*! \param source Asset
            \param tag Request tag from asset event
            \return true if successful
         */
        bool UpdateMesh(Foundation::AssetPtr source, request_tag_t tag); 

        //! Creates or updates a skeleton, based on source asset data
        /*! \param source Asset
            \param tag Request tag from asset event
            \return true if successful
         */
        bool UpdateSkeleton(Foundation::AssetPtr source, request_tag_t tag); 

        //! Creates or updates a material, based on source asset data
        /*! \param source The material asset data.
            \param tag Request tag from raw asset resource event
            \return true if successful
         */
        bool UpdateMaterial(Foundation::AssetPtr source, request_tag_t tag);

        //! Creates or updates particle scripts, based on source asset data
        /*! \param source The particle script asset data.
            \param tag Request tag from raw asset resource event
            \return true if successful
         */
        bool UpdateParticles(Foundation::AssetPtr source, request_tag_t tag);

        //! Creates or updates image based texture, based on source asset data
        /*! \param source The image asset data.
            \param tag Request tag from raw asset resource event
            \return true if successful
         */
        bool UpdateImageTexture(Foundation::AssetPtr source, request_tag_t tag);

        //! Processes resource references of a resource once it has been loaded.
        /*! Adds references to outstanding list and makes requests as necessary.
            If no outstanding references, sends RESOURCE_READY event
            Only does this once per resource (if no previous outstanding requests exist)
         */
        void ProcessResourceReferences(Foundation::ResourcePtr resource);
        
        //! Processes resource ready for an outstanding reference
        void ProcessReferenceReady(Foundation::ResourcePtr resource, request_tag_t tag);
        
        //! Gets number of outstanding (not yet loaded) references for resource
        unsigned GetNumOutstandingReferences(const std::string& id);

        //! resource event category
        event_category_id_t resource_event_category_;
                
        //! Ogre resources
        Foundation::ResourceMap resources_;

        //! Expected request tags from other subsystems
        std::set<request_tag_t> expected_request_tags_;
        
        //! Map of resource request tags by resource
        std::map<std::string, RequestTagVector> request_tags_;
        
        //! Map of source asset types by renderer resource type
        std::map<std::string, std::string> source_types_;
        
        //! Map of reference request tags to resource id's
        std::map<request_tag_t, std::string> reference_request_tags_;
        
        //! Map of outstanding reference requests per resource
        std::map<std::string, Foundation::ResourceReferenceVector> outstanding_references_;
        
        //! Framework we belong to
        Foundation::Framework* framework_;
    };
}
#endif
