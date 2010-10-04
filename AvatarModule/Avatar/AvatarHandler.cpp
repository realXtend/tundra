/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Avatar.cpp
 *  @brief  Logic handler for avatar entities.
 */

#include "StableHeaders.h"

#include "AvatarEvents.h"
#include "Avatar/AvatarHandler.h"
#include "Avatar/AvatarAppearance.h"
#include "AvatarEditing/AvatarEditor.h"
#include "EntityComponent/EC_AvatarAppearance.h"
#include "EntityComponent/EC_OpenSimAvatar.h"
#include "EntityComponent/EC_Controllable.h"

#include "SceneManager.h"
#include "SceneEvents.h"
#include "EventManager.h"
#include "WorldStream.h"
#include "RexNetworkUtils.h"
#include "GenericMessageUtils.h"
#include "NetworkEvents.h"
#include "EC_Mesh.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMovableTextOverlay.h"
#include "EC_OgreAnimationController.h"
#ifdef EC_HoveringText_ENABLED
#include "EC_HoveringText.h"
#endif
#include "EC_OpenSimPresence.h"
#ifdef EC_SoundListener_ENABLED
#include "EC_SoundListener.h"
#endif
#ifdef EC_HoveringWidget_ENABLED
#include "EC_HoveringWidget.h"
#endif
#include "EC_NetworkPosition.h"

#include <QPushButton>

namespace Avatar
{
    AvatarHandler::AvatarHandler(AvatarModule *avatar_module) : 
        avatar_appearance_(avatar_module), 
        framework_(avatar_module->GetFramework()),
        avatar_module_(avatar_module)
    {
        avatar_states_[RexUUID("6ed24bd8-91aa-4b12-ccc7-c97c857ab4e0")] = EC_OpenSimAvatar::Walk;
        avatar_states_[RexUUID("47f5f6fb-22e5-ae44-f871-73aaaf4a6022")] = EC_OpenSimAvatar::Walk;
        avatar_states_[RexUUID("2408fe9e-df1d-1d7d-f4ff-1384fa7b350f")] = EC_OpenSimAvatar::Stand;
        avatar_states_[RexUUID("aec4610c-757f-bc4e-c092-c6e9caf18daf")] = EC_OpenSimAvatar::Fly;
        avatar_states_[RexUUID("1a5fe8ac-a804-8a5d-7cbd-56bd83184568")] = EC_OpenSimAvatar::Sit;
        avatar_states_[RexUUID("1c7600d6-661f-b87b-efe2-d7421eb93c86")] = EC_OpenSimAvatar::Sit;
        avatar_states_[RexUUID("4ae8016b-31b9-03bb-c401-b1ea941db41d")] = EC_OpenSimAvatar::Hover;
        avatar_states_[RexUUID("20f063ea-8306-2562-0b07-5c853b37b31e")] = EC_OpenSimAvatar::Hover;
        avatar_states_[RexUUID("62c5de58-cb33-5743-3d07-9e4cd4352864")] = EC_OpenSimAvatar::Hover;

        connect(this, SIGNAL(ExportAvatar(Scene::EntityPtr, const std::string&, const std::string&, const std::string&)),
            &avatar_appearance_, SLOT(ExportAvatar(Scene::EntityPtr, const std::string&, const std::string&, const std::string&)));
        connect(this, SIGNAL(WebDavExportAvatar(Scene::EntityPtr)),
            &avatar_appearance_, SLOT(WebDavExportAvatar(Scene::EntityPtr)));
    }

    AvatarHandler::~AvatarHandler()
    {
    }

    Scene::EntityPtr AvatarHandler::GetOrCreateAvatarEntity(entity_id_t entityid, const RexUUID &fullid, bool *existing)
    {
        // Make sure scene exists
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
            return Scene::EntityPtr();

        Scene::EntityPtr entity = avatar_module_->GetAvatarEntity(entityid);
        if (entity)
        {
            *existing = true;
            return entity;
        }

        *existing = false;
        entity = CreateNewAvatarEntity(entityid);
        assert(entity.get());
        if (!entity)
            return entity;

        avatar_module_->RegisterFullId(fullid,entityid);
        EC_OpenSimPresence* presence = entity->GetComponent<EC_OpenSimPresence>().get();
        presence->localId = entityid; ///\note In current design it holds that localid == entityid, but I'm not sure if this will always be so?
        presence->agentId = fullid;

        // If we do have a pending appearance, apply it here
        if (pending_appearances_.find(fullid) != pending_appearances_.end())
        {
            std::string appearance = pending_appearances_[fullid];
            pending_appearances_.erase(pending_appearances_.find(fullid));
            EC_OpenSimAvatar* avatar = entity->GetComponent<EC_OpenSimAvatar>().get();
            avatar->SetAppearanceAddress(appearance,false);
            avatar_appearance_.DownloadAppearance(entity);
            AvatarModule::LogDebug("Used pending appearance " + appearance + " for new avatar");
        }

        return entity;
    }

    Scene::EntityPtr AvatarHandler::CreateNewAvatarEntity(entity_id_t entityid)
    {
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene || !avatar_module_->GetFramework()->GetComponentManager()->CanCreate(OgreRenderer::EC_OgrePlaceable::TypeNameStatic()))
            return Scene::EntityPtr();

        QStringList defaultcomponents;
        defaultcomponents.append(EC_OpenSimPresence::TypeNameStatic());
        defaultcomponents.append(EC_OpenSimAvatar::TypeNameStatic());
        defaultcomponents.append(EC_NetworkPosition::TypeNameStatic());
        defaultcomponents.append(EC_AvatarAppearance::TypeNameStatic());
        defaultcomponents.append(OgreRenderer::EC_OgrePlaceable::TypeNameStatic());
        //defaultcomponents.push_back(EC_HoveringText::TypeNameStatic());
#ifdef EC_HoveringWidget_ENABLED
        defaultcomponents.append(EC_HoveringWidget::TypeNameStatic());
#endif
        defaultcomponents.append(OgreRenderer::EC_Mesh::TypeNameStatic());
        defaultcomponents.append(OgreRenderer::EC_OgreAnimationController::TypeNameStatic());

        // Create avatar entity with all default components non-networked
        Scene::EntityPtr entity = scene->CreateEntity(entityid, defaultcomponents, AttributeChange::LocalOnly, false);
        scene->EmitEntityCreated(entity, AttributeChange::LocalOnly);

        ComponentPtr placeable = entity->GetComponent(OgreRenderer::EC_OgrePlaceable::TypeNameStatic());
        if (placeable)
        {
            //CreateNameOverlay(placeable, entityid);
            //ShowAvatarNameOverlay(entityid);
            CreateWidgetOverlay(placeable, entityid);
            CreateAvatarMesh(entityid);
            
            // Now switch networksync off from the placeable, before we do any damage
            placeable->SetNetworkSyncEnabled(false);
        }
        // Switch also networksync off from the mesh
        ComponentPtr mesh = entity->GetComponent(OgreRenderer::EC_Mesh::TypeNameStatic());
        if (mesh)
            mesh->SetNetworkSyncEnabled(false);

        return entity;
    }

    bool AvatarHandler::HandleOSNE_ObjectUpdate(ProtocolUtilities::NetworkEventInboundData* data)
    {
        ProtocolUtilities::NetInMessage &msg = *data->message;
        msg.ResetReading();

        uint64_t regionhandle = msg.ReadU64();
        msg.SkipToNextVariable(); ///\todo Unhandled inbound variable 'TimeDilation'.U16

        // Variable block: Object Data
        size_t instance_count = msg.ReadCurrentBlockInstanceCount();
        for(size_t i = 0; i < instance_count; ++i)
        {
            uint32_t localid = msg.ReadU32();
            msg.SkipToNextVariable();        ///\todo Unhandled inbound variable 'State' U8
            RexUUID fullid = msg.ReadUUID();
            msg.SkipToNextVariable();        ///\todo Unhandled inbound variable 'CRC' U#"
            uint8_t pcode = msg.ReadU8();

            bool existing_avatar = false;
            Scene::EntityPtr entity = GetOrCreateAvatarEntity(localid, fullid, &existing_avatar);
            if (!entity)
                return false;

            EC_OpenSimPresence* presence = entity->GetComponent<EC_OpenSimPresence>().get();
            EC_NetworkPosition* netpos = entity->GetComponent<EC_NetworkPosition>().get();

            presence->regionHandle = regionhandle;

            // Get position from objectdata
            msg.SkipToFirstVariableByName("ObjectData");
            size_t bytes_read = 0;
            const uint8_t *objectdatabytes = msg.ReadBuffer(&bytes_read);
            if (bytes_read >= 28)
            {
                // The data contents:
                // ofs 16 - pos xyz - 3 x float (3x4 bytes)
                netpos->position_ = *reinterpret_cast<const Vector3df*>(&objectdatabytes[16]);
                netpos->Updated();
            }

            msg.SkipToFirstVariableByName("ParentID");
            presence->parentId = msg.ReadU32();

            msg.SkipToFirstVariableByName("NameValue");
            QString namevalue = QString::fromUtf8(msg.ReadString().c_str());
            NameValueMap map = ParseNameValueMap(namevalue.toStdString());
            presence->SetFirstName(map["FirstName"]);
            presence->SetLastName(map["LastName"]);
            ///@note If using reX auth map["RexAuth"] contains the username and authentication address.

#ifdef EC_HoveringWidget_ENABLED
            // Hide own name overlay
            EC_HoveringWidget *overlay= entity->GetComponent<EC_HoveringWidget>().get();
            if(overlay)
            {
                overlay->SetText(presence->GetFullName().c_str());
                if (presence->agentId == avatar_module_->GetServerConnection()->GetInfo().agentID)
                    overlay->SetDisabled(true);
            }
#endif
            // If the server sent an ObjectUpdate on a prim that is actually the client's avatar, and if the Entity that 
            // corresponds to this prim doesn't yet have a Controllable component, add it to the Entity.
            // This also causes a EVENT_CONTROLLABLE_ENTITY to be passed which will register this Entity as the currently 
            // controlled avatar entity. -jj.
            ///\todo Perhaps this logic could be done beforehand when creating the avatar Entity instead of doing it here? -jj.
            if (presence->agentId == avatar_module_->GetServerConnection()->GetInfo().agentID &&
                !entity->GetComponent(EC_Controllable::TypeNameStatic()))
            {
                Foundation::Framework *fw = avatar_module_->GetFramework();
                assert (fw->GetComponentManager()->CanCreate(EC_Controllable::TypeNameStatic()));
                entity->AddComponent(fw->GetComponentManager()->CreateComponent(EC_Controllable::TypeNameStatic()));

#ifdef EC_SoundListener_ENABLED
                // Add sound listener EC for our own avatar.
                entity->AddComponent(fw->GetComponentManager()->CreateComponent(EC_SoundListener::TypeNameStatic()));
#endif

                Scene::Events::EntityEventData event_data;
                event_data.entity = entity;
                fw->GetEventManager()->SendEvent(fw->GetEventManager()->QueryEventCategory("Scene"), Scene::Events::EVENT_CONTROLLABLE_ENTITY, &event_data);

                // If avatar does not have appearance address yet, and the connection info has, then use it
                EC_OpenSimAvatar* avatar = entity->GetComponent<EC_OpenSimAvatar>().get();
                if (avatar->GetAppearanceAddress().empty())
                {
                    std::string avataraddress = avatar_module_->GetServerConnection()->GetInfo().avatarStorageUrl;
                    if (!avataraddress.empty())
                    {
                        avatar->SetAppearanceAddress(avataraddress,false);
                        avatar_appearance_.DownloadAppearance(entity);
                    }
                }
                
                // For some reason the avatar/connection ID might not be in sync before (when setting the appearance for first time),
                // which causes the edit view to not be initially rebuilt. Force build now
               avatar_module_->GetAvatarEditor()->RebuildEditView();
            }

            // Send event notifying about new user in the world
            Foundation::EventManagerPtr eventMgr = avatar_module_->GetFramework()->GetEventManager();
            if (!existing_avatar && presence->agentId != avatar_module_->GetServerConnection()->GetInfo().agentID)
            {
                ProtocolUtilities::UserConnectivityEvent event_data(presence->agentId);
                event_data.fullName = presence->GetFullName();
                event_data.localId = presence->localId;
                eventMgr->SendEvent("NetworkState", ProtocolUtilities::Events::EVENT_USER_CONNECTED, &event_data);
            }

            // Handle setting the avatar as child of another object, or possibly being parent itself
            Events::SceneHandleParentData data(localid);
            eventMgr->SendEvent("Avatar", Events::EVENT_HANDLE_AVATAR_PARENT, &data);

            msg.SkipToNextInstanceStart();
        }
        
        return false;
    }
    
    void AvatarHandler::HandleTerseObjectUpdate_30bytes(const uint8_t* bytes)
    {
        if (!framework_)
            return;
        if (!framework_->GetDefaultWorldScene().get())
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

        Scene::EntityPtr entity = avatar_module_->GetAvatarEntity(localid);
        if(!entity) return;
        EC_NetworkPosition* netpos = entity->GetComponent<EC_NetworkPosition>().get();

        Vector3df position = GetProcessedVector(&bytes[i]);
        i += sizeof(Vector3df);
        if (!IsValidPositionVector(position))
            return;
        
        netpos->velocity_ = GetProcessedScaledVectorFromUint16(&bytes[i],128);
        i += 6;
        
        Quaternion rotation = GetProcessedQuaternion(&bytes[i]);
     
        netpos->position_ = position;
        if (!entity->GetComponent(EC_Controllable::TypeNameStatic()))
        {
            // Do not update rotation for entities controlled by this client,
            // client handles the rotation for itself (jitters during turning may result otherwise).
            netpos->orientation_ = rotation;
        }

        //! \todo what to do with acceleration & rotation velocity? zero them currently
        netpos->accel_ = Vector3df::ZERO;
        netpos->rotvel_ = Vector3df::ZERO;
        netpos->Updated();
        assert(i <= 30);
    }

    void AvatarHandler::HandleTerseObjectUpdateForAvatar_60bytes(const uint8_t* bytes)
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
        Scene::EntityPtr entity = avatar_module_->GetAvatarEntity(localid);
        if(!entity)
            return;
        EC_NetworkPosition* netpos = entity->GetComponent<EC_NetworkPosition>().get();

        Vector3df position = GetProcessedVector(&bytes[i]);
        i += sizeof(Vector3df);

        if (!IsValidPositionVector(position))
            return;

        netpos->velocity_ = GetProcessedScaledVectorFromUint16(&bytes[i],128);
        i += 6;
        
        netpos->accel_ = GetProcessedVectorFromUint16(&bytes[i]);
        i += 6;

        Quaternion rotation = GetProcessedQuaternion(&bytes[i]);
        i += 8;

        netpos->rotvel_ = GetProcessedScaledVectorFromUint16(&bytes[i],128);
        
        netpos->position_ = position;
        if (!entity->GetComponent(EC_Controllable::TypeNameStatic()))
        {
            // Do not update rotation for entities controlled by this client, client handles the rotation for itself (jitters during turning may result otherwise).
            netpos->orientation_ = rotation;
        }

        netpos->Updated();
        assert(i <= 60);
    }

    bool AvatarHandler::HandleRexGM_RexAppearance(ProtocolUtilities::NetworkEventInboundData* data)
    {
        StringVector params = ProtocolUtilities::ParseGenericMessageParameters(*data->message);
        bool overrideappearance = false;

        if (params.size() >= 2)
        {
            std::string avataraddress = params[0];
            RexUUID avatarid(params[1]);

            if (params.size() >= 3)
                overrideappearance = ParseBool(params[2]);

            Scene::EntityPtr entity = avatar_module_->GetAvatarEntity(avatarid);
            if (entity)
            {
                EC_OpenSimAvatar* avatar = entity->GetComponent<EC_OpenSimAvatar>().get();
                avatar->SetAppearanceAddress(avataraddress,overrideappearance);
                avatar_appearance_.DownloadAppearance(entity);
            }
            else
            {
                pending_appearances_[avatarid] = avataraddress;
            }
        }

        return false;
    }

    bool AvatarHandler::HandleRexGM_RexAnim(ProtocolUtilities::NetworkEventInboundData* data)
    {
        StringVector params = ProtocolUtilities::ParseGenericMessageParameters(*data->message);

        if (params.size() < 7)
            return false;

        // Convert any ',' to '.'
        for (uint i = 0; i < params.size(); ++i)
            ReplaceCharInplace(params[i], ',', '.');

        RexUUID avatarid(params[0]);
        float rate = ParseString<float>(params[2], 1.0f);
        float fadein = ParseString<float>(params[3], 0.0f);
        float fadeout = ParseString<float>(params[4], 0.0f);
        int repeats = ParseString<int>(params[5], 1);
        bool stopflag = ParseBool(params[6]);

        if (repeats < 0)
            repeats = 0;

        Scene::EntityPtr entity = avatar_module_->GetAvatarEntity(avatarid);
        if (!entity)
            return false;
        OgreRenderer::EC_OgreAnimationController* anim = entity->GetComponent<OgreRenderer::EC_OgreAnimationController>().get(); 
        if (!anim)
            return false;

        if (!stopflag)
        {
            anim->EnableAnimation(params[1], false, fadein, true);
            anim->SetAnimationSpeed(params[1], rate);
            anim->SetAnimationAutoStop(params[1], true);
            anim->SetAnimationNumLoops(params[1], repeats);
        }
        else
        {
            anim->DisableAnimation(params[1], fadeout);
        }

        return false;
    }

    bool AvatarHandler::HandleOSNE_KillObject(uint32_t objectid)
    {
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
            return false;

        Scene::EntityPtr entity = scene->GetEntity(objectid);
        if(!entity)
            return false;

        RexUUID fullid;
        EC_OpenSimPresence* presence = entity->GetComponent<EC_OpenSimPresence>().get();
        if (presence)
        {
            fullid = presence->agentId;
            if (fullid != avatar_module_->GetServerConnection()->GetInfo().agentID)
            {
                // Send event notifying about user leaving the world
                Foundation::EventManagerPtr eventMgr = avatar_module_->GetFramework()->GetEventManager();
                ProtocolUtilities::UserConnectivityEvent event_data(presence->agentId);
                event_data.fullName = presence->GetFullName();
                event_data.localId = presence->localId;
                eventMgr->SendEvent("NetworkState", ProtocolUtilities::Events::EVENT_USER_DISCONNECTED, &event_data);
            }
        }

        scene->RemoveEntity(objectid);
        avatar_module_->UnregisterFullId(fullid);
        return false;
    }
   
    bool AvatarHandler::HandleOSNE_AvatarAnimation(ProtocolUtilities::NetworkEventInboundData* data)
    {
        ProtocolUtilities::NetInMessage &msg = *data->message;
        msg.ResetReading();
        RexUUID avatarid = msg.ReadUUID();
     
        std::vector<RexUUID> animations_to_start;
        size_t animlistcount = msg.ReadCurrentBlockInstanceCount();
        for(size_t i = 0; i < animlistcount; i++)
        {
            RexUUID animid = msg.ReadUUID();
            s32 animsequence = msg.ReadS32();

            animations_to_start.push_back(animid);
            
            if(avatar_states_.find(animid) != avatar_states_.end())
            {
                // Set avatar state based on animation: not probably best way, but possibly acceptable for now
                SetAvatarState(avatarid, avatar_states_[animid]);
            }
        }
        
        size_t animsourcelistcount = msg.ReadCurrentBlockInstanceCount();
        for(size_t i = 0; i < animsourcelistcount; i++)
            RexUUID objectid = msg.ReadUUID();  

        // PhysicalAvatarEventList not used

        StartAvatarAnimations(avatarid, animations_to_start);

        return false;
    }

    void AvatarHandler::Update(f64 frametime)
    {
        avatar_appearance_.Update(frametime);
    }

    void AvatarHandler::CreateWidgetOverlay(ComponentPtr placeable, entity_id_t entity_id)
    {
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
            return;

        Scene::EntityPtr entity = scene->GetEntity(entity_id);
        if (!entity)
            return;

#ifdef EC_HoveringWidget_ENABLED
        EC_HoveringWidget* overlay = entity->GetComponent<EC_HoveringWidget>().get();
        EC_OpenSimPresence* presence = entity->GetComponent<EC_OpenSimPresence>().get();
        if (overlay && presence)
        {
            overlay->InitializeBillboards();
            overlay->SetText(presence->GetFullName().c_str());
            /*overlay->AddButton(*(new QPushButton("Poke")));
            overlay->AddButton(*(new QPushButton("Chat")));
            overlay->AddButton(*(new QPushButton("Mute")));
            overlay->AddButton(*(new QPushButton("Follow")));*/
        }
#endif
    }
/*
    void AvatarHandler::CreateNameOverlay(ComponentPtr placeable, entity_id_t entity_id)
    {
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
            return;

        Scene::EntityPtr entity = scene->GetEntity(entity_id);
        if (!entity)
            return;

        // Ali: testing EC_HoveringText instead of EC_OgreMovableTextOverlay
        EC_HoveringText* overlay = entity->GetComponent<EC_HoveringText>().get();
        //OgreRenderer::EC_OgreMovableTextOverlay* overlay = entity->GetComponent<OgreRenderer::EC_OgreMovableTextOverlay>().get();
        EC_OpenSimPresence* presence = entity->GetComponent<EC_OpenSimPresence>().get();
        if (overlay && presence)
        {
//            overlay->CreateOverlay(Vector3df(0.0f, 0.0f, 1.5f));
//            overlay->SetText(presence->GetFullName());
//            overlay->SetPlaceable(placeable);
            //overlay->SetTextColor(QColor(255,255,255,255));
            //overlay->SetBackgroundColor(QColor(0,0,0,200));
            overlay->ShowMessage(presence->GetFullName().c_str());
        }

       
    }
*/

    void AvatarHandler::ShowAvatarNameOverlay(entity_id_t entity_id, bool visible)
    {
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
            return;

        Scene::EntityPtr entity = scene->GetEntity(entity_id);
        if (!entity)
            return;

        // Ali: testing EC_HoveringText instead of EC_OgreMovableTextOverlay
/*
        OgreRenderer::EC_OgreMovableTextOverlay* overlay = entity->GetComponent<OgreRenderer::EC_OgreMovableTextOverlay>().get();
        EC_OpenSimPresence* presence = entity->GetComponent<EC_OpenSimPresence>().get();
        if (overlay && presence)
        {
            overlay->SetText(presence->GetFullName());
            overlay->SetVisible(true);
        }
*/
#ifdef EC_HoveringText_ENABLED
        EC_HoveringText* overlay = entity->GetComponent<EC_HoveringText>().get();
        EC_OpenSimPresence* presence = entity->GetComponent<EC_OpenSimPresence>().get();
        if (overlay && presence)
        {
            overlay->SetTextColor(QColor(255,255,255,230));
            overlay->SetBackgroundGradient(QColor(0,0,0,230), QColor(50,50,50,230));
            overlay->ShowMessage(presence->GetFullName().c_str());
            if (!visible)
                overlay->Hide();
        }
#endif
    }
    
    void AvatarHandler::CreateAvatarMesh(entity_id_t entity_id)
    {
        using namespace OgreRenderer;

        Scene::EntityPtr entity = avatar_module_->GetAvatarEntity(entity_id);
        if (!entity)
            return;

        ComponentPtr placeableptr = entity->GetComponent(EC_OgrePlaceable::TypeNameStatic());
        ComponentPtr meshptr = entity->GetComponent(EC_Mesh::TypeNameStatic());
        ComponentPtr animctrlptr = entity->GetComponent(EC_OgreAnimationController::TypeNameStatic());
        
        if (placeableptr && meshptr)
        {
            EC_Mesh* mesh = checked_static_cast<EC_Mesh*>(meshptr.get());
            mesh->SetPlaceable(placeableptr);
            avatar_appearance_.SetupDefaultAppearance(entity);
        }
        
        if (animctrlptr && meshptr)
        {
            EC_OgreAnimationController* animctrl = checked_static_cast<EC_OgreAnimationController*>(animctrlptr.get());
            animctrl->SetMeshEntity(dynamic_cast<EC_Mesh*>(meshptr.get()));
        }
    }
    
    void AvatarHandler::StartAvatarAnimations(const RexUUID& avatarid, const std::vector<RexUUID>& anim_ids)
    {
        using namespace OgreRenderer;

        Scene::EntityPtr entity = avatar_module_->GetAvatarEntity(avatarid);
        if (!entity)
            return;

        EC_OgreAnimationController* animctrl = entity->GetComponent<EC_OgreAnimationController>().get();
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!animctrl || !appearance)
            return;
        
        const AnimationDefinitionMap& anim_defs = appearance->GetAnimations();
        
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
        const EC_OgreAnimationController::AnimationMap& running_anims = animctrl->GetRunningAnimations();
        EC_OgreAnimationController::AnimationMap::const_iterator anim = running_anims.begin();
        while (anim != running_anims.end())
        {
            if (std::find(anims_to_start.begin(), anims_to_start.end(), anim->first) == anims_to_start.end())
                anims_to_stop.push_back(anim->first);
            ++anim;
        }
        
        for (unsigned i = 0; i < anims_to_start.size(); ++i)
        {
            const AnimationDefinition& def = GetAnimationByName(anim_defs, anims_to_start[i]);

            animctrl->EnableAnimation(def.animation_name_, def.looped_, def.fadein_);
            animctrl->SetAnimationSpeed(def.animation_name_, def.speedfactor_);
            animctrl->SetAnimationWeight(def.animation_name_, def.weightfactor_);

            if (def.always_restart_)
                animctrl->SetAnimationTimePosition(def.animation_name_, 0.0);
        }

        for (unsigned i = 0; i < anims_to_stop.size(); ++i)
        {
            const AnimationDefinition& def = GetAnimationByName(anim_defs, anims_to_stop[i]);
            animctrl->DisableAnimation(def.animation_name_, def.fadeout_);
        }
    }

    void AvatarHandler::UpdateAvatarAnimations(entity_id_t avatarid, f64 frametime)
    {
        using namespace OgreRenderer;
        Scene::EntityPtr entity = avatar_module_->GetAvatarEntity(avatarid);
        if (!entity)
            return;

        EC_OgreAnimationController* animctrl = entity->GetComponent<EC_OgreAnimationController>().get();
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        EC_NetworkPosition* netpos = entity->GetComponent<EC_NetworkPosition>().get();
        if (!animctrl || !netpos || !appearance)
            return;
        
        const AnimationDefinitionMap& anim_defs = appearance->GetAnimations();
        
        const EC_OgreAnimationController::AnimationMap& running_anims = animctrl->GetRunningAnimations();
        EC_OgreAnimationController::AnimationMap::const_iterator anim = running_anims.begin();
        while (anim != running_anims.end())
        {
            const AnimationDefinition& def = GetAnimationByName(anim_defs, anim->first);
            // If animation is velocity-adjusted, adjust animation speed by network position speed (horizontal plane movement only)
            if (def.use_velocity_)
            {
                float speed = Vector3df(netpos->velocity_.x, netpos->velocity_.y, 0).getLength() * 0.5;
                animctrl->SetAnimationSpeed(anim->first, def.speedfactor_ * speed);
            }
            ++anim;
        }
    }
    
    void AvatarHandler::SetAvatarState(const RexUUID& avatarid, EC_OpenSimAvatar::State state)
    {
        Scene::EntityPtr entity = avatar_module_->GetAvatarEntity(avatarid);
        if (!entity)
            return;
        EC_OpenSimAvatar* avatar = entity->GetComponent<EC_OpenSimAvatar>().get();
        if (!avatar)
            return;
            
        avatar->SetState(state);
    }
    
    bool AvatarHandler::HandleResourceEvent(event_id_t event_id, IEventData* data)
    {
        return avatar_appearance_.HandleResourceEvent(event_id, data);
    }

    bool AvatarHandler::HandleInventoryEvent(event_id_t event_id, IEventData* data)
    {
        return avatar_appearance_.HandleInventoryEvent(event_id, data);
    }

    bool AvatarHandler::HandleAssetEvent(event_id_t event_id, IEventData* data)
    {
        return avatar_appearance_.HandleAssetEvent(event_id, data);
    }

    Scene::EntityPtr AvatarHandler::GetUserAvatar() const
    {
        ProtocolUtilities::WorldStreamPtr conn = avatar_module_->GetServerConnection();
        if (!conn)
            return Scene::EntityPtr();
        else
            return avatar_module_->GetAvatarEntity(conn->GetInfo().agentID);
    }

    bool AvatarHandler::AvatarExportSupported()
    {
        Scene::EntityPtr entity = GetUserAvatar();
        if (!entity)
            return false;
        ProtocolUtilities::WorldStreamPtr conn = avatar_module_->GetServerConnection();
        if (!conn)
            return false;
        
        // Support anything that has the appearance url set, webdav or legacy
        EC_OpenSimAvatar *os_avatar = entity->GetComponent<EC_OpenSimAvatar>().get();
        if (os_avatar)
            if (os_avatar->GetAppearanceAddress().length() > 0)
                return true;

        // For now, support only legacy storage export
        return (conn->GetConnectionType() == ProtocolUtilities::AuthenticationConnection);
    }

    void AvatarHandler::ExportUserAvatar()
    {
        Scene::EntityPtr entity = GetUserAvatar();
        if (!entity)
        {
            AvatarModule::LogError("User avatar not in scene, cannot export");
            return;
        }
        
        // See whether to use legacy storage or inventory
        ProtocolUtilities::WorldStreamPtr conn = avatar_module_->GetServerConnection();
        if (!conn)
        {
            AvatarModule::LogError("Not connected to server, cannot export avatar");
            return;
        }
        
        if (!AvatarExportSupported())
        {
            AvatarModule::LogError("Avatar export supported to legacy storage only for now");
            return;
        }
        
        // Legacy avatar
        if (conn->GetAuthenticationType() == ProtocolUtilities::AT_RealXtend)
        {
            avatar_appearance_.EmitAppearanceStatus("Exporting avatar to avatar storage, please wait...", 120000);
            emit ExportAvatar(entity, conn->GetUsername(), conn->GetAuthAddress(), conn->GetPassword());
        }
        // Webdav avatar
        else if (conn->GetAuthenticationType() == ProtocolUtilities::AT_Taiga)
        {
            avatar_appearance_.EmitAppearanceStatus("Exporting avatar to webdav inventory, please wait...", 120000);
            QApplication::processEvents();
            emit WebDavExportAvatar(entity);
        }
        // Inventory avatar
        else
            avatar_appearance_.InventoryExportAvatar(entity);
    }

    void AvatarHandler::ExportUserAvatarLocal(const std::string& filename)
    {
        Scene::EntityPtr entity = GetUserAvatar();
        if (!entity)
        {
            AvatarModule::LogError("User avatar not in scene, cannot export locally");
            return;
        }
        
        avatar_appearance_.ExportAvatarLocal(entity, filename);
    }
    
    void AvatarHandler::ReloadUserAvatar()
    {
        Scene::EntityPtr entity = GetUserAvatar();
        if (!entity)
        {
            AvatarModule::LogError("User avatar not in scene, cannot reload appearance");
            return;
        }
        
        // Revert to default if no storage url
        avatar_appearance_.DownloadAppearance(entity, true);
    }
    
    void AvatarHandler::HandleLogout()
    {
        avatar_appearance_.InventoryExportReset();
        pending_appearances_.clear();
    }
}
