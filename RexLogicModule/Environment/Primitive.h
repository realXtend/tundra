/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Primitive.h
 *  @brief  Primitive logic handler.
*/

#ifndef incl_RexLogicModule_Primitive_h
#define incl_RexLogicModule_Primitive_h

#include "ResourceInterface.h"
#include "RexTypes.h"
#include "RexUUID.h"
#include "ComponentInterface.h"
#include "SceneManager.h"
#include "Color.h"

#include <QObject>

class QColor;
class QDomDocument;

class EC_OpenSimPrim;

namespace ProtocolUtilities
{
    class NetworkEventInboundData;
}

namespace RexLogic
{
    class RexLogicModule;
    class EC_AttachedSound;

    class Primitive : public QObject
    {
        Q_OBJECT
        
     public:
        Primitive(RexLogicModule *rexlogicmodule);
        ~Primitive();
        
        // Frame update
        void Update(f64 frametime);
        
        bool HandleOSNE_ObjectUpdate(ProtocolUtilities::NetworkEventInboundData* data);
        bool HandleOSNE_KillObject(uint32_t objectid); 
        bool HandleOSNE_ObjectProperties(ProtocolUtilities::NetworkEventInboundData* data);

        bool HandleRexGM_RexMediaUrl(ProtocolUtilities::NetworkEventInboundData* data);
        bool HandleRexGM_RexFreeData(ProtocolUtilities::NetworkEventInboundData* data);
        bool HandleRexGM_RexPrimData(ProtocolUtilities::NetworkEventInboundData* data);
        bool HandleRexGM_RexPrimAnim(ProtocolUtilities::NetworkEventInboundData* data);
        
        bool HandleOSNE_AttachedSound(ProtocolUtilities::NetworkEventInboundData *data);
        bool HandleOSNE_AttachedSoundGainChange(ProtocolUtilities::NetworkEventInboundData *data);

        void HandleTerseObjectUpdateForPrim_44bytes(const uint8_t* bytes);
        void HandleTerseObjectUpdateForPrim_60bytes(const uint8_t* bytes);

        bool HandleResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        void HandleLogout();

        typedef std::map<std::pair<request_tag_t, asset_type_t>, entity_id_t> EntityResourceRequestMap;

        // Send RexPrimData of a prim entity to server
        ///\todo Move to WorldStream?
        void SendRexPrimData(entity_id_t entityid);

        // Send RexFreeData of a prim entity (if exists) to server
        ///\todo Move to WorldStream?
        void SendRexFreeData(entity_id_t entityid);

        // Start listening to Scene's EC notification signals
        void RegisterToComponentChangeSignals(Scene::ScenePtr scene);
        
        // Deserialize EC's sent by server
        void DeserializeECsFromFreeData(Scene::EntityPtr entity, QDomDocument& doc);
        
    public slots:
        //! Trigger EC sync because of component attributes changing
        void OnComponentChanged(Foundation::ComponentInterface* comp, Foundation::ChangeType change);
        //! Trigger EC sync because of components added/removed to entity
        void OnEntityChanged(Scene::Entity* entity, Foundation::ComponentInterface* comp, Foundation::ChangeType change);
        //! When rex prim propeties have changed, send update to sim
        void OnRexPrimDataChanged(Scene::Entity* entity);
        //! When prim shape propeties have changed, send update to sim
        void OnPrimShapeChanged(const EC_OpenSimPrim& prim);
        //! When prim name has changed
        void OnPrimNameChanged(const EC_OpenSimPrim& prim);
        //! When prim description has changed
        void OnPrimDescriptionChanged(const EC_OpenSimPrim& prim);

    private:
        //! The owning module.
        RexLogicModule *rexlogicmodule_;

        //! @return The entity corresponding to given id AND uuid. This entity is guaranteed to have an existing EC_OpenSimPrim component.
        //!         Does not return null. If the entity doesn't exist, an entity with the given entityid and fullid is created and returned.
        Scene::EntityPtr GetOrCreatePrimEntity(entity_id_t entityid, const RexUUID &fullid);
        Scene::EntityPtr CreateNewPrimEntity(entity_id_t entityid);
        
        //! checks if stored pending rexdata exists for prim and handles it
        //! @param entityid Entity id.
        void CheckPendingRexPrimData(entity_id_t entityid);
        
        //! checks if stored pending rexfreedata exists for prim and handles it
        //! @param entityid Entity id.
        void CheckPendingRexFreeData(entity_id_t entityid);
        
        //! parse TextureEntry data from ObjectUpdate
        /*! @param prim Primitive component to receive texture data
            @param data Byte buffer
            @param length Length of byte buffer
         */
        void ParseTextureEntryData(EC_OpenSimPrim& prim, const uint8_t* bytes, size_t length);
        
        //! handle rexprimdata blob coming from server in a genericmessage
        void HandleRexPrimDataBlob(entity_id_t entityid, const uint8_t* primdata, const int primdata_size);
        
        //! handle rexfreedata
        void HandleRexFreeData(entity_id_t entityid, const std::string& freedata);
        
        //! handles changes in rex ambient sound parameters.
        void HandleAmbientSound(entity_id_t entityid);
        
        //! handles changes in drawtype (mesh/prim). also handles particle scripts.
        //! @param entityid Entity id.
        void HandleDrawType(entity_id_t entityid);
        
        //! Re-binds all the Ogre materials attached to the given prim entity. If the materials haven't yet been loaded in, requests for
        //! those materials are made and the material binding is delayed until the downloads are complete.
        void HandleMeshMaterials(entity_id_t entityid);

        //! Handles starting of looping mesh animation as specified in RexPrimData
        void HandleMeshAnimation(entity_id_t entityid);

        //! handles prim texture/material requests
        //! @param entityid Entity id.
        void HandlePrimTexturesAndMaterial(entity_id_t entityid);
        
        //! handles mesh texture/material changes
        //! @param entityid Entity id.
        void HandleMeshTextures(entity_id_t entityid);

        //! handles the ExtraParams data.
        //! @param entity_id Entity id.
        //! @param extra_params_data Binary data blob.
        void HandleExtraParams(const entity_id_t &entity_id, const uint8_t *extra_params_data);

        //! handles mesh resource being ready
        void HandleMeshReady(entity_id_t entity, Foundation::ResourcePtr res);

        //! handles skeleton resource (used in conjunction with a mesh) being ready
        void HandleSkeletonReady(entity_id_t entity, Foundation::ResourcePtr res);

        //! handles particle script resource being ready
        void HandleParticleScriptReady(entity_id_t entity, Foundation::ResourcePtr res);

        /** Attachs a light component to a prim.
         * @param entity Entity pointer of the prim.
         * @param color Color.
         * @param radius Radius of the light.
         * @param falloff Falloff factor of the light.
         */ 
        void AttachLightComponent(Scene::EntityPtr entity, const Color &color, float radius, float falloff);

        /// Creates hovering text above entity. Uses EC_HoveringText
        /// @param entity Entity.
        /// @param text Text to be shown. If null ("") the EC_HoveringText compoenent will be removed from the entity.
        /// @param text_color Color of the text.
        void AttachHoveringTextComponent(Scene::EntityPtr entity, const std::string &text, const QColor &color);

        //! handles mesh or prim texture resource being ready
        void HandleTextureReady(entity_id_t entity, Foundation::ResourcePtr res);

        void HandleMaterialResourceReady(entity_id_t entityid, Foundation::ResourcePtr res);

        //! handles prim size and visibility
        void HandlePrimScaleAndVisibility(entity_id_t entityid);

        //! discards request tags for certain entity
        void DiscardRequestTags(entity_id_t, EntityResourceRequestMap& map);

        // Go through dirty lists & send changed components to server
        void SerializeECsToNetwork();

        //! Return valid uuid if given id is valid uuid or if given id
        //! is valid asset url with format: 'http://domain/path/xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx'
        //! Return zero uuid if either above works
        static RexUUID UuidForRexObjectUpdatePacket(RexTypes::RexAssetID id);

        //! Return valid url string if given id is valid url. 
        //! Return empty string if given id is not valid url.
        static std::string UrlForRexObjectUpdatePacket(RexTypes::RexAssetID id);

        //! maps tags of all pending resource request to prim entities.
        EntityResourceRequestMap prim_resource_request_tags_;

        //! pending rexprimdatas. This map exists because in some cases the network messages that describe prim parameters
        //! are received before the actual objects have been created (first ObjectUpdate is received). Any such pending
        //! messages are queued here to wait that the object is created. The real problem here is that SLUDP doesn't give
        //! us any reliable ordered stream of communication.
        typedef std::map<RexUUID, std::vector<u8> > RexPrimDataMap;
        RexPrimDataMap pending_rexprimdata_;

        //! pending rexfreedatas
        typedef std::map<RexUUID, std::string > RexFreeDataMap;
        RexFreeDataMap pending_rexfreedata_;
        
        typedef std::set<entity_id_t> EntityIdSet;
        //! entities with local EC changes
        EntityIdSet local_dirty_entities_;
        //! entities with EC changes from the network
        EntityIdSet network_dirty_entities_;
    };
}
#endif
