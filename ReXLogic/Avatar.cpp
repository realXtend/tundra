// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexObject.h"
#include "Avatar.h"
#include "RexLogicModule.h"
#include "EC_openSimAvatar.h"
#include "../OgreRenderingModule/EC_OgrePlaceable.h"
#include "../OgreRenderingModule/Renderer.h"
#include "ConversionUtils.h"

namespace RexLogic
{
    Avatar::Avatar(RexLogicModule *rexlogicmodule) : RexObject(rexlogicmodule)
    {

    }
    
    Avatar::~Avatar()
    {
    }
    
    Foundation::EntityPtr Avatar::GetOrCreateAvatarEntity(Core::entity_id_t entityid, const RexUUID &fullid)
    {
        // Make sure scene exists
        Foundation::ScenePtr scene = rexlogicmodule_->GetCurrentActiveScene();
        if (!scene)
            return Foundation::EntityPtr();

        Foundation::EntityPtr entity = rexlogicmodule_->GetAvatarEntity(entityid);
        if (!entity)
        {
            entity = CreateNewAvatarEntity(entityid);
            rexlogicmodule_->RegisterFullId(fullid,entityid); 
            EC_OpenSimAvatar &avatar = *checked_static_cast<EC_OpenSimAvatar*>(entity->GetComponent("EC_OpenSimAvatar").get());
            avatar.LocalId = entityid; ///\note In current design it holds that localid == entityid, but I'm not sure if this will always be so?
            avatar.FullId = fullid;
        }
        return entity;
    }    

    Foundation::EntityPtr Avatar::CreateNewAvatarEntity(Core::entity_id_t entityid)
    {
        Foundation::ScenePtr scene = rexlogicmodule_->GetCurrentActiveScene();
        if (!scene)
            return Foundation::EntityPtr();
        
        Core::StringVector defaultcomponents;
        defaultcomponents.push_back(EC_OpenSimAvatar::NameStatic());
        defaultcomponents.push_back(OgreRenderer::EC_OgrePlaceable::NameStatic());        
        
        Foundation::EntityPtr entity = scene->CreateEntity(entityid,defaultcomponents);
 
        OgreRenderer::EC_OgrePlaceable &ogrePos = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(entity->GetComponent("EC_OgrePlaceable").get());
        ogrePos.SetScale(Vector3(0.5,1.5,0.5));
        DebugCreateOgreBoundingBox(entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()),"AmbientGreen");
 
        return entity;
    } 

    bool Avatar::HandleOSNE_ObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data)
    {
        NetInMessage *msg = data->message;
 
        msg->ResetReading();
        uint64_t regionhandle = msg->ReadU64();
        msg->SkipToNextVariable(); // TimeDilation U16 ///\todo Unhandled inbound variable 'TimeDilation'.

        uint32_t localid = msg->ReadU32(); 
        msg->SkipToNextVariable();		// State U8 ///\todo Unhandled inbound variable 'State'.
        RexUUID fullid = msg->ReadUUID();
        msg->SkipToNextVariable();		// CRC U32 ///\todo Unhandled inbound variable 'CRC'.
        uint8_t pcode = msg->ReadU8();

        Foundation::EntityPtr entity = GetOrCreateAvatarEntity(localid,fullid);
        EC_OpenSimAvatar &avatar = *checked_static_cast<EC_OpenSimAvatar*>(entity->GetComponent("EC_OpenSimAvatar").get());
        OgreRenderer::EC_OgrePlaceable &ogrePos = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(entity->GetComponent("EC_OgrePlaceable").get());

        avatar.RegionHandle = regionhandle;
        
        // Get position from objectdata
        msg->SkipToFirstVariableByName("ObjectData");
        size_t bytes_read = 0;
        const uint8_t *objectdatabytes = msg->ReadBuffer(&bytes_read);
        if (bytes_read >= 28)
        {
            // The data contents:
            // ofs 16 - pos xyz - 3 x float (3x4 bytes)
            Core::Vector3df pos = *reinterpret_cast<const Core::Vector3df*>(&objectdatabytes[16]);
            ogrePos.SetPosition(Core::OpenSimToOgreCoordinateAxes(pos));
        }                
        
        msg->SkipToFirstVariableByName("ParentID");
        avatar.ParentId = msg->ReadU32();
        
        // NameValue contains: FirstName STRING RW SV " + firstName + "\nLastName STRING RW SV " + lastName
        msg->SkipToFirstVariableByName("NameValue");
        std::string namevalue = msg->ReadString();
        size_t pos = namevalue.find("\n");
        if(pos != std::string::npos)
        {
            avatar.FirstName = namevalue.substr(23,pos-23);
            avatar.LastName = namevalue.substr(pos+23);
        }
        
        // Set own avatar
        if (avatar.FullId == rexlogicmodule_->GetServerConnection()->GetInfo().agentID)
            rexlogicmodule_->GetAvatarController()->SetAvatarEntity(entity);              

        return false;
    }
    
    void Avatar::HandleTerseObjectUpdate_30bytes(const uint8_t* bytes)
    {
        // The data contents:
        // ofs  0 - localid - packed to 4 bytes
        // ofs  4 - position xyz - 3 x float (3x4 bytes)
        // ofs 16 - velocity xyz - packed to 6 bytes
        // ofs 22 - rotation - packed to 8 bytes
        
        //! \todo handle endians
        int i = 0;
        uint32_t localid = *reinterpret_cast<uint32_t*>((uint32_t*)&bytes[i]);                
        i += 4;

        Core::Vector3df position = GetProcessedVector(&bytes[i]);
        i += sizeof(Core::Vector3df);
        
        Core::Vector3df velocity = GetProcessedScaledVectorFromUint16(&bytes[i],128);
        i += 6;
        
        Core::Quaternion rotation = GetProcessedQuaternion(&bytes[i]);

        Foundation::EntityPtr entity = rexlogicmodule_->GetAvatarEntity(localid);
        if(entity)
        {
            /// \todo tucofixme handle velocity        
            if(rexlogicmodule_->GetAvatarController()->GetAvatarEntity() && entity->GetId() == rexlogicmodule_->GetAvatarController()->GetAvatarEntity()->GetId())
                rexlogicmodule_->GetAvatarController()->HandleServerObjectUpdate(position,rotation);
            else
            {
                OgreRenderer::EC_OgrePlaceable &ogrePos = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(entity->GetComponent("EC_OgrePlaceable").get());
                ogrePos.SetPosition(position);
                ogrePos.SetOrientation(rotation);                    
            }
        }
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

        Core::Vector3df position = GetProcessedVector(&bytes[i]);
        i += sizeof(Core::Vector3df);

        Core::Vector3df velocity = GetProcessedScaledVectorFromUint16(&bytes[i],128);
        i += 6;
        
        Core::Vector3df accel = GetProcessedVectorFromUint16(&bytes[i]);
        i += 6;

        Core::Quaternion rotation = GetProcessedQuaternion(&bytes[i]);
        i += 8;        

        Core::Vector3df rotvel = GetProcessedVectorFromUint16(&bytes[i]);

        // set values
        Foundation::EntityPtr entity = rexlogicmodule_->GetAvatarEntity(localid);
        if(entity)
        {
            OgreRenderer::EC_OgrePlaceable &ogrePos = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(entity->GetComponent("EC_OgrePlaceable").get());
            ogrePos.SetPosition(position);
            ogrePos.SetOrientation(rotation);                    
        }
    }
        
    bool Avatar::HandleRexGM_RexAppearance(OpenSimProtocol::NetworkEventInboundData* data)
    {
        return false;
        
        ///\todo tucofixme, Crashes!
        data->message->ResetReading();    
        data->message->SkipToFirstVariableByName("Parameter");
        
        std::string avataraddress = data->message->ReadString();
        RexUUID avatarid(data->message->ReadString());
        
        //! \todo tucofixme, parse bool
        // bool overrideappearance = Core::ParseString<bool>(data->message->ReadString());
        return false;
    }
    
    bool Avatar::HandleOSNE_KillObject(uint32_t objectid)
    {
        Foundation::ScenePtr scene = rexlogicmodule_->GetCurrentActiveScene();
        if (!scene)
            return false;

        RexTypes::RexUUID fullid;
        fullid.SetNull();
        Foundation::EntityPtr entity = scene->GetEntity(objectid);
        if(!entity)
            return false;

        Foundation::ComponentPtr component = entity->GetComponent("EC_OpenSimAvatar");
        if(component)
        {
            EC_OpenSimAvatar &avatar = *checked_static_cast<EC_OpenSimAvatar*>(component.get());
            fullid = avatar.FullId;
        }
        
        scene->DestroyEntity(objectid);
        rexlogicmodule_->UnregisterFullId(fullid);
        return false;
    }     
}