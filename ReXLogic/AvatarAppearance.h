// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_AvatarAppearance_h
#define incl_RexLogic_AvatarAppearance_h

#include "EC_AvatarAppearance.h"

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

namespace RexLogic
{
    class RexLogicModule;
    class AvatarExporter;
    class AvatarExporterRequest;
    typedef boost::shared_ptr<AvatarExporter> AvatarExporterPtr;
    typedef boost::shared_ptr<AvatarExporterRequest> AvatarExporterRequestPtr;
    
    //! Handles setting up and updating avatars' appearance. Owned by RexLogicModule::Avatar.
    class AvatarAppearance
    {
    public:
        AvatarAppearance(RexLogicModule *rexlogicmodule);
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
        void Update(Core::f64 frametime);
        
        //! Handles resource event
        bool HandleResourceEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);
        
        //! Handles inventory event
        bool HandleInventoryEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);        
        
        //! Exports avatar to an authentication/avatar storage server account
        void ExportAvatar(Scene::EntityPtr entity, const std::string& account, const std::string& authserver, const std::string& password);
        
        //! Exports avatar to inventory
        void ExportAvatar(Scene::EntityPtr entity);
        
        //! Changes a material on an avatar. Filename can either be image or material.
        bool ChangeAvatarMaterial(Scene::EntityPtr entity, Core::uint index, const std::string& filename);
        
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
        void ApplyBoneModifier(Scene::EntityPtr entity, const BoneModifier& modifier, Core::Real value);
        
        //! Hides vertices from an entity's mesh. Mesh should be cloned from the base mesh and this must not be called more than once for the entity.
        void HideVertices(Ogre::Entity*, std::set<Core::uint> vertices_to_hide);
        
        //! Processes appearance downloads
        void ProcessAppearanceDownloads();
        
        //! Processes avatar export (result from the avatar exporter threadtask)
        void ProcessAvatarExport();
        
        //! Processes an avatar appearance download result
        void ProcessAppearanceDownload(Scene::EntityPtr entity, const std::vector<Core::u8>& data);
        
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
        static const std::string& GetResourceTypeFromName(const std::string& name);
        
        //! Adds avatar assets to the export request
        void GetAvatarAssetsForExport(AvatarExporterRequestPtr request, EC_AvatarAppearance& appearance);
        
        //! Adds an avatar asset to the export request
        /*! \return true if added successfully
         */
        bool GetAvatarAssetForExport(AvatarExporterRequestPtr request, const AvatarAsset& appearance, bool replace_spaces = false);
        
        //! Adds an avatar asset to the export request
        /*! \return true if added successfully
         */
        bool GetAvatarMaterialForExport(AvatarExporterRequestPtr request, const AvatarMaterial& material);
        
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
        std::map<Core::entity_id_t, HttpUtilities::HttpTaskPtr> appearance_downloaders_;
        
        //! Avatar resource request tags associated to entity
        std::map<Core::request_tag_t, Core::entity_id_t> avatar_resource_tags_;
        
        //! Amount of pending avatar resource requests. When hits 0, should be able to build avatar
        std::map<Core::entity_id_t, Core::uint> avatar_pending_requests_;
        
        //! Avatar exporter task
        AvatarExporterPtr avatar_exporter_;
        
        RexLogicModule *rexlogicmodule_;
    };
}

#endif
