// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexNetworkUtils.h"
#include "Avatar.h"
#include "AvatarAppearance.h"
#include "RexLogicModule.h"
#include "EC_OpenSimPresence.h"
#include "EC_OpenSimAvatar.h"
#include "EC_NetworkPosition.h"
#include "EC_AvatarAppearance.h"
#include "EC_Controllable.h"
#include "SceneEvents.h"
#include <Ogre.h>
#include "EC_OgreMesh.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMovableTextOverlay.h"
#include "EC_OgreAnimationController.h"
#include "Renderer.h"
#include "ConversionUtils.h"
#include "SceneManager.h"

#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/Element.h"
#include "Poco/DOM/Attr.h"
#include "Poco/DOM/NamedNodeMap.h"
#include "Poco/DOM/AutoPtr.h"
#include "Poco/SAX/InputSource.h"

namespace RexLogic
{
    Avatar::Avatar(RexLogicModule *rexlogicmodule) :
        avatar_appearance_(rexlogicmodule)
    { 
        rexlogicmodule_ = rexlogicmodule;

        avatar_states_[RexTypes::RexUUID("6ed24bd8-91aa-4b12-ccc7-c97c857ab4e0")] = EC_OpenSimAvatar::Walk;
        avatar_states_[RexTypes::RexUUID("47f5f6fb-22e5-ae44-f871-73aaaf4a6022")] = EC_OpenSimAvatar::Walk;
        avatar_states_[RexTypes::RexUUID("2408fe9e-df1d-1d7d-f4ff-1384fa7b350f")] = EC_OpenSimAvatar::Stand;
        avatar_states_[RexTypes::RexUUID("aec4610c-757f-bc4e-c092-c6e9caf18daf")] = EC_OpenSimAvatar::Fly;
        avatar_states_[RexTypes::RexUUID("1a5fe8ac-a804-8a5d-7cbd-56bd83184568")] = EC_OpenSimAvatar::Sit;
        avatar_states_[RexTypes::RexUUID("1c7600d6-661f-b87b-efe2-d7421eb93c86")] = EC_OpenSimAvatar::Sit;
        avatar_states_[RexTypes::RexUUID("4ae8016b-31b9-03bb-c401-b1ea941db41d")] = EC_OpenSimAvatar::Hover;
        avatar_states_[RexTypes::RexUUID("20f063ea-8306-2562-0b07-5c853b37b31e")] = EC_OpenSimAvatar::Hover;
        avatar_states_[RexTypes::RexUUID("62c5de58-cb33-5743-3d07-9e4cd4352864")] = EC_OpenSimAvatar::Hover;
    }

    Avatar::~Avatar()
    {
    }
    
    Scene::EntityPtr Avatar::GetOrCreateAvatarEntity(Core::entity_id_t entityid, const RexUUID &fullid)
    {
        // Make sure scene exists
        Scene::ScenePtr scene = rexlogicmodule_->GetCurrentActiveScene();
        if (!scene)
            return Scene::EntityPtr();

        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarEntity(entityid);
        if (!entity)
        {
            entity = CreateNewAvatarEntity(entityid);
            
            if (entity)
            {
                rexlogicmodule_->RegisterFullId(fullid,entityid);
            
                EC_OpenSimPresence &presence = *checked_static_cast<EC_OpenSimPresence*>(entity->GetComponent(EC_OpenSimPresence::NameStatic()).get());
                presence.LocalId = entityid; ///\note In current design it holds that localid == entityid, but I'm not sure if this will always be so?
                presence.FullId = fullid;
            }
        }
        return entity;
    }    

    Scene::EntityPtr Avatar::CreateNewAvatarEntity(Core::entity_id_t entityid)
    {
        Scene::ScenePtr scene = rexlogicmodule_->GetCurrentActiveScene();
        if (!scene || !rexlogicmodule_->GetFramework()->GetComponentManager()->CanCreate(OgreRenderer::EC_OgrePlaceable::NameStatic()))
            return Scene::EntityPtr();
        
        Core::StringVector defaultcomponents;
        defaultcomponents.push_back(EC_OpenSimPresence::NameStatic());
        defaultcomponents.push_back(EC_OpenSimAvatar::NameStatic());
        defaultcomponents.push_back(EC_NetworkPosition::NameStatic());
        defaultcomponents.push_back(EC_AvatarAppearance::NameStatic());
        defaultcomponents.push_back(OgreRenderer::EC_OgrePlaceable::NameStatic());
        defaultcomponents.push_back(OgreRenderer::EC_OgreMovableTextOverlay::NameStatic());
        defaultcomponents.push_back(OgreRenderer::EC_OgreMesh::NameStatic());
        defaultcomponents.push_back(OgreRenderer::EC_OgreAnimationController::NameStatic());
        
        Scene::EntityPtr entity = scene->CreateEntity(entityid, defaultcomponents);

        Foundation::ComponentPtr placeable = entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
        if (placeable)
        {
            OgreRenderer::EC_OgrePlaceable &ogrepos = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(placeable.get());
            //DebugCreateOgreBoundingBox(rexlogicmodule_,
            //    entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()), "AmbientGreen", Vector3(0.5,0.5,1.5));
            
            CreateNameOverlay(placeable, entityid);
            CreateAvatarMesh(entityid);
        }
        
        return entity;
    } 

    bool Avatar::HandleOSNE_ObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data)
    {
        NetInMessage *msg = data->message;
 
        msg->ResetReading();
        uint64_t regionhandle = msg->ReadU64();
        msg->SkipToNextVariable(); // TimeDilation U16 ///\todo Unhandled inbound variable 'TimeDilation'.
        
        // Variable block: Object Data
        size_t instance_count = data->message->ReadCurrentBlockInstanceCount();
        for(size_t i = 0; i < instance_count; ++i)
        {
            uint32_t localid = msg->ReadU32(); 
            msg->SkipToNextVariable();		// State U8 ///\todo Unhandled inbound variable 'State'.
            RexUUID fullid = msg->ReadUUID();
            msg->SkipToNextVariable();		// CRC U32 ///\todo Unhandled inbound variable 'CRC'.
            uint8_t pcode = msg->ReadU8();

            Scene::EntityPtr entity = GetOrCreateAvatarEntity(localid, fullid);
            if (!entity)
                return false;
                
            EC_OpenSimPresence &presence = *checked_static_cast<EC_OpenSimPresence*>(entity->GetComponent(EC_OpenSimPresence::NameStatic()).get());
            EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(entity->GetComponent(EC_NetworkPosition::NameStatic()).get());

            presence.RegionHandle = regionhandle;
            
            // Get position from objectdata
            msg->SkipToFirstVariableByName("ObjectData");
            size_t bytes_read = 0;
            const uint8_t *objectdatabytes = msg->ReadBuffer(&bytes_read);
            if (bytes_read >= 28)
            {
                // The data contents:
                // ofs 16 - pos xyz - 3 x float (3x4 bytes)
                netpos.position_ = Core::OpenSimToOgreCoordinateAxes(*reinterpret_cast<const Core::Vector3df*>(&objectdatabytes[16]));
                netpos.Updated();
            }

            msg->SkipToFirstVariableByName("ParentID");
            presence.ParentId = msg->ReadU32();
            
            // NameValue contains: FirstName STRING RW SV " + firstName + "\nLastName STRING RW SV " + lastName
            msg->SkipToFirstVariableByName("NameValue");
            std::string namevalue = msg->ReadString();
            NameValueMap map = ParseNameValueMap(namevalue);
            presence.SetFirstName(map["FirstName"]);
            presence.SetLastName(map["LastName"]);
            
            // If the server sent an ObjectUpdate on a prim that is actually the client's avatar, and if the Entity that 
            // corresponds to this prim doesn't yet have a Controllable component, add it to the Entity.
            // This also causes a EVENT_CONTROLLABLE_ENTITY to be passed which will register this Entity as the currently 
            // controlled avatar entity. -jj.
            ///\todo Perhaps this logic could be done beforehand when creating the avatar Entity instead of doing it here? -jj.
            if (presence.FullId == rexlogicmodule_->GetServerConnection()->GetInfo().agentID && !entity->GetComponent(EC_Controllable::NameStatic()))
            {
                Foundation::Framework *fw = rexlogicmodule_->GetFramework();
                assert (fw->GetComponentManager()->CanCreate(EC_Controllable::NameStatic()));

                entity->AddEntityComponent(fw->GetComponentManager()->CreateComponent(EC_Controllable::NameStatic()));

                Scene::Events::EntityEventData event_data;
                event_data.entity = entity;
                fw->GetEventManager()->SendEvent(fw->GetEventManager()->QueryEventCategory("Scene"), Scene::Events::EVENT_CONTROLLABLE_ENTITY, &event_data);
            }

            ShowAvatarNameOverlay(presence.LocalId);

            msg->SkipToFirstVariableByName("JointAxisOrAnchor");
            msg->SkipToNextVariable(); // To next instance
        }
        
        return false;
    }
    
    void Avatar::HandleTerseObjectUpdate_30bytes(const uint8_t* bytes)
    {
        if (!rexlogicmodule_ || !rexlogicmodule_->GetCurrentActiveScene().get())
            return;

        // The data contents:
        // ofs  0 - localid - packed to 4 bytes
        // ofs  4 - position xyz - 3 x float (3x4 bytes)
        // ofs 16 - velocity xyz - packed to 6 bytes
        // ofs 22 - rotation - packed to 8 bytes
        
        //! \todo handle endians
        int i = 0;
        uint32_t localid = *reinterpret_cast<uint32_t*>((uint32_t*)&bytes[i]);                
        i += 4;

        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarEntity(localid);
        if(!entity) return;
        EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(entity->GetComponent(EC_NetworkPosition::NameStatic()).get());

        Core::Vector3df position = GetProcessedVector(&bytes[i]);    
        i += sizeof(Core::Vector3df);
        if (!IsValidPositionVector(position))
            return;
        
        netpos.velocity_ = GetProcessedScaledVectorFromUint16(&bytes[i],128);
        i += 6;
        
        Core::Quaternion rotation = GetProcessedQuaternion(&bytes[i]);
     
        netpos.position_ = position;
        if (!entity->GetComponent(EC_Controllable::NameStatic()))
        {
            // Do not update rotation for entities controlled by this client, client handles the rotation for itself (jitters during turning may result otherwise).
            netpos.rotation_ = rotation;
        }
                  
        //! \todo what to do with acceleration & rotation velocity? zero them currently
        netpos.accel_ = Core::Vector3df::ZERO;
        netpos.rotvel_ = Core::Vector3df::ZERO;
        
        netpos.Updated();

        assert(i <= 30);
    }    
    
    void Avatar::HandleTerseObjectUpdateForAvatar_60bytes(const uint8_t* bytes)
    {
        // The data contents:
        // ofs  0 - localid - packed to 4 bytes
        // ofs  4 - 0
        // ofs  5 - 1
        // ofs  6 - empty 14 bytes
        // ofs 20 - 128
        // ofs 21 - 63
        // ofs 22 - position xyz - 3 x float (3x4 bytes)
        // ofs 34 - velocity xyz - packed to 6 bytes        
        // ofs 40 - acceleration xyz - packed to 6 bytes           
        // ofs 46 - rotation - packed to 8 bytes 
        // ofs 54 - rotational vel - packed to 6 bytes
        
        //! \todo handle endians
        int i = 0;
        uint32_t localid = *reinterpret_cast<uint32_t*>((uint32_t*)&bytes[i]);                
        i += 22;
        
        // set values
        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarEntity(localid);
        if(!entity) return;        
        EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(entity->GetComponent(EC_NetworkPosition::NameStatic()).get());

        Core::Vector3df position = GetProcessedVector(&bytes[i]);
        i += sizeof(Core::Vector3df);

        if (!IsValidPositionVector(position))
            return;

        netpos.velocity_ = GetProcessedScaledVectorFromUint16(&bytes[i],128);
        i += 6;
        
        netpos.accel_ = GetProcessedVectorFromUint16(&bytes[i]);
        i += 6;

        Core::Quaternion rotation = GetProcessedQuaternion(&bytes[i]);
        i += 8;        

        netpos.rotvel_ = GetProcessedScaledVectorFromUint16(&bytes[i],128);
        
        netpos.position_ = position;
        if (!entity->GetComponent(EC_Controllable::NameStatic()))
        {
            // Do not update rotation for entities controlled by this client, client handles the rotation for itself (jitters during turning may result otherwise).
            netpos.rotation_ = rotation;
        }
        
        netpos.Updated();

        assert(i <= 60);                            
    }
        
    bool Avatar::HandleRexGM_RexAppearance(OpenSimProtocol::NetworkEventInboundData* data)
    {        
        data->message->ResetReading();    
        data->message->SkipToFirstVariableByName("Parameter");

        // Variable block begins
        size_t instance_count = data->message->ReadCurrentBlockInstanceCount();

        bool overrideappearance = false;

        if (instance_count >= 2)
        {
            std::string avataraddress = data->message->ReadString();
            RexUUID avatarid(data->message->ReadString());
        
            if (instance_count >= 3)
                overrideappearance = ParseBool(data->message->ReadString());
        
            Scene::EntityPtr entity = rexlogicmodule_->GetAvatarEntity(avatarid);
            if(entity)
            {
                EC_OpenSimAvatar &avatar = *checked_static_cast<EC_OpenSimAvatar*>(entity->GetComponent(EC_OpenSimAvatar::NameStatic()).get());        
                avatar.SetAppearanceAddress(avataraddress,overrideappearance);
                avatar_appearance_.DownloadAppearance(entity);
            }
        }
        
        return false;
    }
    
    bool Avatar::HandleOSNE_KillObject(uint32_t objectid)
    {
        Scene::ScenePtr scene = rexlogicmodule_->GetCurrentActiveScene();
        if (!scene)
            return false;

        RexTypes::RexUUID fullid;
        fullid.SetNull();
        Scene::EntityPtr entity = scene->GetEntity(objectid);
        if(!entity)
            return false;

        Foundation::ComponentPtr component = entity->GetComponent(EC_OpenSimPresence::NameStatic());
        if(component)
        {
            EC_OpenSimPresence &presence = *checked_static_cast<EC_OpenSimPresence*>(component.get());
            fullid = presence.FullId;
        }
        
        scene->RemoveEntity(objectid);
        rexlogicmodule_->UnregisterFullId(fullid);
        return false;
    }
   
    bool Avatar::HandleOSNE_AvatarAnimation(OpenSimProtocol::NetworkEventInboundData* data)   
    {        
        data->message->ResetReading();
        RexUUID avatarid = data->message->ReadUUID();
     
        std::vector<RexUUID> animations_to_start;
        size_t animlistcount = data->message->ReadCurrentBlockInstanceCount();
        for(size_t i = 0; i < animlistcount; i++)
        {
            RexUUID animid = data->message->ReadUUID();
            Core::s32 animsequence = data->message->ReadS32();

            animations_to_start.push_back(animid);
            
            if(avatar_states_.find(animid) != avatar_states_.end())
            {
                // Set avatar state based on animation: not probably best way, but possibly acceptable for now
                SetAvatarState(avatarid, avatar_states_[animid]);
            }
        }
        
        size_t animsourcelistcount = data->message->ReadCurrentBlockInstanceCount();
        for(size_t i = 0; i < animsourcelistcount; i++)
        {
            RexUUID objectid = data->message->ReadUUID();  
        }
        // PhysicalAvatarEventList not used
                
        StartAvatarAnimations(avatarid, animations_to_start);
                
        return false;
    }     
    
    void Avatar::Update(Core::f64 frametime)
    {
        avatar_appearance_.Update(frametime);
    }
        
    void Avatar::UpdateAvatarNameOverlayPositions()
    {
        Foundation::ComponentManager::const_iterator it;
        for (it = rexlogicmodule_->GetFramework()->GetComponentManager()->Begin("EC_OgreMovableTextOverlay");
             it != rexlogicmodule_->GetFramework()->GetComponentManager()->End("EC_OgreMovableTextOverlay"); ++it)
        {
            OgreRenderer::EC_OgreMovableTextOverlay &name_overlay = *static_cast<OgreRenderer::EC_OgreMovableTextOverlay*>(it->lock().get());
            name_overlay.Update();
        }
    }
    
    void Avatar::CreateNameOverlay(Foundation::ComponentPtr placeable, Core::entity_id_t entity_id)
    {
        Scene::ScenePtr scene = rexlogicmodule_->GetCurrentActiveScene();
        if (!scene)
            return;
        
        Scene::EntityPtr entity = scene->GetEntity(entity_id);
        if (!entity)
            return;
        
        Foundation::ComponentPtr overlay = entity->GetComponent(OgreRenderer::EC_OgreMovableTextOverlay::NameStatic());
        EC_OpenSimPresence &presence = *checked_static_cast<EC_OpenSimPresence*>(entity->GetComponent(EC_OpenSimPresence::NameStatic()).get());
        if (overlay)
        {
            OgreRenderer::EC_OgreMovableTextOverlay &name_overlay = *checked_static_cast<OgreRenderer::EC_OgreMovableTextOverlay*>(overlay.get());
            name_overlay.CreateOverlay(Core::Vector3df(0.0f, 0.0f, 1.5f));
            name_overlay.SetText(presence.GetFullName());
            name_overlay.SetPlaceable(placeable);
        }
    }   
    void Avatar::ShowAvatarNameOverlay(Core::entity_id_t entity_id)
    {
        Scene::ScenePtr scene = rexlogicmodule_->GetCurrentActiveScene();
        if (!scene)
            return;
        
        Scene::EntityPtr entity = scene->GetEntity(entity_id);
        if (!entity)
            return;
        
        Foundation::ComponentPtr overlay = entity->GetComponent(OgreRenderer::EC_OgreMovableTextOverlay::NameStatic());
        EC_OpenSimPresence &presence = *checked_static_cast<EC_OpenSimPresence*>(entity->GetComponent(EC_OpenSimPresence::NameStatic()).get());
        if (overlay)
        {
            OgreRenderer::EC_OgreMovableTextOverlay &name_overlay = *checked_static_cast<OgreRenderer::EC_OgreMovableTextOverlay*>(overlay.get());
            name_overlay.SetText(presence.GetFullName());
            name_overlay.SetVisible(true);
        }
    }
    
    void Avatar::CreateAvatarMesh(Core::entity_id_t entity_id)
    {
        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarEntity(entity_id);
        if (!entity)
            return;
            
        Foundation::ComponentPtr placeableptr = entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
        Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        Foundation::ComponentPtr animctrlptr = entity->GetComponent(OgreRenderer::EC_OgreAnimationController::NameStatic());
        
        if (placeableptr && meshptr)
        {
            OgreRenderer::EC_OgrePlaceable &placeable = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(placeableptr.get());
            OgreRenderer::EC_OgreMesh &mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(meshptr.get());
            
            mesh.SetPlaceable(placeableptr);
            avatar_appearance_.SetupDefaultAppearance(entity);
        }
        
        if (animctrlptr && meshptr)
        {
            OgreRenderer::EC_OgreAnimationController &animctrl = *checked_static_cast<OgreRenderer::EC_OgreAnimationController*>(animctrlptr.get());
            
            animctrl.SetMeshEntity(meshptr);
        }
    }
    
    void Avatar::StartAvatarAnimations(const RexTypes::RexUUID& avatarid, const std::vector<RexTypes::RexUUID>& anim_ids)
    {
        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarEntity(avatarid);
        if (!entity)
            return;
        
        Foundation::ComponentPtr animctrlptr = entity->GetComponent(OgreRenderer::EC_OgreAnimationController::NameStatic());
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!animctrlptr || !appearanceptr)
            return;
        
        OgreRenderer::EC_OgreAnimationController &animctrl = *checked_static_cast<OgreRenderer::EC_OgreAnimationController*>(animctrlptr.get());
        EC_AvatarAppearance &appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());
        const AnimationDefinitionMap& anim_defs = appearance.GetAnimations();
        
        // Convert uuid's to actual animation names
        std::vector<std::string> anims_to_start;
        for (unsigned i = 0; i < anim_ids.size(); ++i)
        {
            AnimationDefinitionMap::const_iterator def = anim_defs.find(anim_ids[i]);
            if (def != anim_defs.end())
                anims_to_start.push_back(def->second.animation_name_);
        }
        
        // Other animations that are going on have to be stopped
        std::vector<std::string> anims_to_stop;
        const OgreRenderer::EC_OgreAnimationController::AnimationMap& running_anims = animctrl.GetRunningAnimations();
        OgreRenderer::EC_OgreAnimationController::AnimationMap::const_iterator anim = running_anims.begin();
        while (anim != running_anims.end())
        {
            if (std::find(anims_to_start.begin(), anims_to_start.end(), anim->first) == anims_to_start.end())
                anims_to_stop.push_back(anim->first);
            ++anim;
        }
        
        for (unsigned i = 0; i < anims_to_start.size(); ++i)
        {
            const AnimationDefinition& def = GetAnimationByName(anim_defs, anims_to_start[i]);
            
            animctrl.EnableAnimation(
                def.animation_name_,
                def.looped_,
                def.fadein_
            );
            
            animctrl.SetAnimationSpeed(def.animation_name_, def.speedfactor_);
            animctrl.SetAnimationWeight(def.animation_name_, def.weightfactor_);
            
            if (def.always_restart_)
                animctrl.SetAnimationTimePosition(def.animation_name_, 0.0);
        }
        
        for (unsigned i = 0; i < anims_to_stop.size(); ++i)
        {
            const AnimationDefinition& def = GetAnimationByName(anim_defs, anims_to_stop[i]);
            
            animctrl.DisableAnimation(
                def.animation_name_,
                def.fadeout_
            );
        }
    }

    void Avatar::UpdateAvatarAnimations(Core::entity_id_t avatarid, Core::f64 frametime)
    {
        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarEntity(avatarid);
        if (!entity)
            return;
        
        Foundation::ComponentPtr animctrlptr = entity->GetComponent(OgreRenderer::EC_OgreAnimationController::NameStatic());
        Foundation::ComponentPtr netposptr = entity->GetComponent(EC_NetworkPosition::NameStatic());
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!animctrlptr || !netposptr || !appearanceptr)
            return;
        
        OgreRenderer::EC_OgreAnimationController &animctrl = *checked_static_cast<OgreRenderer::EC_OgreAnimationController*>(animctrlptr.get());
        EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(netposptr.get());
        EC_AvatarAppearance &appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());
        const AnimationDefinitionMap& anim_defs = appearance.GetAnimations();
        
        const OgreRenderer::EC_OgreAnimationController::AnimationMap& running_anims = animctrl.GetRunningAnimations();
        OgreRenderer::EC_OgreAnimationController::AnimationMap::const_iterator anim = running_anims.begin();
        while (anim != running_anims.end())
        {
            const AnimationDefinition& def = GetAnimationByName(anim_defs, anim->first);
            // If animation is velocity-adjusted, adjust animation speed by network position speed (horizontal plane movement only)
            if (def.use_velocity_)
            {
                Core::Real speed = Core::Vector3df(netpos.velocity_.x, netpos.velocity_.y, 0).getLength() * 0.5;
                
                animctrl.SetAnimationSpeed(anim->first, def.speedfactor_ * speed);
            }
            
            ++anim;
        }
    }
    
    void Avatar::SetAvatarState(const RexTypes::RexUUID& avatarid, EC_OpenSimAvatar::State state)
    {
        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarEntity(avatarid);
        if (!entity)
            return;
         
        Foundation::ComponentPtr avatar_ptr = entity->GetComponent(EC_OpenSimAvatar::NameStatic());
        if (!avatar_ptr)
            return;
        EC_OpenSimAvatar &avatar = *checked_static_cast<EC_OpenSimAvatar*>(avatar_ptr.get());
        avatar.SetState(state);
    }
}
