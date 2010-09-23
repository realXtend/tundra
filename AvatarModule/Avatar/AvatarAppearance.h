// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Avatar_AvatarAppearance_h
#define incl_Avatar_AvatarAppearance_h

#include "EntityComponent/EC_AvatarAppearance.h"
#include "AvatarModule.h"
#include "AvatarModuleApi.h" 

class QDomDocument;

namespace Ogre
{
    class Bone;
    class Entity;
    class Node;
    class Vector3;
    class Quaternion;
}

namespace HttpUtilities
{
    class HttpTask;
    typedef boost::shared_ptr<HttpTask> HttpTaskPtr;
}

namespace Avatar
{
    class AvatarExporter;
    class AvatarExporterRequest;
    typedef boost::shared_ptr<AvatarExporter> AvatarExporterPtr;
    typedef boost::shared_ptr<AvatarExporterRequest> AvatarExporterRequestPtr;

    //! Handles setting up and updating avatars' appearance. Owned by RexLogicModule::Avatar.
    class AV_MODULE_API AvatarAppearance
    {
        //! States of inventory-based export. Must go input-first
        enum InventoryExportState
        {
            Idle = 0,
            Assets,
            Avatar
        };
        
    public:
        AvatarAppearance(AvatarModule *avatar_module);
        ~AvatarAppearance();
        
        //! Reads default appearance of avatar from file to xml document
        void ReadDefaultAppearance(const std::string& filename);
        
        //! Reads an avatar's appearance from avatar storage
        /*! The storage address should be set beforehand in the EC_OpensimAvatar component.
            \param entity Avatar entity
            \param use_default Whether to reset to default appearance if there is no storage url, default false
         */
        void DownloadAppearance(Scene::EntityPtr entity, bool use_default = false);
        
        //! Reads an avatar's appearance from a file
        /*! \param entity Avatar entity
            \param filename Path/filename to load from. Note: can either be an xml or mesh file
            \return true if successful
         */
        bool LoadAppearance(Scene::EntityPtr entity, const std::string& filename);
        
        //! Sets up an avatar entity's default appearance.
        void SetupDefaultAppearance(Scene::EntityPtr entity);
        
        //! Sets up an avatar entity's appearance with data from the appearance EC.
        /*! Since this involves deserializing the appearance description XML & (re)creating the mesh entity,
            should only be called when the whole appearance changes. Calls also SetupDynamicAppearance().
         */
        void SetupAppearance(Scene::EntityPtr entity);
        
        //! Sets ups the dynamic part of an avatar's appearance. This includes morphs & bone modifiers.
        void SetupDynamicAppearance(Scene::EntityPtr entity);
        
        //! Adjusts (possibly dynamic) height offset of avatar
        void AdjustHeightOffset(Scene::EntityPtr entity);
        
        //! Performs frame-based update.
        void Update(f64 frametime);
        
        //! Handles resource event
        bool HandleResourceEvent(event_id_t event_id, IEventData* data);
        //! Handles asset event
        bool HandleAssetEvent(event_id_t event_id, IEventData* data);
        //! Handles inventory event
        bool HandleInventoryEvent(event_id_t event_id, IEventData* data);        
        
        //! Exports avatar to an authentication/avatar storage server account
        void ExportAvatar(Scene::EntityPtr entity, const std::string& account, const std::string& authserver, const std::string& password);
        
        //! Exports avatar to a local directory, including appearance xml and all assets
        void ExportAvatarLocal(Scene::EntityPtr entity, const std::string& outname);
        
        //! Exports avatar via webdav inventory
        void WebDavExportAvatar(Scene::EntityPtr entity);

        //! Finalize the webdav export by sending xml file that points to uploaded files
        void WebDavExportAvatarFinalize(Scene::EntityPtr entity, const QStringList &file_list);

        //! Exports avatar to inventory
        void InventoryExportAvatar(Scene::EntityPtr entity);

        //! Exports avatar to inventory, finalization (after assets uploaded)
        void InventoryExportAvatarFinalize(Scene::EntityPtr entity);
        
        //! Resets avatar export state
        void InventoryExportReset();
        
        //! Changes a material on an avatar. Filename can either be image or material.
        bool ChangeAvatarMaterial(Scene::EntityPtr entity, uint index, const std::string& filename);
        
        //! Adds an attachment on the avatar. Filename is the xml attachment description.
        bool AddAttachment(Scene::EntityPtr entity, const std::string& filename);
        
    private:
        //! Sets up an avatar mesh
        void SetupMeshAndMaterials(Scene::EntityPtr entity);
        
        //! Sets up avatar morphs
        void SetupMorphs(Scene::EntityPtr entity);
        
        //! Sets up avatar bone modifiers
        void SetupBoneModifiers(Scene::EntityPtr entity);
        
        //! Sets up avatar attachments
        void SetupAttachments(Scene::EntityPtr entity);
        
        //! Resets mesh entity bones to initial transform from the mesh original skeleton
        void ResetBones(Scene::EntityPtr entity);
        
        //! Applies a bone modifier
        void ApplyBoneModifier(Scene::EntityPtr entity, const BoneModifier& modifier, float value);
        
        //! Hides vertices from an entity's mesh. Mesh should be cloned from the base mesh and this must not be called more than once for the entity.
        void HideVertices(Ogre::Entity*, std::set<uint> vertices_to_hide);
        
        //! Processes appearance downloads
        void ProcessAppearanceDownloads();
        
        //! Processes avatar export (result from the avatar exporter threadtask)
        void ProcessAvatarExport();
        
        //! Processes an avatar appearance download result
        void ProcessAppearanceDownload(Scene::EntityPtr entity, const u8* data, uint size);

        //! Processes an avatar appearance asset (inventory based avatar)
        void ProcessInventoryAppearance(Scene::EntityPtr entity, const u8* data, uint size, QString base_url = QString());
        
        //! Requests needed avatar resouces
        uint RequestAvatarResources(Scene::EntityPtr entity, const AvatarAssetMap& assets, bool inventorymode = false, QString base_url = QString());
            
        //! Fixes up avatar resource references after downloading of all avatar assets complete
        void FixupResources(Scene::EntityPtr entity);
        
        //! Fixes up a generic resource, whose type is known
        void FixupResource(AvatarAsset& asset, const AvatarAssetMap& asset_map, const std::string& resource_type);
        
        //! Fixes up avatar material (material script/texture resources)
        void FixupMaterial(AvatarMaterial& mat, const AvatarAssetMap& asset_map);
        
        //! Prepares new avatar appearance using xml as the main file
        bool PrepareAppearanceFromXml(Scene::EntityPtr entity, const std::string& filename);

        //! Prepares new avatar appearance using mesh as the main file, with optional xml file
        bool PrepareAppearanceFromMesh(Scene::EntityPtr entity, const std::string& filename);
                
        //! Guesses avatar asset resource type from human-readable asset name
        static const std::string& GetResourceTypeFromName(const std::string& name, bool inventorymode = false);
        
        //! Adds avatar assets to the export request
        void GetAvatarAssetsForExport(AvatarExporterRequestPtr request, EC_AvatarAppearance& appearance, bool inventorymode = false);
        
        //! Adds an avatar asset to the export request
        /*! \return true if added successfully
         */
        bool GetAvatarAssetForExport(AvatarExporterRequestPtr request, const AvatarAsset& appearance, bool replace_spaces = false, bool inventorymode = false);
        
        //! Adds an avatar asset to the export request
        /*! \return true if added successfully
         */
        bool GetAvatarMaterialForExport(AvatarExporterRequestPtr request, const AvatarMaterial& material, bool inventorymode = false);
        
        //! Gets a bone safely from the avatar skeleton
        /*! \return Pointer to bone, or 0 if does not exist
         */
        Ogre::Bone* GetAvatarBone(Scene::EntityPtr entity, const std::string& bone_name);
        
        //! Gets initial derived transform of a bone. This is something Ogre can't give us automatically
        void GetInitialDerivedBonePosition(Ogre::Node* bone, Ogre::Vector3& position);
        
        //! Adds a directory as a temporary Ogre resource directory, group name "Avatar"
        /*! Each time this is called, the previously set temp directory will be removed from the resource system.
         */
        void AddTempResourceDirectory(const std::string& dirname);
        
        //! Default avatar appearance xml document
        boost::shared_ptr<QDomDocument> default_appearance_;
        
        //! Thread tasks for avatar appearance downloads
        std::map<entity_id_t, HttpUtilities::HttpTaskPtr> appearance_downloaders_;
        
        //! Avatar resource request tags associated to entity
        std::map<request_tag_t, entity_id_t> avatar_resource_tags_;

        //! Avatar appearance xml asset request tags associated to entity, for inventory based appearance
        std::map<request_tag_t, entity_id_t> avatar_appearance_tags_;
        
        //! Amount of pending avatar resource requests. When hits 0, should be able to build avatar
        std::map<entity_id_t, uint> avatar_pending_requests_;
        
        //! Legacy storage avatar exporter task
        AvatarExporterPtr avatar_exporter_;
        
        //! Progress of inventory based export
        InventoryExportState inv_export_state_;
        //! Temp assetmap for inventory based export
        AvatarAssetMap inv_export_assetmap_;
        //! Dummy request to collect assets for inventory based export
        AvatarExporterRequestPtr inv_export_request_;
        //! Entity that is being used for inventory based export
        Scene::EntityWeakPtr inv_export_entity_;
        
        Foundation::Framework *framework_;

        AvatarModule *avatar_module_;
    };
}

#endif
