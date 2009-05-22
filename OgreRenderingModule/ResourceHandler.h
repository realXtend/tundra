// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_ResourceHandler_h
#define incl_OgreRenderer_ResourceHandler_h

#include "ResourceInterface.h"

namespace OgreRenderer
{
    //! Manages Ogre resources & requests for their data from the asset system. Used internally by Renderer.
    class ResourceHandler
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
        Core::request_tag_t RequestResource(const std::string& id, const std::string& type);   
        
        //! Remove a renderer-specific resource. Called by Renderer
        void RemoveResource(const std::string& id, const std::string& type);
        
        //! Handles an asset system event. Called by OgreRenderingModule
        bool HandleAssetEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! Handles a resource event. Called by OgreRenderingModule
        bool HandleResourceEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);
        
    private:
        //! Returns an Ogre texture resource
        /*! Does not automatically make a request to the asset system
            \param id Resource ID, same as asset ID
            \return Resource pointer, or null if not found
         */
        Foundation::ResourcePtr GetTexture(const std::string& id);

        //! Requests a texture to be downloaded & decoded
        /*! A resource event (with the returned request tag) is sent as each quality level is decoded.
            \param id Resource ID, same as asset ID
            \return Request tag, 0 if asset ID invalid or asset system fatally non-existent
         */
        Core::request_tag_t RequestTexture(const std::string& id);

        //! Deletes an Ogre texture resource
        /*! \param id Resource ID, same as asset ID
         */         
        void RemoveTexture(const std::string& id);

        //! Returns an Ogre mesh resource
        /*! Does not automatically make a request to the asset system
            \param id Resource ID, same as asset ID
            \return Resource pointer, or null if not found
         */
        Foundation::ResourcePtr GetMesh(const std::string& id);
    
        //! Requests a mesh to be downloaded & decoded
        /*! A resource event (with the returned request tag) will be sent once download is finished
            \param id Resource ID, same as asset ID
            \return Request tag, 0 if asset ID invalid or asset system fatally non-existent
         */
        Core::request_tag_t RequestMesh(const std::string& id);

        //! Deletes an Ogre mesh resource
        void RemoveMesh(const std::string& id);

        //! Returns an Ogre material resource
        /*! Does not automatically make a request to the asset system
            \param id Resource ID, same as asset ID
            \return Resource pointer, or null if not found
         */
        Foundation::ResourcePtr GetMaterial(const std::string& id);
    
        //! Requests a material to be downloaded & decoded
        /*! A resource event (with the returned request tag) will be sent once download is finished
            \param id Resource ID, same as asset ID
            \return Request tag, 0 if asset ID invalid or asset system fatally non-existent
         */
        Core::request_tag_t RequestMaterial(const std::string& id);

        //! Deletes an Ogre material resource
        void RemoveMaterial(const std::string& id);


        //! Creates or updates a texture, based on a source raw texture resource
        /*! \param source Raw texture 
            \param tag Request tag from raw texture resource event
            \return true if successful
         */
        bool UpdateTexture(Foundation::ResourcePtr source, Core::request_tag_t tag);

        //! Creates or updates a mesh, based on source asset data
        /*! \param source Asset
            \param tag Request tag from asset event        
            \return true if successful
         */
        bool UpdateMesh(Foundation::AssetPtr source, Core::request_tag_t tag); 

        //! Creates or updates a material, based on a source raw material resource
        /*! \param source The material asset data.
            \param tag Request tag from raw asset resource event
            \return true if successful
         */
        bool UpdateMaterial(Foundation::AssetPtr source, Core::request_tag_t tag);

        //! resource event category
        Core::event_category_id_t resourcecategory_id_;
                
        //! Ogre texture resources
        Foundation::ResourceMap textures_;

        //! Ogre mesh resources
        Foundation::ResourceMap meshes_;

        //! Ogre material resources
        Foundation::ResourceMap materials_;

        /// \note There will be several Ogre-related resources: Texture, Mesh, Material, ParticleSystem, Skeleton. Perhaps we don't want to have a map for
        ///       each resource type.

        //! Expected request tags from other subsystems
        std::set<Core::request_tag_t> expected_request_tags_;
        
        //! Map of resource request tags by resource
        std::map<std::string, Core::RequestTagVector> request_tags_;
        
        //! Framework we belong to
        Foundation::Framework* framework_;
    };
}
#endif
