// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Primitive.h"
#include "RexNetworkUtils.h"
#include "RexLogicModule.h"
#include "EC_OpenSimPrim.h"
#include "EC_NetworkPosition.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMesh.h"
#include "EC_OgreCustomObject.h"
#include "EC_OgreLight.h"
#include "EC_OgreParticleSystem.h"
#include "OgreTextureResource.h"
#include "OgreMaterialResource.h"
#include "OgreMeshResource.h"
#include "OgreParticleResource.h"
#include "OgreMaterialUtils.h"
#include "Renderer.h"
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

    //DebugCreateOgreBoundingBox(rexlogicmodule_, entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()),"AmbientRed");
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
        msg->SkipToNextVariable();        // State U8 ///\todo Unhandled inbound variable 'State'.
        RexUUID fullid = msg->ReadUUID();
        msg->SkipToNextVariable();        // CRC U32 ///\todo Unhandled inbound variable 'CRC'.
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
            // The data contents:
            // ofs  0 - pos xyz - 3 x float (3x4 bytes)
            // ofs 12 - vel xyz - 3 x float (3x4 bytes)
            // ofs 24 - acc xyz - 3 x float (3x4 bytes)
            // ofs 36 - orientation, quat with last (w) component omitted - 3 x float (3x4 bytes)
            // ofs 48 - angular velocity - 3 x float (3x4 bytes)
            // total 60 bytes

            Core::Vector3Df vec = Core::OpenSimToOgreCoordinateAxes(*reinterpret_cast<const Core::Vector3df*>(&objectdatabytes[0]));
            if (IsValidPositionVector(vec))
                netpos.position_ = vec;

            vec = Core::OpenSimToOgreCoordinateAxes(*reinterpret_cast<const Core::Vector3df*>(&objectdatabytes[12])); 
            if (IsValidVelocityVector(vec))
                netpos.velocity_ = vec;

            vec = Core::OpenSimToOgreCoordinateAxes(*reinterpret_cast<const Core::Vector3df*>(&objectdatabytes[24]));
            if (IsValidVelocityVector(vec)) // Use Velocity validation for Acceleration as well - it's ok as they are quite similar.
                netpos.accel_ = vec;

            netpos.rotation_ = Core::OpenSimToOgreQuaternion(Core::UnpackQuaternionFromFloat3((float*)&objectdatabytes[36])); 
            vec = Core::OpenSimToOgreCoordinateAxes(*reinterpret_cast<const Core::Vector3df*>(&objectdatabytes[48]));
            if (IsValidVelocityVector(vec)) // Use Velocity validation for Angular Velocity as well - it's ok as they are quite similar.
                netpos.rotvel_ = vec;
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

    Core::Vector3Df vec = GetProcessedVector(&bytes[i]);
    if (IsValidPositionVector(vec))
        netpos.position_ = vec;
    i += sizeof(Core::Vector3df);

    netpos.velocity_ = GetProcessedScaledVectorFromUint16(&bytes[i],128);
    i += 6;

    netpos.accel_ = GetProcessedVectorFromUint16(&bytes[i]); 
    i += 6;

    netpos.rotation_ = GetProcessedQuaternion(&bytes[i]);
    i += 8;

    netpos.rotvel_ = GetProcessedScaledVectorFromUint16(&bytes[i],128);
    i += 16;

    netpos.Updated();
    assert(i <= 60);
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
        HandleRexPrimDataBlob(entity->GetId(), &fulldata[0], fulldata.size());
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
        HandleRexPrimDataBlob(entityid, &i->second[0], i->second.size());
        pending_rexprimdata_.erase(i);
    }
}

void Primitive::SendRexPrimData(Core::entity_id_t entityid)
{
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity)
        return;

    EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());
    
    RexUUID fullid = prim.FullId;
    
    std::vector<uint8_t> buffer;
    buffer.resize(4096);
    int idx = 0;
    bool send_asset_urls = false;
    
    // graphical values
    WriteUInt8ToBytes(prim.DrawType, &buffer[0], idx);
    WriteBoolToBytes(prim.IsVisible, &buffer[0], idx);
    WriteBoolToBytes(prim.CastShadows, &buffer[0], idx);
    WriteBoolToBytes(prim.LightCreatesShadows, &buffer[0], idx);
    WriteBoolToBytes(prim.DescriptionTexture, &buffer[0], idx);
    WriteBoolToBytes(prim.ScaleToPrim, &buffer[0], idx);
    WriteFloatToBytes(prim.DrawDistance, &buffer[0], idx);
    WriteFloatToBytes(prim.LOD, &buffer[0], idx);   
    
    // UUIDs
    // Note: if the EC contains asset urls that can not be encoded as UUIDs, we still have to send
    // invalid (null) UUIDs to retain binary compatibility with the rexprimdatablob
    
    if (IsUrlBased(prim.MeshID) || IsUrlBased(prim.CollisionMeshID) || IsUrlBased(prim.ParticleScriptID) || IsUrlBased(prim.AnimationPackageID))
        send_asset_urls = true;
        
    WriteUUIDToBytes(RexUUID(prim.MeshID), &buffer[0], idx);
    WriteUUIDToBytes(RexUUID(prim.CollisionMeshID), &buffer[0], idx);
    WriteUUIDToBytes(RexUUID(prim.ParticleScriptID), &buffer[0], idx);

    // Animation
    WriteUUIDToBytes(RexUUID(prim.AnimationPackageID), &buffer[0], idx);
    WriteNullTerminatedStringToBytes(prim.AnimationName, &buffer[0], idx);
    WriteFloatToBytes(prim.AnimationRate, &buffer[0], idx);

    // Materials
    size_t mat_count = prim.Materials.size();
    MaterialMap::const_iterator i = prim.Materials.begin();
    WriteUInt8ToBytes((uint8_t)mat_count, &buffer[0], idx); 
    while (i != prim.Materials.end())
    {
        // Write assettype, uuid, index for each
        if (IsUrlBased(i->second.asset_id))
            send_asset_urls = true;
        WriteUInt8ToBytes(i->second.Type, &buffer[0], idx);
        WriteUUIDToBytes(RexUUID(i->second.asset_id), &buffer[0], idx);
        WriteUInt8ToBytes(i->first, &buffer[0], idx);
        ++i;
    }

    WriteNullTerminatedStringToBytes(prim.ServerScriptClass, &buffer[0], idx);

    // Sound
    if (IsUrlBased(prim.SoundID))
        send_asset_urls = true;
    WriteUUIDToBytes(RexUUID(prim.SoundID), &buffer[0], idx);
    WriteFloatToBytes(prim.SoundVolume, &buffer[0], idx);
    WriteFloatToBytes(prim.SoundRadius, &buffer[0], idx);
    
    WriteUInt32ToBytes(prim.SelectPriority, &buffer[0], idx);

    // Extension: url based asset id's
    if (send_asset_urls)
    {
        WriteNullTerminatedStringToBytes(prim.MeshID, &buffer[0], idx);
        WriteNullTerminatedStringToBytes(prim.CollisionMeshID, &buffer[0], idx);
        WriteNullTerminatedStringToBytes(prim.ParticleScriptID, &buffer[0], idx);
        WriteNullTerminatedStringToBytes(prim.AnimationPackageID, &buffer[0], idx);
        WriteNullTerminatedStringToBytes(prim.SoundID, &buffer[0], idx);
        i = prim.Materials.begin();
        while (i != prim.Materials.end())        
        {
            WriteNullTerminatedStringToBytes(i->second.asset_id, &buffer[0], idx);
            ++i;
        }
    }  
          
    buffer.resize(idx);

    RexServerConnectionPtr conn = rexlogicmodule_->GetServerConnection();
    if (!conn)
        return;
    Core::StringVector strings;
    strings.push_back(fullid.ToString());
    conn->SendGenericMessageBinary("RexPrimData", strings, buffer);
    
}
    
void Primitive::HandleRexPrimDataBlob(Core::entity_id_t entityid, const uint8_t* primdata, const int primdata_size)
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

    prim.MeshID = ReadUUIDFromBytes(primdata,idx).ToString();
    prim.CollisionMeshID = ReadUUIDFromBytes(primdata,idx).ToString();    
    prim.ParticleScriptID = ReadUUIDFromBytes(primdata,idx).ToString();

    // animation
    prim.AnimationPackageID = ReadUUIDFromBytes(primdata,idx).ToString();
    prim.AnimationName = ReadNullTerminatedStringFromBytes(primdata,idx);
    prim.AnimationRate = ReadFloatFromBytes(primdata,idx);

    MaterialMap materials;
    uint8_t tempmaterialindex = 0; 
    uint8_t tempmaterialcount = ReadUInt8FromBytes(primdata,idx);
    std::vector<uint8_t> material_indexes;
    for(int i=0;i<tempmaterialcount;i++)
    {
        MaterialData newmaterialdata;

        newmaterialdata.Type = ReadUInt8FromBytes(primdata,idx);
        newmaterialdata.asset_id = ReadUUIDFromBytes(primdata,idx).ToString();
        tempmaterialindex = ReadUInt8FromBytes(primdata,idx);
        material_indexes.push_back(tempmaterialindex); 
        materials[tempmaterialindex] = newmaterialdata;
    }
    prim.Materials = materials;

    prim.ServerScriptClass = ReadNullTerminatedStringFromBytes(primdata,idx);

    // sound
    prim.SoundID = ReadUUIDFromBytes(primdata,idx).ToString();
    prim.SoundVolume = ReadFloatFromBytes(primdata,idx);
    prim.SoundRadius = ReadFloatFromBytes(primdata,idx);

    prim.SelectPriority = ReadUInt32FromBytes(primdata,idx);

    // Copy selectpriority to the placeable for the renderer raycast
    Foundation::ComponentPtr placeableptr = entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
    if (placeableptr)
    {
        OgreRenderer::EC_OgrePlaceable &placeable = 
            *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(placeableptr.get());
        placeable.SetSelectPriority(prim.SelectPriority);
    }

    // Asset urls
    // Check for reasonably longer data. There might be an extra endzero
    if (idx+5 < primdata_size)
    {
        std::string rex_mesh_url = ReadNullTerminatedStringFromBytes(primdata,idx);
        std::string rex_collision_mesh_url = ReadNullTerminatedStringFromBytes(primdata,idx);
        std::string rex_particle_script_url = ReadNullTerminatedStringFromBytes(primdata,idx);
        std::string rex_animation_package_url = ReadNullTerminatedStringFromBytes(primdata,idx);
        std::string rex_sound_url = ReadNullTerminatedStringFromBytes(primdata,idx);

        if (rex_mesh_url.size() > 0)
            prim.CollisionMeshID = rex_mesh_url;
        if (rex_collision_mesh_url.size() > 0)
            prim.CollisionMeshID = rex_collision_mesh_url;
        if (rex_particle_script_url.size() > 0)
            prim.ParticleScriptID = rex_particle_script_url;
        if (rex_animation_package_url.size() > 0)
            prim.AnimationPackageID = rex_animation_package_url;
        if (rex_sound_url.size() > 0)
            prim.SoundID = rex_sound_url;

        for (int i=0; i<tempmaterialcount; ++i)
        {
            std::string rex_material_url = ReadNullTerminatedStringFromBytes(primdata,idx);
            if (rex_material_url.size() > 0)
            {
                uint8_t material_index = material_indexes[i];
                materials[material_index].asset_id = rex_material_url;
            }
        }
        prim.Materials = materials;
    }
    
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
        
        ///\todo Odd behavior? The ENTITY_SELECTED event is passed only after the server responds with an ObjectProperties
        /// message. Should we maintain our own notion of what's selected and rename this event to PRIM_OBJECT_PROPERTIES or
        /// something similar? Or is it desired that the ObjectProperties wire message defines exactly what objects the
        /// client has selected?

        // Send the 'Entity Selected' event.
        Core::event_category_id_t event_category_id = rexlogicmodule_->GetFramework()->GetEventManager()->QueryEventCategory("Scene");
        Scene::Events::SceneEventData event_data(prim.LocalId);
        rexlogicmodule_->GetFramework()->GetEventManager()->SendEvent(event_category_id, Scene::Events::EVENT_ENTITY_SELECTED, &event_data);
    }
    else
        RexLogicModule::LogInfo("Received 'ObjectProperties' packet for unknown entity (" + full_id.ToString() + ").");
    
    return false;
}

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
    if ((prim.DrawType == RexTypes::DRAWTYPE_MESH) && (!RexTypes::IsNull(prim.MeshID)))
    {
        // Remove custom object component if exists
        Foundation::ComponentPtr customptr = entity->GetComponent(OgreRenderer::EC_OgreCustomObject::NameStatic());
        if (customptr)
            entity->RemoveEntityComponent(customptr);

        // Get/create mesh component 
        Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        if (!meshptr)
            entity->AddEntityComponent(meshptr = rexlogicmodule_->GetFramework()->GetComponentManager()->CreateComponent(OgreRenderer::EC_OgreMesh::NameStatic()));
        if (!meshptr)
            return;
        OgreRenderer::EC_OgreMesh& mesh = *(dynamic_cast<OgreRenderer::EC_OgreMesh*>(meshptr.get()));
        
        // Attach to placeable if not yet attached
        if (!mesh.GetPlaceable())
            mesh.SetPlaceable(entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()), entity.get());
        
        // Change mesh if yet nonexistent/changed
        // assume name to be UUID of mesh asset, which should be true of OgreRenderer resources
        const std::string& mesh_name = prim.MeshID;
        if (mesh.GetMeshName() != mesh_name)
        {
            boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
                GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
            Core::request_tag_t tag = renderer->RequestResource(mesh_name, OgreRenderer::OgreMeshResource::GetTypeStatic());

            // Remember that we are going to get a resource event for this entity
            if (tag)
                prim_resource_request_tags_[std::make_pair(tag, RexTypes::RexAT_Mesh)] = entityid;
        }
        
        // Set rendering distance & shadows
        mesh.SetDrawDistance(prim.DrawDistance);
        mesh.SetCastShadows(prim.CastShadows);
        
        // Check/request mesh textures
        HandleMeshMaterials(entityid);
    }
    else if (prim.DrawType == RexTypes::DRAWTYPE_PRIM)
    {
        // Remove mesh component if exists
        Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        if (meshptr)
            entity->RemoveEntityComponent(meshptr);

        // Get/create custom (manual) object component 
        Foundation::ComponentPtr customptr = entity->GetComponent(OgreRenderer::EC_OgreCustomObject::NameStatic());
        if (!customptr)
            entity->AddEntityComponent(customptr = rexlogicmodule_->GetFramework()->GetComponentManager()->CreateComponent(OgreRenderer::EC_OgreCustomObject::NameStatic()));
        if (!customptr)
            return;
        OgreRenderer::EC_OgreCustomObject& custom = *(dynamic_cast<OgreRenderer::EC_OgreCustomObject*>(customptr.get()));

        // Attach to placeable if not yet attached
        if (!custom.GetPlaceable())
            custom.SetPlaceable(entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()), entity.get());

        // Set rendering distance/cast shadows setting
        custom.SetDrawDistance(prim.DrawDistance);
        custom.SetCastShadows(prim.CastShadows);

        // Request prim textures
        HandlePrimTexturesAndMaterial(entityid);

        // Create/update geometry
        if (prim.HasPrimShapeData)
        {
            CreatePrimGeometry(rexlogicmodule_->GetFramework(), custom.GetObject(), prim);
            custom.CommitChanges();
            
            Scene::Events::EntityEventData event_data;
            event_data.entity = entity;
            Foundation::EventManagerPtr event_manager = rexlogicmodule_->GetFramework()->GetEventManager();
            event_manager->SendEvent(event_manager->QueryEventCategory("Scene"), Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED, &event_data);
        }
    }

    if (!RexTypes::IsNull(prim.ParticleScriptID))
    {
        // Create particle system component & attach, if does not yet exist
        Foundation::ComponentPtr particleptr = entity->GetComponent(OgreRenderer::EC_OgreParticleSystem::NameStatic());
        if (!particleptr)
            entity->AddEntityComponent(particleptr = rexlogicmodule_->GetFramework()->GetComponentManager()->CreateComponent(OgreRenderer::EC_OgreParticleSystem::NameStatic()));
        if (!particleptr)
            return;
        OgreRenderer::EC_OgreParticleSystem& particle = *(dynamic_cast<OgreRenderer::EC_OgreParticleSystem*>(particleptr.get()));
        
        // Attach to placeable if not yet attached
        if (!particle.GetPlaceable())
            particle.SetPlaceable(entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()));
            
        // Change particle system if yet nonexistent/changed
        const std::string& script_name = prim.ParticleScriptID;
        
        if (particle.GetParticleSystemName(0).find(script_name) == std::string::npos)
        {
            boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
                GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
            Core::request_tag_t tag = renderer->RequestResource(script_name, OgreRenderer::OgreParticleResource::GetTypeStatic());

            // Remember that we are going to get a resource event for this entity
            if (tag)
                prim_resource_request_tags_[std::make_pair(tag, RexTypes::RexAT_ParticleScript)] = entityid;
        }
    }
    else
    {
        // If should be no particle system, remove it if exists
        Foundation::ComponentPtr particleptr = entity->GetComponent(OgreRenderer::EC_OgreParticleSystem::NameStatic());
        if (particleptr)
        {
            entity->RemoveEntityComponent(particleptr);
        }
    }
    
    // Handle visibility via the placeable
    {
        Foundation::ComponentPtr placeableptr = entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
        if (placeableptr)
        {
            OgreRenderer::EC_OgrePlaceable& placeable = *(dynamic_cast<OgreRenderer::EC_OgrePlaceable*>(placeableptr.get()));
            placeable.GetSceneNode()->setVisible(prim.IsVisible);
        }
    }
    
}

void Primitive::HandlePrimTexturesAndMaterial(Core::entity_id_t entityid)
{
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity) 
        return;
    EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());
    
    boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
        GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();

    // Check for prim material override
    if ((prim.Materials[0].Type == RexTypes::RexAT_MaterialScript) && (!RexTypes::IsNull(prim.Materials[0].asset_id)))
    {
        std::string matname = prim.Materials[0].asset_id;
        
        // Request material if don't have it yet
        if (!renderer->GetResource(matname, OgreRenderer::OgreMaterialResource::GetTypeStatic()))
        {
            Core::request_tag_t tag = renderer->RequestResource(matname, OgreRenderer::OgreMaterialResource::GetTypeStatic());
             
            // Remember that we are going to get a resource event for this entity
            if (tag)
                prim_resource_request_tags_[std::make_pair(tag, RexTypes::RexAT_MaterialScript)] = entityid;
        }
    }
    else
    {
        // Otherwise request normal textures
        std::set<RexTypes::RexAssetID> tex_requests;
        
        if (!RexTypes::IsNull(prim.PrimDefaultTextureID))
            tex_requests.insert(prim.PrimDefaultTextureID);
            
        TextureMap::const_iterator i = prim.PrimTextures.begin();
        while (i != prim.PrimTextures.end())
        {
            if (!RexTypes::IsNull(i->second))
                tex_requests.insert(i->second);
            ++i;
        }
        
        std::set<RexTypes::RexAssetID>::const_iterator j = tex_requests.begin();
        while (j != tex_requests.end())
        {
            std::string texname = (*j);
            Core::request_tag_t tag = renderer->RequestResource(texname, OgreRenderer::OgreTextureResource::GetTypeStatic());
             
            // Remember that we are going to get a resource event for this entity
            if (tag)
                prim_resource_request_tags_[std::make_pair(tag, RexTypes::RexAT_Texture)] = entityid;
            
            ++j;
        }
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
        if (RexTypes::IsNull(i->second.asset_id))
        {
            ++i;
            continue;
        }

        const std::string mat_name = i->second.asset_id;
        Core::uint idx = i->first;   

        //! \todo in the future material names will probably not correspond directly to texture names, so can't use this kind of check
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
    // (lighting model/shaders used have no absolute range cap)
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
        else if (res->GetType() == OgreRenderer::OgreParticleResource::GetTypeStatic())
            asset_type = RexTypes::RexAT_ParticleScript;

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
        case RexAT_ParticleScript:
            HandleParticleScriptReady(i->second, res);
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

    Scene::Events::EntityEventData event_data;
    event_data.entity = entity;
    Foundation::EventManagerPtr event_manager = rexlogicmodule_->GetFramework()->GetEventManager();
    event_manager->SendEvent(event_manager->QueryEventCategory("Scene"), Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED, &event_data);
}

void Primitive::HandleParticleScriptReady(Core::entity_id_t entityid, Foundation::ResourcePtr res)
{
    if (!res) return;
    if (res->GetType() != OgreRenderer::OgreParticleResource::GetTypeStatic()) return;
    OgreRenderer::OgreParticleResource* partres = checked_static_cast<OgreRenderer::OgreParticleResource*>(res.get());
     
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity) return;
    EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());

    Foundation::ComponentPtr particleptr = entity->GetComponent(OgreRenderer::EC_OgreParticleSystem::NameStatic());
    if (!particleptr) return;
    OgreRenderer::EC_OgreParticleSystem& particle = *checked_static_cast<OgreRenderer::EC_OgreParticleSystem*>(particleptr.get());

    particle.RemoveParticleSystems();
    for (Core::uint i = 0; i < partres->GetNumTemplates(); ++i)
    {
        particle.AddParticleSystem(partres->GetTemplateName(i));
    }

    // Set adjustment orientation for system (legacy haxor)
    //Core::Quaternion adjust(Core::PI, 0, Core::PI);
    //particle.SetAdjustOrientation(adjust);
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
            if ((i->second.Type == RexTypes::RexAT_Texture) && (i->second.asset_id.compare(res->GetId()) == 0))
            {
                // Use a legacy material with the same name as the texture, created automatically by renderer
                meshptr->SetMaterial(idx, res->GetId());
                
                Scene::Events::EntityEventData event_data;
                event_data.entity = entity;
                Foundation::EventManagerPtr event_manager = rexlogicmodule_->GetFramework()->GetEventManager();
                event_manager->SendEvent(event_manager->QueryEventCategory("Scene"), Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED, &event_data);            
            }
            ++i;
        }
    }
}

void Primitive::HandleMaterialResourceReady(Core::entity_id_t entityid, Foundation::ResourcePtr res)
{
    assert(res.get());
    if (!res) 
        return;
    assert(res->GetType() == OgreRenderer::OgreMaterialResource::GetTypeStatic());
    if (res->GetType() != OgreRenderer::OgreMaterialResource::GetTypeStatic()) 
        return;
           
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity) return;
    EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());            
       
    // Handle material ready for prim
    if (prim.DrawType == RexTypes::DRAWTYPE_PRIM)
    {
        Foundation::ComponentPtr customptr = entity->GetComponent(OgreRenderer::EC_OgreCustomObject::NameStatic());
        if (customptr && res->GetId() == prim.Materials[0].asset_id  && prim.Materials[0].Type == RexTypes::RexAT_MaterialScript)
        {
            OgreRenderer::EC_OgreCustomObject& custom = *checked_static_cast<OgreRenderer::EC_OgreCustomObject*>(customptr.get());
            // Update geometry now that the material exists
            if (prim.HasPrimShapeData)
            {
                CreatePrimGeometry(rexlogicmodule_->GetFramework(), custom.GetObject(), prim);
                custom.CommitChanges();

                Scene::Events::EntityEventData event_data;
                event_data.entity = entity;
                Foundation::EventManagerPtr event_manager = rexlogicmodule_->GetFramework()->GetEventManager();
                event_manager->SendEvent(event_manager->QueryEventCategory("Scene"), Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED, &event_data);
            }
        }
    }
    
    // Handle material ready for mesh
    if (prim.DrawType == RexTypes::DRAWTYPE_MESH)
    {
        Foundation::ComponentPtr mesh = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        if (mesh) 
        {
            OgreRenderer::EC_OgreMesh* meshptr = checked_static_cast<OgreRenderer::EC_OgreMesh*>(mesh.get());      
            // If don't have the actual mesh entity yet, no use trying to set the material
            if (!meshptr->GetEntity()) return;
            
            MaterialMap::const_iterator i = prim.Materials.begin();
            while (i != prim.Materials.end())
            {
                Core::uint idx = i->first;
                if ((i->second.Type == RexTypes::RexAT_MaterialScript) && (i->second.asset_id == res->GetId()))
                {
                    OgreRenderer::OgreMaterialResource *materialRes = dynamic_cast<OgreRenderer::OgreMaterialResource*>(res.get());
                    assert(materialRes);

                    Ogre::MaterialPtr mat = materialRes->GetMaterial();
                    if (!mat.get())
                    {
                        std::stringstream ss;
                        ss << std::string("Resource \"") << res->GetId() << "\" did not contain a proper Ogre::MaterialPtr!";
                        RexLogicModule::LogWarning(ss.str());
                    }
                    else
                    {
                        meshptr->SetMaterial(idx, mat->getName());
                        
                        Scene::Events::EntityEventData event_data;
                        event_data.entity = entity;
                        Foundation::EventManagerPtr event_manager = rexlogicmodule_->GetFramework()->GetEventManager();
                        event_manager->SendEvent(event_manager->QueryEventCategory("Scene"), Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED, &event_data);
                                    
                        //std::stringstream ss;
                        //ss << std::string("Set submesh ") << idx << " to use material \"" << mat->getName() << "\"";
                        //RexLogicModule::LogDebug(ss.str());
                    }
                }
                ++i;
            }
        }
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
    if (!entity)
        return;
        
    EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent(EC_OpenSimPrim::NameStatic()).get());            
    Foundation::ComponentPtr placeable = entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());  
    if (!placeable)
        return;
        
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

void SkipTextureEntrySection(const uint8_t* bytes, int& idx, int length, int elementsize)
{
    idx += elementsize; // Default value
    uint32_t bits;
    int num_bits;
    
    while ((idx < length) && (ReadTextureEntryBits(bits, num_bits, bytes, idx)))
    {
        if (idx >= length)
            return;
        idx += elementsize; // More values
    }
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
    prim.PrimDefaultTextureID = default_texture_id.ToString();
    
    while ((idx < length) && (ReadTextureEntryBits(bits, num_bits, bytes, idx)))
    {
        if (idx >= length)
            return;
        RexTypes::RexUUID texture_id = ReadUUIDFromBytes(bytes, idx);
        for (int i = 0; i < num_bits; ++i)
        {
            if (bits & 1)
            {
                prim.PrimTextures[i] = texture_id.ToString();
            }
            bits >>= 1;
        }
    }
    
    if (idx >= length)
        return;
    
    Core::Color default_color = ReadColorFromBytesInverted(bytes, idx);
    prim.PrimDefaultColor = default_color;
    
    while ((idx < length) && (ReadTextureEntryBits(bits, num_bits, bytes, idx)))
    {
        if (idx >= length)
            return;
        Core::Color color = ReadColorFromBytesInverted(bytes, idx);
        for (int i = 0; i < num_bits; ++i)
        {
            if (bits & 1)
            {
                prim.PrimColors[i] = color;
            }
            bits >>= 1;
        }
    }
    
    SkipTextureEntrySection(bytes, idx, length, 4); // RepeatU
    SkipTextureEntrySection(bytes, idx, length, 4); // RepeatV
    SkipTextureEntrySection(bytes, idx, length, 2); // OffsetU
    SkipTextureEntrySection(bytes, idx, length, 2); // OffsetV
    SkipTextureEntrySection(bytes, idx, length, 2); // Rotation
    
    uint8_t default_materialtype = bytes[idx++];
    prim.PrimDefaultMaterialType = default_materialtype;
    
    while ((idx < length) && (ReadTextureEntryBits(bits, num_bits, bytes, idx)))
    {
        if (idx >= length)
            return;
        uint8_t materialtype = bytes[idx++];
        for (int i = 0; i < num_bits; ++i)
        {
            if (bits & 1)
            {
                prim.PrimMaterialTypes[i] = materialtype;
            }
            bits >>= 1;
        }
    }
}

} // namespace RexLogic
