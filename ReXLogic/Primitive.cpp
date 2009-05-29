// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Primitive.h"
#include "RexNetworkUtils.h"
#include "RexLogicModule.h"
#include "EC_OpenSimPrim.h"
#include "EC_NetworkPosition.h"
#include "EC_Viewable.h"
#include "../OgreRenderingModule/EC_OgrePlaceable.h"
#include "../OgreRenderingModule/EC_OgreMesh.h"
#include "../OgreRenderingModule/EC_OgreCustomObject.h"
#include "../OgreRenderingModule/EC_OgreLight.h"
#include "../OgreRenderingModule/OgreMeshResource.h"
#include "../OgreRenderingModule/OgreTextureResource.h"
#include "../OgreRenderingModule/OgreMaterialResource.h"
#include "../OgreRenderingModule/OgreMaterialUtils.h"
#include "../OgreRenderingModule/Renderer.h"
#include "ConversionUtils.h"
#include "QuatUtils.h"
#include "SceneEvents.h"
#include "ResourceInterface.h"
#include "PrimGeometryUtils.h"
#include "SceneManager.h"

namespace RexLogic
{

    Primitive::Primitive(RexLogicModule *rexlogicmodule)
    {
        rexlogicmodule_ = rexlogicmodule;
    }

    Primitive::~Primitive()
    {
    }

    Scene::EntityPtr Primitive::GetOrCreatePrimEntity(Core::entity_id_t entityid, const RexUUID &fullid)
    {
        // Make sure scene exists
        Scene::ScenePtr scene = rexlogicmodule_->GetCurrentActiveScene();
        if (!scene)
            return Scene::EntityPtr();
          
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
        if (!entity)
        {
            // Create a new entity.
            Scene::EntityPtr entity = CreateNewPrimEntity(entityid);
            rexlogicmodule_->RegisterFullId(fullid,entityid); 
            EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());
            prim.LocalId = entityid; ///\note In current design it holds that localid == entityid, but I'm not sure if this will always be so?
            prim.FullId = fullid;
            CheckPendingRexPrimData(entityid);
            return entity;
        }

        // Send the 'Entity Updated' event.
        /*
        Core::event_category_id_t cat_id = framework_->GetEventManager()->QueryEventCategory("Scene");
        Foundation::ComponentInterfacePtr component = entity->GetComponent(EC_OpenSimPrim::NameStatic());
        EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(component.get());
        Scene::SceneEventData::Events entity_event_data(entityid);
        entity_event_data.sceneName = scene->Name();
        framework_->GetEventManager()->SendEvent(cat_id, Scene::Events::EVENT_ENTITY_UPDATED, &entity_event_data);
        */
        return entity;
    }  

    Scene::EntityPtr Primitive::CreateNewPrimEntity(Core::entity_id_t entityid)
    {
        Scene::ScenePtr scene = rexlogicmodule_->GetCurrentActiveScene();
        if (!scene)
            return Scene::EntityPtr();
        
        Core::StringVector defaultcomponents;
        defaultcomponents.push_back(EC_OpenSimPrim::NameStatic());
        defaultcomponents.push_back(EC_NetworkPosition::NameStatic());
        defaultcomponents.push_back(OgreRenderer::EC_OgrePlaceable::NameStatic());
                
        Scene::EntityPtr entity = scene->CreateEntity(entityid,defaultcomponents); 

        DebugCreateOgreBoundingBox(rexlogicmodule_, entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()),"AmbientRed");
        return entity;
    }    

    bool Primitive::HandleOSNE_ObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data)
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

            Scene::EntityPtr entity = GetOrCreatePrimEntity(localid, fullid);
            EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());
            EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(entity->GetComponent(EC_NetworkPosition::NameStatic()).get());

            ///\todo Are we setting the param or looking up by this param? I think the latter, but this is now doing the former. 
            ///      Will cause problems with multigrid support.
            prim.RegionHandle = regionhandle;

            prim.Material = msg->ReadU8();
            prim.ClickAction = msg->ReadU8();

            prim.Scale = Core::OpenSimToOgreCoordinateAxes(msg->ReadVector3());
            // Scale is not handled by interpolation system, so set directly
            HandlePrimScale(localid);
            
            size_t bytes_read = 0;
            const uint8_t *objectdatabytes = msg->ReadBuffer(&bytes_read);
            if (bytes_read == 60)
            {
                Core::Vector3Df ogre_pos;
                Core::Quaternion ogre_quat;

                // The data contents:
                // ofs  0 - pos xyz - 3 x float (3x4 bytes)
                // ofs 12 - vel xyz - 3 x float (3x4 bytes)
                // ofs 24 - acc xyz - 3 x float (3x4 bytes)
                // ofs 36 - orientation, quat with last (w) component omitted - 3 x float (3x4 bytes)
                // ofs 48 - angular velocity - 3 x float (3x4 bytes)
                // total 60 bytes
                
                netpos.position_ = Core::OpenSimToOgreCoordinateAxes(*reinterpret_cast<const Core::Vector3df*>(&objectdatabytes[0]));                
                netpos.velocity_ = Core::OpenSimToOgreCoordinateAxes(*reinterpret_cast<const Core::Vector3df*>(&objectdatabytes[12])); 
                netpos.accel_ = Core::OpenSimToOgreCoordinateAxes(*reinterpret_cast<const Core::Vector3df*>(&objectdatabytes[24]));
                netpos.rotation_ = Core::OpenSimToOgreQuaternion(Core::UnpackQuaternionFromFloat3((float*)&objectdatabytes[36])); 
                netpos.rotvel_ = Core::OpenSimToOgreCoordinateAxes(*reinterpret_cast<const Core::Vector3df*>(&objectdatabytes[48]));
                netpos.Updated();
            }
            else
                RexLogicModule::LogError("Error reading ObjectData for prim:" + Core::ToString(prim.LocalId) + ". Bytes read:" + Core::ToString(bytes_read));
            
            prim.ParentId = msg->ReadU32();
            prim.UpdateFlags = msg->ReadU32();
            
            // Read prim shape
            prim.PathCurve = msg->ReadU8();
            prim.ProfileCurve = msg->ReadU8();
            prim.PathBegin = msg->ReadU16() * 0.00002f;
            prim.PathEnd = msg->ReadU16() * 0.00002f;
            prim.PathScaleX = msg->ReadU8() * 0.01f;
            prim.PathScaleY = msg->ReadU8() * 0.01f;
            prim.PathShearX = ((int8_t)msg->ReadU8()) * 0.01f;
            prim.PathShearY = ((int8_t)msg->ReadU8()) * 0.01f;
            prim.PathTwist = msg->ReadS8() * 0.01f;
            prim.PathTwistBegin = msg->ReadS8() * 0.01f;
            prim.PathRadiusOffset = msg->ReadS8() * 0.01f;
            prim.PathTaperX = msg->ReadS8() * 0.01f;
            prim.PathTaperY = msg->ReadS8() * 0.01f;
            prim.PathRevolutions = 1.0f + msg->ReadU8() * 0.015f;
            prim.PathSkew = msg->ReadS8() * 0.01f;
            prim.ProfileBegin = msg->ReadU16() * 0.00002f;
            prim.ProfileEnd = msg->ReadU16() * 0.00002f;
            prim.ProfileHollow = msg->ReadU16() * 0.00002f;
            prim.HasPrimShapeData = true;
            
            // Texture entry
            const uint8_t *textureentrybytes = msg->ReadBuffer(&bytes_read);
            ParseTextureEntryData(prim, textureentrybytes, bytes_read);
            
            // Skip to prim text
            msg->SkipToFirstVariableByName("Text");
            prim.HoveringText = msg->ReadString(); 
            msg->SkipToNextVariable();      // TextColor
            prim.MediaUrl = msg->ReadString();
            msg->SkipToNextVariable();      // PSBlock
            
            // If there are extra params, handle them.
            if (msg->ReadVariableSize() > 1)
            {
                const uint8_t *extra_params_data = msg->ReadBuffer(&bytes_read);
                HandleExtraParams(localid, extra_params_data);
            }
            
            msg->SkipToFirstVariableByName("JointAxisOrAnchor");
            msg->SkipToNextVariable(); // To next instance
            
            HandleDrawType(localid);
        }
        
        return false;
    }
    
    void Primitive::HandleTerseObjectUpdateForPrim_60bytes(const uint8_t* bytes)
    {
        // The data contents:
        // ofs  0 - localid - packed to 4 bytes
        // ofs  4 - state
        // ofs  5 - 0
        // ofs  6 - position xyz - 3 x float (3x4 bytes)
        // ofs 18 - velocity xyz - packed to 6 bytes        
        // ofs 24 - acceleration xyz - packed to 6 bytes           
        // ofs 30 - rotation - packed to 8 bytes 
        // ofs 38 - rotational vel - packed to 6 bytes
        
        //! \todo handle endians
        int i = 0;
        uint32_t localid = *reinterpret_cast<uint32_t*>((uint32_t*)&bytes[i]);                
        i += 6;
        
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(localid);
        if(!entity) return;
        EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(entity->GetComponent(EC_NetworkPosition::NameStatic()).get());
        
        netpos.position_ = GetProcessedVector(&bytes[i]);
        i += sizeof(Core::Vector3df);
        
        netpos.velocity_ = GetProcessedScaledVectorFromUint16(&bytes[i],128);
        i += 6;
        
        netpos.accel_ = GetProcessedVectorFromUint16(&bytes[i]); 
        i += 6;

        netpos.rotation_ = GetProcessedQuaternion(&bytes[i]);
        i += 8;

        netpos.rotvel_ = GetProcessedScaledVectorFromUint16(&bytes[i],128);
        
        netpos.Updated();
    }
    
    bool Primitive::HandleRexGM_RexMediaUrl(OpenSimProtocol::NetworkEventInboundData* data)
    {
        /// \todo tucofixme
        return false;
    }    

    bool Primitive::HandleRexGM_RexPrimData(OpenSimProtocol::NetworkEventInboundData* data)
    {
        std::vector<Core::u8> fulldata;
        RexUUID primuuid;
        
        data->message->ResetReading();
        data->message->SkipToFirstVariableByName("Parameter");
        
        // Variable block begins
        size_t instance_count = data->message->ReadCurrentBlockInstanceCount();
        size_t read_instances = 0;
        
        // First instance contains the UUID.
        primuuid.FromString(data->message->ReadString());
        ++read_instances;
                
        // Calculate full data size
        size_t fulldatasize = data->message->GetDataSize();
        size_t bytes_read = data->message->BytesRead();
        fulldatasize -= bytes_read;
        
        // Allocate memory block
        fulldata.resize(fulldatasize);
        int offset = 0;
        
        // Read the binary data.
        // The first instance contains always the UUID and rest of instances contain only binary data.
        // Data for multiple objects are never sent in the same message. All of the necessary data fits in one message.
        // Read the data:
        while((data->message->BytesRead() < data->message->GetDataSize()) && (read_instances < instance_count))
        {
            const Core::u8* readbytedata = data->message->ReadBuffer(&bytes_read);
            memcpy(&fulldata[offset], readbytedata, bytes_read);
            offset += bytes_read;
            ++read_instances;
        }

        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(primuuid);
        // If cannot get the entity, put to pending rexprimdata
        if (entity)           
            HandleRexPrimDataBlob(entity->GetId(), &fulldata[0]);
        else
            pending_rexprimdata_[primuuid] = fulldata;
            
        return false;
    }

    void Primitive::CheckPendingRexPrimData(Core::entity_id_t entityid)
    {
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
        if (!entity) return;
        EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());
               
        RexPrimDataMap::iterator i = pending_rexprimdata_.find(prim.FullId);
        if (i != pending_rexprimdata_.end())
        {
            HandleRexPrimDataBlob(entityid, &i->second[0]);
            pending_rexprimdata_.erase(i);
        }
    }

    void Primitive::HandleRexPrimDataBlob(Core::entity_id_t entityid, const uint8_t* primdata)
    {
        int idx = 0;

        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
        if (!entity)
            return;
            
        EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());

        // graphical values
        prim.DrawType = ReadUInt8FromBytes(primdata,idx);
        prim.IsVisible = ReadBoolFromBytes(primdata,idx);
        prim.CastShadows = ReadBoolFromBytes(primdata,idx);
        prim.LightCreatesShadows = ReadBoolFromBytes(primdata,idx);
        prim.DescriptionTexture = ReadBoolFromBytes(primdata,idx);    
        prim.ScaleToPrim = ReadBoolFromBytes(primdata,idx);
        prim.DrawDistance = ReadFloatFromBytes(primdata,idx);
        prim.LOD = ReadFloatFromBytes(primdata,idx);

        prim.MeshUUID = ReadUUIDFromBytes(primdata,idx);
        prim.CollisionMesh = ReadUUIDFromBytes(primdata,idx);      
        
        prim.ParticleScriptUUID = ReadUUIDFromBytes(primdata,idx);

        // animation
        prim.AnimationPackageUUID = ReadUUIDFromBytes(primdata,idx);        
        prim.AnimationName = ReadNullTerminatedStringFromBytes(primdata,idx);
        prim.AnimationRate = ReadFloatFromBytes(primdata,idx);

        MaterialMap materials;
        uint8_t tempmaterialindex = 0; 
        uint8_t tempmaterialcount = ReadUInt8FromBytes(primdata,idx);
        for(int i=0;i<tempmaterialcount;i++)
        {
            MaterialData newmaterialdata;

            newmaterialdata.Type = ReadUInt8FromBytes(primdata,idx);
            newmaterialdata.UUID = ReadUUIDFromBytes(primdata,idx);
            tempmaterialindex = ReadUInt8FromBytes(primdata,idx);
            materials[tempmaterialindex] = newmaterialdata;
        }
        prim.Materials = materials;

        prim.ServerScriptClass = ReadNullTerminatedStringFromBytes(primdata,idx);
  
        // sound
        prim.SoundUUID = ReadUUIDFromBytes(primdata,idx);       
        prim.SoundVolume = ReadFloatFromBytes(primdata,idx);
        prim.SoundRadius = ReadFloatFromBytes(primdata,idx);               

        prim.SelectPriority = ReadUInt32FromBytes(primdata,idx);
   
        // Handle any change in the drawtype of the prim. Also, 
        // the Ogre materials on this prim have possibly changed. Issue requests of the new materials 
        // from the asset provider and bind the new materials to this prim.
        HandleDrawType(entityid);
        HandlePrimScale(entityid);
    }
    
    bool Primitive::HandleOSNE_KillObject(uint32_t objectid)
    {
        Scene::ScenePtr scene = rexlogicmodule_->GetCurrentActiveScene();
        if (!scene)
            return false;

        RexTypes::RexUUID fullid;
        fullid.SetNull();
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(objectid);
        if(!entity)
            return false;

        Foundation::ComponentPtr component = entity->GetComponent(EC_OpenSimPrim::NameStatic());
        if(component)
        {
            EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(component.get());
            fullid = prim.FullId;
        }
        
        scene->RemoveEntity(objectid);
        rexlogicmodule_->UnregisterFullId(fullid);        
        return false;
    }    

    bool Primitive::HandleOSNE_ObjectProperties(OpenSimProtocol::NetworkEventInboundData* data)
    {
        NetInMessage *msg = data->message;
        msg->ResetReading();
        
        RexUUID full_id = msg->ReadUUID();
        msg->SkipToFirstVariableByName("Name");
        std::string name = msg->ReadString();
        std::string desc = msg->ReadString();
        ///\todo Handle rest of the vars.
        
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(full_id);
        if(entity)
        {
            EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());
            prim.ObjectName = name;
            prim.Description = desc;
            
            // Send the 'Entity Selected' event.
            Core::event_category_id_t event_category_id = rexlogicmodule_->GetFramework()->GetEventManager()->QueryEventCategory("Scene");
            Scene::Events::SceneEventData event_data(prim.LocalId);
            rexlogicmodule_->GetFramework()->GetEventManager()->SendEvent(event_category_id, Scene::Events::EVENT_ENTITY_SELECTED, &event_data);
        }
        else
            RexLogicModule::LogInfo("Received 'ObjectProperties' packet for unknown entity (" + full_id.ToString() + ").");
        
        return false;
    }
/*
    void Primitive::HandleOgreMaterialsChanged(Core::entity_id_t entityid)
    {
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
        assert(entity.get());
        if (!entity) 
            return;

        boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();

        EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());

        for(MaterialMap::iterator iter = prim.Materials.begin(); iter != prim.Materials.end(); ++iter)
        {
            RexLogic::MaterialData &material = iter->second;

            Core::request_tag_t tag = renderer->RequestResource(material.UUID.ToString(), OgreRenderer::OgreMaterialResource::GetTypeStatic());

            // Remember that we are going to get a resource event for this entity
            if (tag)
            {
                prim_resource_request_tags_[std::make_pair(tag, RexTypes::RexAT_MaterialScript)] = entityid;
                ///\todo remove, clb debug.
                RexLogicModule::LogInfo("Requested Ogre material.");
            }
        }
    }
*/
    ///\todo We now pass the entityid in the function. It is assumed that the entity contains exactly one EC_OpenSimPrim and one EC_OgreMesh component.
    /// Possibly in the future an entity can have several meshes attached to it, so we should pass in the EC_OgreMesh in question.
    void Primitive::HandleDrawType(Core::entity_id_t entityid)
    {
        ///\todo Make this only discard mesh resource request tags.
        // Discard old request tags for this entity
        DiscardRequestTags(entityid, prim_resource_request_tags_);
                                
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
        if (!entity)
            return;
        EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());
        if ((prim.DrawType == RexTypes::DRAWTYPE_MESH) && (!prim.MeshUUID.IsNull()))
        {
            // Remove custom object component if exists
            Foundation::ComponentPtr customptr = entity->GetComponent(OgreRenderer::EC_OgreCustomObject::NameStatic());
            if (customptr)
            {
                entity->RemoveEntityComponent(customptr);
            }
            
            // Get/create mesh component 
            Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
            if (!meshptr)
                entity->AddEntityComponent(meshptr = rexlogicmodule_->GetFramework()->GetComponentManager()->CreateComponent(OgreRenderer::EC_OgreMesh::NameStatic()));
            if (!meshptr)
                return;
            OgreRenderer::EC_OgreMesh& mesh = *(dynamic_cast<OgreRenderer::EC_OgreMesh*>(meshptr.get()));
            
            // Attach to placeable if not yet attached
            if (!mesh.GetPlaceable())
                mesh.SetPlaceable(entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()));
                             
            // Change mesh if yet nonexistent/changed
            // assume name to be UUID of mesh asset, which should be true of OgreRenderer resources
            std::string mesh_name = prim.MeshUUID.ToString();
            if (mesh.GetMeshName() != mesh_name)
            {
                boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
                    GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
                Core::request_tag_t tag = renderer->RequestResource(mesh_name, OgreRenderer::OgreMeshResource::GetTypeStatic());

                // Remember that we are going to get a resource event for this entity
                if (tag)
                    prim_resource_request_tags_[std::make_pair(tag, RexTypes::RexAT_Mesh)] = entityid;
            }
            
            // Check/request mesh textures
            HandleMeshMaterials(entityid);
        }
        else if (prim.DrawType == RexTypes::DRAWTYPE_PRIM)
        {
            // Remove mesh component if exists
            Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
            if (meshptr)
            {
                entity->RemoveEntityComponent(meshptr);
            }
            
            // Get/create custom (manual) object component 
            Foundation::ComponentPtr customptr = entity->GetComponent(OgreRenderer::EC_OgreCustomObject::NameStatic());
            if (!customptr)
                entity->AddEntityComponent(customptr = rexlogicmodule_->GetFramework()->GetComponentManager()->CreateComponent(OgreRenderer::EC_OgreCustomObject::NameStatic()));
            if (!customptr)
                return;
            OgreRenderer::EC_OgreCustomObject& custom = *(dynamic_cast<OgreRenderer::EC_OgreCustomObject*>(customptr.get()));
            
            // Attach to placeable if not yet attached
            if (!custom.GetPlaceable())
                custom.SetPlaceable(entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()));
            
            // Request prim textures
            HandlePrimTextures(entityid);
            
            // Create/update geometry
            if (prim.HasPrimShapeData)
                CreatePrimGeometry(custom.GetObject(), prim);
        }
    } 
    
    void Primitive::HandlePrimTextures(Core::entity_id_t entityid)
    {
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
        if (!entity) 
            return;
        EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());
        
        boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();

        std::set<RexTypes::RexUUID> tex_requests;
        
        if (!prim.PrimDefaultTexture.IsNull())
            tex_requests.insert(prim.PrimDefaultTexture);
            
        TextureMap::const_iterator i = prim.PrimTextures.begin();
        while (i != prim.PrimTextures.end())
        {
            if (!i->second.IsNull())
                tex_requests.insert(i->second);
            ++i;
        }
        
        std::set<RexTypes::RexUUID>::const_iterator j = tex_requests.begin();
        while (j != tex_requests.end())
        {
            std::string texname = (*j).ToString();
            Core::request_tag_t tag = renderer->RequestResource(texname, OgreRenderer::OgreTextureResource::GetTypeStatic());
             
            // Remember that we are going to get a resource event for this entity
            if (tag)
                prim_resource_request_tags_[std::make_pair(tag, RexTypes::RexAT_Texture)] = entityid;
                
            ++j;
        }
    }
    
    void Primitive::HandleMeshMaterials(Core::entity_id_t entityid)
    {
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
        if (!entity) 
            return;
        EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());            
           
        Foundation::ComponentPtr mesh = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        if (!mesh)
            return;
            
        OgreRenderer::EC_OgreMesh* meshptr = checked_static_cast<OgreRenderer::EC_OgreMesh*>(mesh.get());
     
        boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
            
        // Loop through all the materials in the mesh 
        MaterialMap::const_iterator i = prim.Materials.begin();
        while (i != prim.Materials.end())
        {
            if (i->second.UUID.IsNull())
            {
                ++i;
                continue;
            }

            const std::string mat_name = i->second.UUID.ToString();
            Core::uint idx = i->first;   

            //! \todo in the future material names will not correspond directly to texture names, so can't use this kind of check
            // If the mesh material is up-to-date, no need to process any further.
            if (meshptr->GetMaterialName(idx) == mat_name)
            {
                ++i;
                continue;
            }

            switch(i->second.Type)
            {
                case RexTypes::RexAT_Texture:
                {
                    Foundation::ResourcePtr res = renderer->GetResource(mat_name, OgreRenderer::OgreTextureResource::GetTypeStatic());
                    if (res)
                        HandleTextureReady(entityid, res);
                    else
                    {                
                        Core::request_tag_t tag = renderer->RequestResource(mat_name, OgreRenderer::OgreTextureResource::GetTypeStatic());

                        // Remember that we are going to get a resource event for this entity
                        if (tag)
                            prim_resource_request_tags_[std::make_pair(tag, RexTypes::RexAT_Texture)] = entityid;   
                    } 
                }
                break;
                case RexTypes::RexAT_MaterialScript:
                {
                    Foundation::ResourcePtr res = renderer->GetResource(mat_name, OgreRenderer::OgreMaterialResource::GetTypeStatic());
                    if (res)
                        HandleMaterialResourceReady(entityid, res);
                    else
                    {                
                        Core::request_tag_t tag = renderer->RequestResource(mat_name, OgreRenderer::OgreMaterialResource::GetTypeStatic());

                        // Remember that we are going to get a resource event for this entity
                        if (tag)
                            prim_resource_request_tags_[std::make_pair(tag, RexTypes::RexAT_MaterialScript)] = entityid;   
                    } 
                }
                break;
            }

            ++i;
        }    
    }
    
    void Primitive::HandleExtraParams(const Core::entity_id_t &entity_id, const uint8_t *extra_params_data)
    {
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entity_id);
        if (!entity)
            return;
        
        int idx = 0;
        uint8_t num_params = ReadUInt8FromBytes(extra_params_data, idx);
        for (uint8_t param_i = 0; param_i < num_params; ++param_i)
        {
            uint16_t param_type = ReadUInt16FromBytes(extra_params_data, idx);
            Core::uint param_size = ReadSInt32FromBytes(extra_params_data, idx);
            switch (param_type)
            {
            case 32: // light
            {
                // If light component doesn't exist, create it.
                if(!entity->GetComponent(OgreRenderer::EC_OgreLight::NameStatic()).get())
                    entity->AddEntityComponent(rexlogicmodule_->GetFramework()->GetComponentManager()->CreateComponent("EC_OgreLight"));
                    
                // Read the data.
                Core::Color color = ReadColorFromBytes(extra_params_data, idx);
                float radius = ReadFloatFromBytes(extra_params_data, idx);
                float cutoff = ReadFloatFromBytes(extra_params_data, idx); //this seems not be used anywhere.
                float falloff = ReadFloatFromBytes(extra_params_data, idx);
                
                AttachLightComponent(entity, color, radius, falloff);
                break;
            }
            ///\todo Are we interested in other types of extra params? Probably not.
            case 16: // flexible
            case 48: // sculpt
            default:
                break;
            }
        }
    }
    
    void Primitive::AttachLightComponent(Scene::EntityPtr entity, Core::Color color, float radius, float falloff)
    {
        if (radius < 0.001) radius = 0.001;

        // Attenuation calculation
        float x = 3.f * (1.f + falloff);
        float linear = x / radius; // % of brightness at radius
        // Add a constant quad term for diminishing the light more beyond radius
        float quad = 0.3f / (radius * radius); 

        // Use the point where linear attenuation has reduced intensity to 1/20 as max range
        // (OpenGL has no absolute light range cap like Direct3D)
        float max_radius = radius;
        if (linear > 0.0)
        {
            max_radius = 20 / linear;
            if (max_radius < radius)
                max_radius = radius;
        }

        if (!entity->GetComponent(OgreRenderer::EC_OgreLight::NameStatic()))
            return;

        OgreRenderer::EC_OgreLight& light = *checked_static_cast<OgreRenderer::EC_OgreLight*>
            (entity->GetComponent(OgreRenderer::EC_OgreLight::NameStatic()).get());
            
        ///\note Only point lights are supported at the moment.
        light.SetType(OgreRenderer::EC_OgreLight::LT_Point);
        Foundation::ComponentPtr placeable = entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());  
        if (placeable)
            light.SetPlaceable(placeable);
        
        ///\note Test if the color values have to be in range [0, 1].
        light.SetColor(color);
        light.SetAttenuation(max_radius, 0.0f, linear, quad);
    }

    bool Primitive::HandleResourceEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == Resource::Events::RESOURCE_READY)
        {
            Resource::Events::ResourceReady* event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);
            Foundation::ResourcePtr res = event_data->resource_;
            RexTypes::asset_type_t asset_type = -1;

            ///\todo Perhaps the Resource should be able to tell us what from what type of Asset it was produced from, instead of having to do it here.
            // First we identify what kind of resource is ready:
            if (res->GetType() == OgreRenderer::OgreMeshResource::GetTypeStatic())
                asset_type = RexTypes::RexAT_Mesh;
            else if (res->GetType() == OgreRenderer::OgreTextureResource::GetTypeStatic())
                asset_type = RexTypes::RexAT_Texture;
            else if (res->GetType() == OgreRenderer::OgreMaterialResource::GetTypeStatic())
                asset_type = RexTypes::RexAT_MaterialScript;

            EntityResourceRequestMap::iterator i = prim_resource_request_tags_.find(std::make_pair(event_data->tag_, asset_type));
            if (i == prim_resource_request_tags_.end())
                return false; // Not our resource request, go somewhere else to process it.

            switch(asset_type)
            {
            case RexAT_Texture:
                HandleTextureReady(i->second, res);
                break;
            case RexAT_Mesh:
                HandleMeshReady(i->second, res);
                break;
            case RexAT_MaterialScript:
                HandleMaterialResourceReady(i->second, res);
                break;
            default:
                assert(false && "Invalid asset_type added to prim_resource_request_tags_! Don't know how it ended up there and don't know how to handle!");
                break;
            }

            prim_resource_request_tags_.erase(i);
            return true;
        }
        
        return false;
    }
    
    void Primitive::HandleMeshReady(Core::entity_id_t entityid, Foundation::ResourcePtr res)
    {     
        if (!res) return;
        if (res->GetType() != OgreRenderer::OgreMeshResource::GetTypeStatic()) return;
        
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
        if (!entity) return;
        EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());

        Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        if (!meshptr) return;
        OgreRenderer::EC_OgreMesh& mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(meshptr.get());
        
        mesh.SetMesh(res->GetId());

        // Set adjustment orientation for mesh (a legacy haxor, Ogre meshes usually have Y-axis as vertical)
        Core::Quaternion adjust(Core::PI/2, 0, Core::PI);
        mesh.SetAdjustOrientation(adjust);

        HandlePrimScale(entityid);
        
        // Check/set textures now that we have the mesh
        HandleMeshMaterials(entityid); 
    }

    void Primitive::HandleTextureReady(Core::entity_id_t entityid, Foundation::ResourcePtr res)
    {
        assert(res.get());
        if (!res) 
            return;
        assert(res->GetType() == OgreRenderer::OgreTextureResource::GetTypeStatic());
        if (res->GetType() != OgreRenderer::OgreTextureResource::GetTypeStatic()) 
            return;
               
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
        if (!entity) return;
        EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());            
        if (prim.DrawType == RexTypes::DRAWTYPE_MESH)
        {
            Foundation::ComponentPtr mesh = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
            if (!mesh) return;
            OgreRenderer::EC_OgreMesh* meshptr = checked_static_cast<OgreRenderer::EC_OgreMesh*>(mesh.get());      
            // If don't have the actual mesh entity yet, no use trying to set texture
            if (!meshptr->GetEntity()) return;
                            
            MaterialMap::const_iterator i = prim.Materials.begin();
            while (i != prim.Materials.end())
            {
                Core::uint idx = i->first;
                // For now, handle only textures, not materials
                if ((i->second.Type == RexTypes::RexAT_Texture) && (i->second.UUID.ToString() == res->GetId()))
                {
                    // debug material creation to see diffuse textures
                    Ogre::MaterialPtr mat = OgreRenderer::GetOrCreateUnlitTexturedMaterial(res->GetId().c_str());
                    OgreRenderer::SetTextureUnitOnMaterial(mat, res->GetId().c_str());
                   
                    meshptr->SetMaterial(idx, res->GetId());
                }
                ++i;
            }
        }
        else
        {
            // Handle prim texture
            Foundation::ComponentPtr custom = entity->GetComponent(OgreRenderer::EC_OgreCustomObject::NameStatic());
            if (!custom) return;
            OgreRenderer::EC_OgreCustomObject* customptr = checked_static_cast<OgreRenderer::EC_OgreCustomObject*>(custom.get());
            
            Ogre::ManualObject* manual = customptr->GetObject();
            if (!manual) return;
            
            for (Core::uint i = 0; i < manual->getNumSections(); ++i)
            {
                // If this section of the custom geometry is using the received texture, update the material
                // Note: material has already been created beforehand, which is kind of hackish
                if (manual->getSection(i)->getMaterialName() == res->GetId())
                {
                    Ogre::MaterialPtr mat = OgreRenderer::GetOrCreateUnlitTexturedMaterial(res->GetId().c_str());
                    OgreRenderer::SetTextureUnitOnMaterial(mat, res->GetId().c_str());
                    break;
                }
            }
        }
    }

    void Primitive::HandleMaterialResourceReady(Core::entity_id_t entityid, Foundation::ResourcePtr res)
    {
        //! \todo crash, probably when mesh arrives, but material for it is not yet ready / failed to parse. Also other crashes but got too frustrated to check. See also todo item in ResourceHandler::UpdateMaterial(). -cm
        return;

        assert(res.get());
        if (!res) 
            return;
        assert(res->GetType() == OgreRenderer::OgreMaterialResource::GetTypeStatic());
        if (res->GetType() != OgreRenderer::OgreMaterialResource::GetTypeStatic()) 
            return;
               
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
        if (!entity) return;
        EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());            
           
        Foundation::ComponentPtr mesh = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        assert(mesh.get());
        if (!mesh) 
            return;
        OgreRenderer::EC_OgreMesh* meshptr = checked_static_cast<OgreRenderer::EC_OgreMesh*>(mesh.get());      
        // If don't have the actual mesh entity yet, no use trying to set the material
        if (!meshptr->GetEntity()) return;     
                        
        MaterialMap::const_iterator i = prim.Materials.begin();
        while (i != prim.Materials.end())
        {
            Core::uint idx = i->first;
            // For now, handle only textures, not materials
            if ((i->second.Type == RexTypes::RexAT_MaterialScript) && (i->second.UUID.ToString() == res->GetId()))
            {
                OgreRenderer::OgreMaterialResource *materialRes = dynamic_cast<OgreRenderer::OgreMaterialResource*>(res.get());
                assert(materialRes);

                Ogre::MaterialPtr mat = materialRes->GetMaterial();
                assert(mat.get());
                if (!mat.get())
                {
                    std::stringstream ss;
                    ss << std::string("Resource \"") << res->GetId() << "\" did not contain a proper Ogre::MaterialPtr!";
                    RexLogicModule::LogInfo(ss.str());
                }
                meshptr->SetMaterial(idx, mat->getName());
                std::stringstream ss;
                ss << std::string("Set submesh ") << idx << " to use material \"" << mat->getName() << "\"";
                RexLogicModule::LogInfo(ss.str());
            }
            ++i;
        }
    }

    void Primitive::DiscardRequestTags(Core::entity_id_t entityid, Primitive::EntityResourceRequestMap& map)
    {
        std::vector<Primitive::EntityResourceRequestMap::iterator> tags_to_remove;

        EntityResourceRequestMap::iterator i = map.begin();
        while (i != map.end())
        {
            if (i->second == entityid)
                tags_to_remove.push_back(i);
            ++i;
        }
        for (int j = 0; j < tags_to_remove.size(); ++j)
            map.erase(tags_to_remove[j]);
    }
    
    void Primitive::HandlePrimScale(Core::entity_id_t entityid)
    {
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
        if (!entity) return;
        EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());            
        Foundation::ComponentPtr placeable = entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());  
        if (!placeable) return;
        OgreRenderer::EC_OgrePlaceable &ogrepos = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(placeable.get());
        
        // Handle scale mesh to prim-setting
        Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        if (meshptr)
        {
            OgreRenderer::EC_OgreMesh& mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(meshptr.get());
            Core::Vector3df adjust_scale(1.0, 1.0, 1.0);
            if (prim.ScaleToPrim && mesh.GetEntity())
            {
                Core::Vector3df min, max, size;

                mesh.GetBoundingBox(min, max);
                size = max - min;
                if ((size.x != 0.0) && (size.y != 0.0) && (size.z != 0.0))
                {
                    adjust_scale.x /= size.x;
                    adjust_scale.y /= size.y;
                    adjust_scale.z /= size.z;
                }
            }

            // Because Ogre doesn't care about rotation when combining scaling, have to do a nasty hack here
            // (meshes have X & Y axes swapped)
            const Core::Vector3df& prim_scale = prim.Scale;
            if ((prim_scale.x != 0.0) && (prim_scale.y != 0.0) && (prim_scale.z != 0.0))
            {
                adjust_scale.y /= prim_scale.y;
                adjust_scale.z /= prim_scale.z;

                adjust_scale.y *= prim_scale.z;
                adjust_scale.z *= prim_scale.y;
            }

            mesh.SetAdjustScale(adjust_scale);
        }

        ogrepos.SetScale(prim.Scale);
    }
    
    void Primitive::ParseTextureEntryData(EC_OpenSimPrim& prim, const uint8_t* bytes, size_t length)
    {
        prim.PrimTextures.clear();
        prim.PrimColors.clear();
        
        int idx = 0;
        uint32_t bits;
        int num_bits;
        
        if (idx >= length)
            return;
        
        RexTypes::RexUUID default_texture_id = ReadUUIDFromBytes(bytes, idx);
        prim.PrimDefaultTexture = default_texture_id;
        
        while ((idx < length) && (ReadTextureEntryBits(bits, num_bits, bytes, idx)))
        {
            if (idx >= length)
                return;
            RexTypes::RexUUID texture_id = ReadUUIDFromBytes(bytes, idx);
            for (int i = 0; i < num_bits; ++i)
            {
                if (bits & 1)
                {
                    prim.PrimTextures[i] = texture_id;
                }
                bits >>= 1;
            }
        }
        
        if (idx >= length)
            return;
        
        Core::Color default_color = ReadColorFromBytes(bytes, idx);
        prim.PrimDefaultColor = default_color;
        
        while ((idx < length) && (ReadTextureEntryBits(bits, num_bits, bytes, idx)))
        {
            if (idx >= length)
                return;
            Core::Color color = ReadColorFromBytes(bytes, idx);
            for (int i = 0; i < num_bits; ++i)
            {
                if (bits & 1)
                {
                    prim.PrimColors[i] = color;
                }
                bits >>= 1;
            }
        }
    }
}