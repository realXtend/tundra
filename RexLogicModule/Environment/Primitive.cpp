/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Primitive.cpp
 *  @brief  Primitive logic handler.
 */

#include "StableHeaders.h"

#include "RexNetworkUtils.h"
#include "RexLogicModule.h"
#include "EntityComponent/EC_FreeData.h"
#include "EntityComponent/EC_AttachedSound.h"
#include "Environment/Primitive.h"

#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "EC_AnimationController.h"
#include "EC_OgreCustomObject.h"
#include "EC_OgreLight.h"
#include "EC_OgreParticleSystem.h"

#include "OgreTextureResource.h"
#include "OgreMaterialResource.h"
#include "OgreMeshResource.h"
#include "OgreParticleResource.h"
#include "OgreSkeletonResource.h"
#include "OgreMaterialUtils.h"
#include "Renderer.h"
#include "QuatUtils.h"

#include "SceneEvents.h"
#include "ResourceInterface.h"
#include "Environment/PrimGeometryUtils.h"
#include "SceneManager.h"
#include "AssetServiceInterface.h"
#include "ISoundService.h"
#include "GenericMessageUtils.h"
#include "EventManager.h"
#include "ServiceManager.h"
#include "WorldStream.h"

#include "EC_NetworkPosition.h"
#ifdef EC_HoveringText_ENABLED
#include "EC_HoveringText.h"
#endif
#include "EC_OpenSimPrim.h"

#include "IAttribute.h"

#include <OgreSceneNode.h>

#include <QUrl>
#include <QColor>
#include <QDomDocument>

namespace RexLogic
{

Primitive::Primitive(RexLogicModule *rexlogicmodule) : rexlogicmodule_(rexlogicmodule)
{
}

Primitive::~Primitive()
{
}

void Primitive::Update(f64 frametime)
{
    SerializeECsToNetwork();
}

Scene::EntityPtr Primitive::GetOrCreatePrimEntity(entity_id_t entityid, const RexUUID &fullid, bool *created)
{
    // Make sure scene exists
    Scene::ScenePtr scene = rexlogicmodule_->GetFramework()->GetDefaultWorldScene();
    if (!scene)
        return Scene::EntityPtr();

    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity)
    {
        // Create a new entity.
        Scene::EntityPtr entity = CreateNewPrimEntity(entityid);
        rexlogicmodule_->RegisterFullId(fullid,entityid); 
        EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
        
        // Connect to property changes to be sent to sim
        connect(prim, SIGNAL(RexPrimDataChanged(Scene::Entity*)), SLOT(OnRexPrimDataChanged(Scene::Entity*)));
        connect(prim, SIGNAL(PrimShapeChanged(const EC_OpenSimPrim&)), SLOT(OnPrimShapeChanged(const EC_OpenSimPrim&)));
        connect(prim, SIGNAL(PrimNameChanged(const EC_OpenSimPrim&)), SLOT(OnPrimNameChanged(const EC_OpenSimPrim&)));
        connect(prim, SIGNAL(PrimDescriptionChanged(const EC_OpenSimPrim&)), SLOT(OnPrimDescriptionChanged(const EC_OpenSimPrim&)));

        prim->LocalId = entityid; ///\note In current design it holds that localid == entityid, but I'm not sure if this will always be so?
        prim->FullId = fullid;
        CheckPendingRexPrimData(entityid);
        CheckPendingRexFreeData(entityid);
        *created = true;
        return entity;
    }

    // Send the 'Entity Updated' event.
    /*
    event_category_id_t cat_id = framework_->GetEventManager()->QueryEventCategory("Scene");
    ComponentPtr component = entity->GetComponent(EC_OpenSimPrim::TypeNameStatic());
    EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(component.get());
    Scene::SceneEventData::Events entity_event_data(entityid);
    entity_event_data.sceneName = scene->Name();
    framework_->GetEventManager()->SendEvent(cat_id, Scene::Events::EVENT_ENTITY_UPDATED, &entity_event_data);
    */
    *created = false;
    return entity;
}

Scene::EntityPtr Primitive::CreateNewPrimEntity(entity_id_t entityid)
{
    Scene::ScenePtr scene = rexlogicmodule_->GetFramework()->GetDefaultWorldScene();
    if (!scene)
        return Scene::EntityPtr();

    QStringList components;
    components.append(EC_OpenSimPrim::TypeNameStatic());
    components.append(EC_NetworkPosition::TypeNameStatic());
    components.append(EC_Placeable::TypeNameStatic());

    // Create entity with all default components non-networksynced
    Scene::EntityPtr entity = scene->CreateEntity(entityid, components, AttributeChange::LocalOnly, false);

    return entity;
}

bool Primitive::HandleOSNE_ObjectUpdate(ProtocolUtilities::NetworkEventInboundData* data)
{
    ProtocolUtilities::NetInMessage *msg = data->message;

    msg->ResetReading();
    uint64_t regionhandle = msg->ReadU64();
    msg->SkipToNextVariable(); // TimeDilation U16

    // Variable block: Object Data
    size_t instance_count = data->message->ReadCurrentBlockInstanceCount();
    for(size_t i = 0; i < instance_count; ++i)
    {
        uint32_t localid = msg->ReadU32();
        msg->SkipToNextVariable();        // State U8
        RexUUID fullid = msg->ReadUUID();
        msg->SkipToNextVariable();        // CRC U32
        uint8_t pcode = msg->ReadU8();
        bool was_created;

        Scene::EntityPtr entity = GetOrCreatePrimEntity(localid, fullid, &was_created);
        EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
        EC_NetworkPosition *netpos = entity->GetComponent<EC_NetworkPosition>().get();

        ///\todo Are we setting the param or looking up by this param? I think the latter, but this is now doing the former. 
        ///      Will cause problems with multigrid support.
        prim->RegionHandle = regionhandle;

        prim->Material = msg->ReadU8();
        prim->ClickAction = msg->ReadU8();

        prim->Scale = msg->ReadVector3();
        // Scale is not handled by interpolation system, so set directly
        HandlePrimScaleAndVisibility(localid);

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

            Vector3df vec = (*reinterpret_cast<const Vector3df*>(&objectdatabytes[0]));
            if (IsValidPositionVector(vec))
                netpos->position_ = vec;

            vec = *reinterpret_cast<const Vector3df*>(&objectdatabytes[12]);
            if (IsValidVelocityVector(vec))
                netpos->velocity_ = vec;

            vec = *reinterpret_cast<const Vector3df*>(&objectdatabytes[24]);
            if (IsValidVelocityVector(vec)) // Use Velocity validation for Acceleration as well - it's ok as they are quite similar.
                netpos->accel_ = vec;

            netpos->orientation_ = UnpackQuaternionFromFloat3((float*)&objectdatabytes[36]);
            vec = *reinterpret_cast<const Vector3df*>(&objectdatabytes[48]);
            if (IsValidVelocityVector(vec)) // Use Velocity validation for Angular Velocity as well - it's ok as they are quite similar.
                netpos->rotvel_ = vec;
            netpos->Updated();
        }
        else
            RexLogicModule::LogError("Error reading ObjectData for prim:" + ToString(prim->LocalId) + ". Bytes read:" + ToString(bytes_read));

        prim->ParentId = msg->ReadU32();
        prim->UpdateFlags = msg->ReadU32();

        // Read prim shape
        prim->PathCurve.Set(msg->ReadU8(), AttributeChange::LocalOnly);
        prim->ProfileCurve.Set(msg->ReadU8(), AttributeChange::LocalOnly);
        prim->PathBegin.Set(msg->ReadU16() * 0.00002f, AttributeChange::LocalOnly);
        prim->PathEnd.Set(msg->ReadU16() * 0.00002f, AttributeChange::LocalOnly);
        prim->PathScaleX.Set(msg->ReadU8() * 0.01f, AttributeChange::LocalOnly);
        prim->PathScaleY.Set(msg->ReadU8() * 0.01f, AttributeChange::LocalOnly);
        prim->PathShearX.Set(((int8_t)msg->ReadU8()) * 0.01f, AttributeChange::LocalOnly);
        prim->PathShearY.Set(((int8_t)msg->ReadU8()) * 0.01f, AttributeChange::LocalOnly);
        prim->PathTwist.Set(msg->ReadS8() * 0.01f, AttributeChange::LocalOnly);
        prim->PathTwistBegin.Set(msg->ReadS8() * 0.01f, AttributeChange::LocalOnly);
        prim->PathRadiusOffset.Set(msg->ReadS8() * 0.01f, AttributeChange::LocalOnly);
        prim->PathTaperX.Set(msg->ReadS8() * 0.01f, AttributeChange::LocalOnly);
        prim->PathTaperY.Set(msg->ReadS8() * 0.01f, AttributeChange::LocalOnly);
        prim->PathRevolutions.Set(1.0f + msg->ReadU8() * 0.015f, AttributeChange::LocalOnly);
        prim->PathSkew.Set( msg->ReadS8() * 0.01f, AttributeChange::LocalOnly);
        prim->ProfileBegin.Set(msg->ReadU16() * 0.00002f, AttributeChange::LocalOnly);
        prim->ProfileEnd.Set(msg->ReadU16() * 0.00002f, AttributeChange::LocalOnly);
        prim->ProfileHollow.Set(msg->ReadU16() * 0.00002f, AttributeChange::LocalOnly);
        prim->HasPrimShapeData = true;

        // Texture entry
        const uint8_t *textureentrybytes = msg->ReadBuffer(&bytes_read);
        ParseTextureEntryData(*prim, textureentrybytes, bytes_read);

        // Hovering text
        msg->SkipToFirstVariableByName("Text");
        prim->HoveringText = msg->ReadString();

        // Text color
        const uint8_t *colorBytes = msg->ReadBuffer(&bytes_read);
        assert(bytes_read == 4 && "Invalid length for fixed-sized variable TextColor in ObjectUpdate packet! Should be 4 bytes always.");
        if (bytes_read != 4)
            throw Exception("Invalid length for fixed-sized variable TextColor in ObjectUpdate packet! Should be 4 bytes always.");

        // Convert from bytes to QColor
        int idx = 0;
        int r = colorBytes[idx++];
        int g = colorBytes[idx++];
        int b = colorBytes[idx++];
        int a = 255 - colorBytes[idx++];
        QColor color(r, g, b, a);

        AttachHoveringTextComponent(entity, prim->HoveringText, color);

        // read mediaurl, and send an event if it was changed
        std::string prevMediaUrl = prim->MediaUrl;
        prim->MediaUrl = msg->ReadString();
        //RexLogicModule::LogInfo("MediaURL: " + prim->MediaUrl);
        if (prim->MediaUrl.compare(prevMediaUrl) != 0)
        {
            //RexLogicModule::LogInfo("MediaURL changed: " + prim->MediaUrl);
            Scene::Events::EntityEventData event_data;
            event_data.entity = entity;
            Foundation::EventManagerPtr event_manager = rexlogicmodule_->GetFramework()->GetEventManager();
            event_manager->SendEvent("Scene", Scene::Events::EVENT_ENTITY_MEDIAURL_SET, &event_data);
        }

        msg->SkipToNextVariable(); // PSBlock

        // If there are extra params, handle them.
        if (msg->ReadVariableSize() > 1)
        {
            const uint8_t *extra_params_data = msg->ReadBuffer(&bytes_read);
            HandleExtraParams(localid, extra_params_data);
        }

        msg->SkipToNextInstanceStart();

        HandleDrawType(localid);

        // Handle setting the prim as child of another object, or possibly being parent itself
        rexlogicmodule_->HandleMissingParent(localid);
        rexlogicmodule_->HandleObjectParent(localid);
        if (was_created)
        {
            Scene::ScenePtr scene = rexlogicmodule_->GetFramework()->GetDefaultWorldScene();
            if (scene)
                scene->EmitEntityCreated(entity, AttributeChange::LocalOnly);
        }
    }

    return false;
}

void Primitive::HandleTerseObjectUpdateForPrim_44bytes(const uint8_t* bytes)
{
    // The data contents:
    // ofs  0 - localid - packed to 4 bytes
    // ofs  4 - state (attachment point)
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
    EC_NetworkPosition *netpos = entity->GetComponent<EC_NetworkPosition>().get();

    Vector3df vec = GetProcessedVector(&bytes[i]);
    if (IsValidPositionVector(vec))
        netpos->position_ = vec;
    i += sizeof(Vector3df);

    netpos->velocity_ = GetProcessedScaledVectorFromUint16(&bytes[i],128);
    i += 6;

    netpos->accel_ = GetProcessedVectorFromUint16(&bytes[i]); 
    i += 6;

    netpos->orientation_ = GetProcessedQuaternion(&bytes[i]);
    i += 8;

    netpos->rotvel_ = GetProcessedScaledVectorFromUint16(&bytes[i],128);
    i += 6;

    netpos->Updated();
    assert(i <= 44);
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
    EC_NetworkPosition *netpos = entity->GetComponent<EC_NetworkPosition>().get();

    Vector3df vec = GetProcessedVector(&bytes[i]);
    if (IsValidPositionVector(vec))
        netpos->position_ = vec;
    i += sizeof(Vector3df);

    netpos->velocity_ = GetProcessedScaledVectorFromUint16(&bytes[i],128);
    i += 6;

    netpos->accel_ = GetProcessedVectorFromUint16(&bytes[i]); 
    i += 6;

    netpos->orientation_ = GetProcessedQuaternion(&bytes[i]);
    i += 8;

    netpos->rotvel_ = GetProcessedScaledVectorFromUint16(&bytes[i],128);
    i += 16;

    netpos->Updated();
    assert(i <= 60);
}

bool Primitive::HandleRexGM_RexMediaUrl(ProtocolUtilities::NetworkEventInboundData* data)
{
    // handled now in pymodules/mediaurlhandler/
    return false;
}

bool Primitive::HandleRexGM_RexPrimAnim(ProtocolUtilities::NetworkEventInboundData* data)
{
    StringVector params = ProtocolUtilities::ParseGenericMessageParameters(*data->message);

    // Should have 5 parameters:
    // 0: prim id
    // 1: animation name
    // 2: animation speed
    // 3: looped
    // 4: stop flag (false = start animation, true = stop)
    if (params.size() < 5)
        return false;    
    // Animation speed may have , instead of . so replace
    ReplaceCharInplace(params[2], ',', '.');
    
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(RexUUID(params[0]));
    if (!entity)
        return false;
    
    // Entity should have animationcontroller
    EC_AnimationController* anim = dynamic_cast<EC_AnimationController*>(
        entity->GetOrCreateComponent(EC_AnimationController::TypeNameStatic()).get());
    if (!anim)
        return false;
    // Attach the mesh entity to animationcontroller, if not yet attached
    //ComponentPtr mesh = entity->GetComponent(EC_Mesh::TypeNameStatic());
    ComponentPtr mesh = entity->GetComponent<EC_Mesh>("rex");
    if (!mesh)
        return false;
    EC_Mesh *ogre_mesh = dynamic_cast<EC_Mesh*>(mesh.get());
    if (anim->GetMeshEntity() != ogre_mesh)
        anim->SetMeshEntity(ogre_mesh);
    
    try
    {
        float rate = ParseString<float>(params[2]);
        bool looped = ParseBool(params[3]);
        bool stop = ParseBool(params[4]);
        
        if (stop)
        {
            anim->DisableAnimation(params[1], 0.25f);
        }
        else
        {
            anim->EnableAnimation(params[1], looped, 0.025f);
            anim->SetAnimationSpeed(params[1], rate);
            if (rate < 0.0f)
                anim->SetAnimationToEnd(params[1]);
            anim->SetAnimationAutoStop(params[1], looped == false);
        }
    }
    catch (...) {}  
        
    return false;
        
}    

bool Primitive::HandleRexGM_RexPrimData(ProtocolUtilities::NetworkEventInboundData* data)
{
    std::vector<u8> fulldata;
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
        const u8* readbytedata = data->message->ReadBuffer(&bytes_read);
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

bool Primitive::HandleRexGM_RexFreeData(ProtocolUtilities::NetworkEventInboundData* data)
{
    StringVector params = ProtocolUtilities::ParseGenericMessageParameters(*data->message);
    
    if (params.size() < 2)
        return false;
    
    // First parameter: prim id
    RexUUID primuuid(params[0]);
    // Rest of parameters: free data in pieces
    std::string freedata;
    for (uint i = 1; i < params.size(); ++i)
        freedata.append(params[i]);
    
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(primuuid);
    // If cannot get the entity, put to pending rexfreedata
    if (entity)
        HandleRexFreeData(entity->GetId(), freedata);
    else
        pending_rexfreedata_[primuuid] = freedata;
    
    return false;
}

void Primitive::CheckPendingRexPrimData(entity_id_t entityid)
{
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity) return;
    EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();

    RexPrimDataMap::iterator i = pending_rexprimdata_.find(prim->FullId);
    if (i != pending_rexprimdata_.end())
    {
        HandleRexPrimDataBlob(entityid, &i->second[0], i->second.size());
        pending_rexprimdata_.erase(i);
    }
}

void Primitive::CheckPendingRexFreeData(entity_id_t entityid)
{
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity) return;
    EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();

    RexFreeDataMap::iterator i = pending_rexfreedata_.find(prim->FullId);
    if (i != pending_rexfreedata_.end())
    {
        HandleRexFreeData(entityid, i->second);
        pending_rexfreedata_.erase(i);
    }
}

void Primitive::SendRexPrimData(entity_id_t entityid)
{
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity)
        return;

    EC_OpenSimPrim* prim = entity->GetComponent<EC_OpenSimPrim>().get();
    
    RexUUID fullid = prim->FullId;
    
    std::vector<uint8_t> buffer;
    buffer.resize(4096);
    int idx = 0;
    bool send_asset_urls = false;
    
    // graphical values
    WriteUInt8ToBytes(prim->DrawType, &buffer[0], idx);
    WriteBoolToBytes(prim->IsVisible.Get(), &buffer[0], idx);
    WriteBoolToBytes(prim->CastShadows.Get(), &buffer[0], idx);
    WriteBoolToBytes(prim->LightCreatesShadows, &buffer[0], idx);
    WriteBoolToBytes(prim->DescriptionTexture, &buffer[0], idx);
    WriteBoolToBytes(prim->ScaleToPrim, &buffer[0], idx);
    WriteFloatToBytes(prim->DrawDistance.Get(), &buffer[0], idx);
    WriteFloatToBytes(prim->LOD.Get(), &buffer[0], idx);   
    
    // UUIDs
    // Note: if the EC contains asset urls that can not be encoded as UUIDs, we still have to send
    // invalid (null) UUIDs to retain binary compatibility with the rexprimdatablob
    
    if (IsUrlBased(prim->MeshID) || IsUrlBased(prim->CollisionMeshID) || IsUrlBased(prim->ParticleScriptID) || IsUrlBased(prim->AnimationPackageID))
        send_asset_urls = true;
        
    WriteUUIDToBytes(UuidForRexObjectUpdatePacket(prim->MeshID), &buffer[0], idx);
    WriteUUIDToBytes(UuidForRexObjectUpdatePacket(prim->CollisionMeshID), &buffer[0], idx);
    WriteUUIDToBytes(UuidForRexObjectUpdatePacket(prim->ParticleScriptID), &buffer[0], idx);

    // Animation
    WriteUUIDToBytes(UuidForRexObjectUpdatePacket(prim->AnimationPackageID), &buffer[0], idx);
    WriteNullTerminatedStringToBytes(prim->AnimationName, &buffer[0], idx);
    WriteFloatToBytes(prim->AnimationRate, &buffer[0], idx);

    // Materials
    size_t mat_count = prim->Materials.size();
    MaterialMap::const_iterator i = prim->Materials.begin();
    WriteUInt8ToBytes((uint8_t)mat_count, &buffer[0], idx); 
    while (i != prim->Materials.end())
    {
        // Write assettype, uuid, index for each
        if (IsUrlBased(i->second.asset_id))
            send_asset_urls = true;
        WriteUInt8ToBytes(i->second.Type, &buffer[0], idx);
        WriteUUIDToBytes(UuidForRexObjectUpdatePacket(i->second.asset_id), &buffer[0], idx);
        WriteUInt8ToBytes(i->first, &buffer[0], idx);
        ++i;
    }

    WriteNullTerminatedStringToBytes(prim->ServerScriptClass, &buffer[0], idx);

    // Sound
    if (IsUrlBased(prim->SoundID))
        send_asset_urls = true;
    WriteUUIDToBytes(UuidForRexObjectUpdatePacket(prim->SoundID), &buffer[0], idx);
    WriteFloatToBytes(prim->SoundVolume, &buffer[0], idx);
    WriteFloatToBytes(prim->SoundRadius, &buffer[0], idx);
    
    WriteUInt32ToBytes(prim->SelectPriority, &buffer[0], idx);

    // Extension: url based asset id's
    if (send_asset_urls)
    {
        WriteNullTerminatedStringToBytes(UrlForRexObjectUpdatePacket(prim->MeshID), &buffer[0], idx);
        WriteNullTerminatedStringToBytes(UrlForRexObjectUpdatePacket(prim->CollisionMeshID), &buffer[0], idx);
        WriteNullTerminatedStringToBytes(UrlForRexObjectUpdatePacket(prim->ParticleScriptID), &buffer[0], idx);
        WriteNullTerminatedStringToBytes(UrlForRexObjectUpdatePacket(prim->AnimationPackageID), &buffer[0], idx);
        WriteNullTerminatedStringToBytes(UrlForRexObjectUpdatePacket(prim->SoundID), &buffer[0], idx);
        i = prim->Materials.begin();
        while (i != prim->Materials.end())
        {
            WriteNullTerminatedStringToBytes(UrlForRexObjectUpdatePacket(i->second.asset_id), &buffer[0], idx);
            ++i;
        }
    }

    buffer.resize(idx);

    WorldStreamPtr conn = rexlogicmodule_->GetServerConnection();
    if (!conn)
        return;
    StringVector strings;
    strings.push_back(fullid.ToString());
    conn->SendGenericMessageBinary("RexPrimData", strings, buffer);
}

void Primitive::SendRexFreeData(entity_id_t entityid)
{
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity)
        return;

    EC_OpenSimPrim* prim = entity->GetComponent<EC_OpenSimPrim>().get();
    EC_FreeData* free = entity->GetComponent<EC_FreeData>().get();
    if (!free || !prim )
        return;

    WorldStreamPtr conn = rexlogicmodule_->GetServerConnection();
    if (!conn)
        return;

    RexUUID fullid = prim->FullId;
    StringVector strings;
    strings.push_back(fullid.ToString());
    const std::string& freedata = free->FreeData;

    // Split freedata into chunks of 200
    for (uint i = 0; i < freedata.length(); i += 200)
    {
        uint j = i + 200;
        if (j > freedata.length())
            j = freedata.length();
        strings.push_back(freedata.substr(i, j-i));
    }
    conn->SendGenericMessage("RexData", strings);
}

void Primitive::HandleRexPrimDataBlob(entity_id_t entityid, const uint8_t* primdata, const int primdata_size)
{
    int idx = 0;

    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity)
        return;

    EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();

    // graphical values
    prim->DrawType = ReadUInt8FromBytes(primdata,idx);
    prim->IsVisible.Set(ReadBoolFromBytes(primdata,idx), AttributeChange::LocalOnly);
    prim->CastShadows.Set(ReadBoolFromBytes(primdata,idx), AttributeChange::LocalOnly);
    prim->LightCreatesShadows = ReadBoolFromBytes(primdata,idx);
    prim->DescriptionTexture = ReadBoolFromBytes(primdata,idx);
    prim->ScaleToPrim = ReadBoolFromBytes(primdata,idx);
    prim->DrawDistance.Set(ReadFloatFromBytes(primdata,idx), AttributeChange::LocalOnly);
    prim->LOD.Set(ReadFloatFromBytes(primdata,idx), AttributeChange::LocalOnly);

    prim->MeshID = ReadUUIDFromBytes(primdata,idx).ToString();
    prim->CollisionMeshID = ReadUUIDFromBytes(primdata,idx).ToString();
    prim->ParticleScriptID = ReadUUIDFromBytes(primdata,idx).ToString();

    // animation
    prim->AnimationPackageID = ReadUUIDFromBytes(primdata,idx).ToString();
    prim->AnimationName = ReadNullTerminatedStringFromBytes(primdata,idx);
    prim->AnimationRate = ReadFloatFromBytes(primdata,idx);

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
    prim->Materials = materials;

    prim->ServerScriptClass = ReadNullTerminatedStringFromBytes(primdata,idx);

    // sound
    prim->SoundID = ReadUUIDFromBytes(primdata,idx).ToString();
    prim->SoundVolume = ReadFloatFromBytes(primdata,idx);
    prim->SoundRadius = ReadFloatFromBytes(primdata,idx);

    prim->SelectPriority = ReadUInt32FromBytes(primdata,idx);

    // Copy selectpriority to the placeable for the renderer raycast
    EC_Placeable* placeable = entity->GetComponent<EC_Placeable>().get();
    if (placeable)
    {
        placeable->SetSelectPriority(prim->SelectPriority);
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
            prim->MeshID = rex_mesh_url;
        if (rex_collision_mesh_url.size() > 0)
            prim->CollisionMeshID = rex_collision_mesh_url;
        if (rex_particle_script_url.size() > 0)
            prim->ParticleScriptID = rex_particle_script_url;
        if (rex_animation_package_url.size() > 0)
            prim->AnimationPackageID = rex_animation_package_url;
        if (rex_sound_url.size() > 0)
            prim->SoundID = rex_sound_url;

        for (int i=0; i<tempmaterialcount; ++i)
        {
            std::string rex_material_url = ReadNullTerminatedStringFromBytes(primdata,idx);
            if (rex_material_url.size() > 0)
            {
                uint8_t material_index = material_indexes[i];
                materials[material_index].asset_id = rex_material_url;
            }
        }
        prim->Materials = materials;
    }
    
    // Handle any change in the drawtype of the prim. Also, 
    // the Ogre materials on this prim have possibly changed. Issue requests of the new materials 
    // from the asset provider and bind the new materials to this prim.
    HandleDrawType(entityid);
    HandlePrimScaleAndVisibility(entityid);
    // Handle sound parameters
    HandleAmbientSound(entityid);
}

void Primitive::HandleRexFreeData(entity_id_t entityid, const std::string& freedata)
{
    int idx = 0;

    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity)
        return;

    // Get/create freedata component
    ComponentPtr freeptr = entity->GetOrCreateComponent(EC_FreeData::TypeNameStatic());
    if (!freeptr)
        return;
    EC_FreeData& free = *(dynamic_cast<EC_FreeData*>(freeptr.get()));
    free.FreeData = freedata;
    
    // Parse into XML form (may or may not succeed), and create/update EC's as result
    // (primitive form of EC serialization/replication)
    QDomDocument temp_doc;
    if (temp_doc.setContent(QByteArray::fromRawData(freedata.c_str(), freedata.size())))
    {
        DeserializeECsFromFreeData(entity, temp_doc);
        Scene::Events::SceneEventData event_data(entity->GetId());
        Foundation::EventManagerPtr event_manager = rexlogicmodule_->GetFramework()->GetEventManager();
        event_manager->SendEvent("Scene", Scene::Events::EVENT_ENTITY_ECS_RECEIVED, &event_data);
    }
}

bool Primitive::HandleOSNE_KillObject(uint32_t objectid)
{
    Scene::ScenePtr scene = rexlogicmodule_->GetFramework()->GetDefaultWorldScene();
    if (!scene)
        return false;

    RexUUID fullid, childfullid;
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(objectid);
    if(!entity)
        return false;

    EC_OpenSimPrim* prim = entity->GetComponent<EC_OpenSimPrim>().get();
    if (prim)
        fullid = prim->FullId;

    //need to remove children aswell... ///\todo is there a better way of doing this?
    for(Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = **iter;
        EC_OpenSimPrim *prim = entity.GetComponent<EC_OpenSimPrim>().get();
        if (!prim)
            continue;

        if (prim->ParentId == objectid)
        {
            childfullid = prim->FullId;
            scene->RemoveEntity(prim->LocalId);
            rexlogicmodule_->UnregisterFullId(childfullid);
        }
    }

    scene->RemoveEntity(objectid);
    rexlogicmodule_->UnregisterFullId(fullid);
    return false;
}

bool Primitive::HandleOSNE_ObjectProperties(ProtocolUtilities::NetworkEventInboundData* data)
{
    ProtocolUtilities::NetInMessage *msg = data->message;
    msg->ResetReading();
    
    RexUUID full_id = msg->ReadUUID();
    msg->SkipToFirstVariableByName("Name");
    std::string name = msg->ReadString();
    std::string desc = msg->ReadString();
    ///\todo Handle rest of the vars.
    
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(full_id);
    if (entity)
    {
        EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
        prim->Name.Set(QString::fromStdString(name), AttributeChange::LocalOnly);
        prim->Description.Set(QString::fromStdString(desc), AttributeChange::LocalOnly);
        
        ///\todo Odd behavior? The ENTITY_SELECTED event is passed only after the server responds with an ObjectProperties
        /// message. Should we maintain our own notion of what's selected and rename this event to PRIM_OBJECT_PROPERTIES or
        /// something similar? Or is it desired that the ObjectProperties wire message defines exactly what objects the
        /// client has selected?

        // Send the 'Entity Selected' event.
        /*
        event_category_id_t event_category_id = rexlogicmodule_->GetFramework()->GetEventManager()->QueryEventCategory("Scene");
        Scene::Events::SceneEventData event_data(prim->LocalId);
        rexlogicmodule_->GetFramework()->GetEventManager()->SendEvent(event_category_id, Scene::Events::EVENT_ENTITY_SELECTED, &event_data);
        */
    }
    else
        RexLogicModule::LogInfo("Received 'ObjectProperties' packet for unknown entity (" + full_id.ToString() + ").");
    
    return false;
}

///\todo We now pass the entityid in the function. It is assumed that the entity contains exactly one EC_OpenSimPrim and one EC_Mesh component.
/// Possibly in the future an entity can have several meshes attached to it, so we should pass in the EC_Mesh in question.
void Primitive::HandleDrawType(entity_id_t entityid)
{
    ///\todo Make this only discard mesh resource request tags.
    // Discard old request tags for this entity
    DiscardRequestTags(entityid, prim_resource_request_tags_);

    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity)
        return;

    EC_OpenSimPrim &prim = *(entity->GetComponent<EC_OpenSimPrim>().get());
    if ((prim.DrawType == RexTypes::DRAWTYPE_MESH) && (!RexTypes::IsNull(prim.MeshID)))
    {
        // Remove custom object component if exists
        ComponentPtr customptr = entity->GetComponent(EC_OgreCustomObject::TypeNameStatic());
        if (customptr)
            entity->RemoveComponent(customptr);

        // Get/create mesh component 
        //ComponentPtr meshptr = entity->GetOrCreateComponent(EC_Mesh::TypeNameStatic());
        ComponentPtr meshptr = entity->GetOrCreateComponent(EC_Mesh::TypeNameStatic(), "rex");
        if (!meshptr)
            return;
        EC_Mesh& mesh = *(dynamic_cast<EC_Mesh*>(meshptr.get()));
        // Make sure network sync is off
        mesh.SetNetworkSyncEnabled(false);
        
        // Attach to placeable if not yet attached
        if (!mesh.GetPlaceable())
            mesh.SetPlaceable(entity->GetComponent(EC_Placeable::TypeNameStatic()));
        
        // Change mesh if yet nonexistent/changed
        // assume name to be UUID of mesh asset, which should be true of OgreRenderer resources
        const std::string& mesh_name = prim.MeshID;
        if ((!RexTypes::IsNull(mesh_name)) && (mesh.GetMeshName() != mesh_name))
        {
            boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
                GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
            if (renderer)
            {
                request_tag_t tag = renderer->RequestResource(mesh_name, OgreRenderer::OgreMeshResource::GetTypeStatic());

                // Remember that we are going to get a resource event for this entity
                if (tag)
                    prim_resource_request_tags_[std::make_pair(tag, RexTypes::RexAT_Mesh)] = entityid;
            }
        }
        
        // Load mesh skeleton if used/specified
        const std::string& skeleton_name = prim.AnimationPackageID;
        if ((!RexTypes::IsNull(skeleton_name)) && (mesh.GetSkeletonName() != skeleton_name))
        {
            boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
                GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
            if (renderer)
            {
                request_tag_t tag = renderer->RequestResource(skeleton_name, OgreRenderer::OgreSkeletonResource::GetTypeStatic());

                // Remember that we are going to get a resource event for this entity
                if (tag)
                    prim_resource_request_tags_[std::make_pair(tag, RexTypes::RexAT_Skeleton)] = entityid;
            }
        }
        
        
        // Set rendering distance & shadows
        mesh.SetDrawDistance(prim.DrawDistance.Get());
        mesh.SetCastShadows(prim.CastShadows.Get());
        
        // Check/request mesh textures
        HandleMeshMaterials(entityid);
        
        // Check/set animation
        HandleMeshAnimation(entityid);
    }
    else if (prim.DrawType == RexTypes::DRAWTYPE_PRIM)
    {
        // Remove mesh component if exists
        //ComponentPtr meshptr = entity->GetComponent(EC_Mesh::TypeNameStatic());
        ComponentPtr meshptr = entity->GetComponent<EC_Mesh>("rex");
        if (meshptr)
            entity->RemoveComponent(meshptr);
        // Detach from animationcontroller
        EC_AnimationController* anim =
            entity->GetComponent<EC_AnimationController>().get();
        if (anim)
            anim->SetMeshEntity(0);

        // Get/create custom (manual) object component 
        ComponentPtr customptr = entity->GetOrCreateComponent(EC_OgreCustomObject::TypeNameStatic());
        if (!customptr)
            return;
        EC_OgreCustomObject& custom = *(checked_static_cast<EC_OgreCustomObject*>(customptr.get()));

        // Attach to placeable if not yet attached
        if (!custom.GetPlaceable())
            custom.SetPlaceable(entity->GetComponent(EC_Placeable::TypeNameStatic()));

        // Set rendering distance/cast shadows setting
        custom.SetDrawDistance(prim.DrawDistance.Get());
        custom.SetCastShadows(prim.CastShadows.Get());

        // Request prim textures
        HandlePrimTexturesAndMaterial(entityid);

        // Create/update geometry
        if (prim.HasPrimShapeData)
        {
            Ogre::ManualObject* manual = CreatePrimGeometry(rexlogicmodule_->GetFramework(), prim);
            custom.CommitChanges(manual);
            
            Scene::Events::EntityEventData event_data;
            event_data.entity = entity;
            Foundation::EventManagerPtr event_manager = rexlogicmodule_->GetFramework()->GetEventManager();
            event_manager->SendEvent("Scene", Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED, &event_data);
        }
    }

    if (!RexTypes::IsNull(prim.ParticleScriptID))
    {
        // Create particle system component & attach, if does not yet exist
        ComponentPtr particleptr = entity->GetOrCreateComponent(EC_OgreParticleSystem::TypeNameStatic());
         if (!particleptr)
            return;
        EC_OgreParticleSystem& particle = *(checked_static_cast<EC_OgreParticleSystem*>(particleptr.get()));
        
        // Attach to placeable if not yet attached
        if (!particle.GetPlaceable())
            particle.SetPlaceable(entity->GetComponent(EC_Placeable::TypeNameStatic()));
            
        // Change particle system if yet nonexistent/changed
        const std::string& script_name = prim.ParticleScriptID;
        
        if (particle.GetParticleSystemName(0).find(script_name) == std::string::npos)
        {
            boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
                GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
            if (renderer)
            {
                request_tag_t tag = renderer->RequestResource(script_name, OgreRenderer::OgreParticleResource::GetTypeStatic());

                // Remember that we are going to get a resource event for this entity
                if (tag)
                    prim_resource_request_tags_[std::make_pair(tag, RexTypes::RexAT_ParticleScript)] = entityid;
            }
        }
    }
    else
    {
        // If should be no particle system, remove it if exists
        ComponentPtr particleptr = entity->GetComponent(EC_OgreParticleSystem::TypeNameStatic());
        if (particleptr)
        {
            entity->RemoveComponent(particleptr);
        }
    }
}

void Primitive::HandlePrimTexturesAndMaterial(entity_id_t entityid)
{
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity) 
        return;
    EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
    
    boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
        GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    if (!renderer)
        return;

    // Check for prim material override
    if ((prim->Materials.size()) && (prim->Materials[0].Type == RexTypes::RexAT_MaterialScript) && (!RexTypes::IsNull(prim->Materials[0].asset_id)))
    {
        std::string matname = prim->Materials[0].asset_id;
        
        // Request material if don't have it yet
        if (!renderer->GetResource(matname, OgreRenderer::OgreMaterialResource::GetTypeStatic()))
        {
            request_tag_t tag = renderer->RequestResource(matname, OgreRenderer::OgreMaterialResource::GetTypeStatic());
             
            // Remember that we are going to get a resource event for this entity
            if (tag)
                prim_resource_request_tags_[std::make_pair(tag, RexTypes::RexAT_MaterialScript)] = entityid;
        }
    }
    else
    {
        // Otherwise request normal textures
        std::set<RexTypes::RexAssetID> tex_requests;
        
        if (!RexTypes::IsNull(prim->PrimDefaultTextureID))
            tex_requests.insert(prim->PrimDefaultTextureID);
            
        TextureMap::const_iterator i = prim->PrimTextures.begin();
        while (i != prim->PrimTextures.end())
        {
            if (!RexTypes::IsNull(i->second))
                tex_requests.insert(i->second);
            ++i;
        }
        
        std::set<RexTypes::RexAssetID>::const_iterator j = tex_requests.begin();
        while (j != tex_requests.end())
        {
            std::string texname = (*j);
            
            // Request texture if don't have it yet
            if (!renderer->GetResource(texname, OgreRenderer::OgreTextureResource::GetTypeStatic()))
            {
                request_tag_t tag = renderer->RequestResource(texname, OgreRenderer::OgreTextureResource::GetTypeStatic());
             
                // Remember that we are going to get a resource event for this entity
                if (tag)
                    prim_resource_request_tags_[std::make_pair(tag, RexTypes::RexAT_Texture)] = entityid;
            }
            
            ++j;
        }
    }
}

void Primitive::HandleMeshMaterials(entity_id_t entityid)
{
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity) 
        return;
    EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();                    
    EC_Mesh* meshptr = entity->GetComponent<EC_Mesh>().get();
    if (!meshptr)
        return;
        
    boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
        GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    if (!renderer)
        return;
        
    // Loop through all the materials in the mesh 
    MaterialMap::const_iterator i = prim->Materials.begin();
    while (i != prim->Materials.end())
    {
        if (RexTypes::IsNull(i->second.asset_id))
        {
            ++i;
            continue;
        }

        const std::string mat_name = i->second.asset_id;
        uint idx = i->first;   

        //! \todo in the future material names will probably not correspond directly to texture names, so can't use this kind of check
        // If the mesh material is up-to-date, no need to process any further.
        if (meshptr->GetMaterialName(idx) == mat_name)
        {
            ++i;
            continue;
        }

        switch(i->second.Type)
        {
            case RexTypes::RexAT_TextureJPEG:
            case RexTypes::RexAT_Texture:
            {
                Foundation::ResourcePtr res = renderer->GetResource(mat_name, OgreRenderer::OgreTextureResource::GetTypeStatic());
                if (res)
                    HandleTextureReady(entityid, res);
                else
                {
                    request_tag_t tag = renderer->RequestResource(mat_name, OgreRenderer::OgreTextureResource::GetTypeStatic());

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
                    request_tag_t tag = renderer->RequestResource(mat_name, OgreRenderer::OgreMaterialResource::GetTypeStatic());

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

void Primitive::HandleMeshAnimation(entity_id_t entityid)
{
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity) return;
    EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
    
    // Set animation now if applicable
    if ((!RexTypes::IsNull(prim->AnimationPackageID)) && (!prim->AnimationName.empty()))
    {
        // Entity should have animationcontroller
        EC_AnimationController* anim = dynamic_cast<EC_AnimationController*>(
            entity->GetOrCreateComponent(EC_AnimationController::TypeNameStatic()).get());
        if (anim)
        {
            // Attach the mesh entity to animationcontroller, if not yet attached
            //ComponentPtr mesh = entity->GetComponent(EC_Mesh::TypeNameStatic());
            ComponentPtr mesh = entity->GetComponent<EC_Mesh>("rex");
            if (!mesh)
                return;
            EC_Mesh* ogre_mesh = dynamic_cast<EC_Mesh*>(mesh.get());
            if (anim->GetMeshEntity() != ogre_mesh)
                anim->SetMeshEntity(ogre_mesh);
            
            // Check if any other animations than the supposed one is running, and stop them
            const EC_AnimationController::AnimationMap& anims = anim->GetRunningAnimations();
            EC_AnimationController::AnimationMap::const_iterator i = anims.begin();
            while (i != anims.end())
            {
                if (i->first != prim->AnimationName)
                    anim->DisableAnimation(i->first); 
                ++i;
            }
                        
            anim->EnableAnimation(prim->AnimationName, true, 1.0f);
            anim->SetAnimationSpeed(prim->AnimationName, prim->AnimationRate);
        }
    }
}

void Primitive::HandleExtraParams(const entity_id_t &entity_id, const uint8_t *extra_params_data)
{
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entity_id);
    if (!entity)
        return;
    
    int idx = 0;
    uint8_t num_params = ReadUInt8FromBytes(extra_params_data, idx);
    for (uint8_t param_i = 0; param_i < num_params; ++param_i)
    {
        uint16_t param_type = ReadUInt16FromBytes(extra_params_data, idx);
        uint param_size = ReadSInt32FromBytes(extra_params_data, idx);
        switch (param_type)
        {
        case 32: // light
        {
            // If light component doesn't exist, create it.
            entity->GetOrCreateComponent(EC_OgreLight::TypeNameStatic());
                
            // Read the data.
            Color color = ReadColorFromBytes(extra_params_data, idx);
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

void Primitive::AttachLightComponent(Scene::EntityPtr entity, Color &color, float radius, float falloff)
{
    if (radius < 0.001)
        radius = 0.001f;

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

    EC_OgreLight* light = entity->GetComponent<EC_OgreLight>().get();
    if (!light)
        return;

    ///\note Only point lights are supported at the moment.
    light->SetType(EC_OgreLight::LT_Point);
    ComponentPtr placeable = entity->GetComponent(EC_Placeable::TypeNameStatic());
    if (placeable)
        light->SetPlaceable(placeable);

    color.r = clamp<float>(color.r, 0.f, 1.f);
    color.g = clamp<float>(color.g, 0.f, 1.f);
    color.b = clamp<float>(color.b, 0.f, 1.f);
    color.a = clamp<float>(color.a, 0.f, 1.f);

    light->SetColor(color);
    light->SetAttenuation(max_radius, 0.0f, linear, quad);
}

void Primitive::AttachHoveringTextComponent(Scene::EntityPtr entity, const std::string &text, const QColor &color)
{
#ifdef EC_HoveringText_ENABLED
    if (text.empty())
    {
        boost::shared_ptr<EC_HoveringText> hoveringText = entity->GetComponent<EC_HoveringText>("llSetText");
        if (!hoveringText)
            return;

        entity->RemoveComponent(hoveringText);
    }
    else
    {
        ComponentPtr component = entity->GetOrCreateComponent(EC_HoveringText::TypeNameStatic(), "llSetText");
        assert(component.get());
        EC_HoveringText &hoveringText = *(checked_static_cast<EC_HoveringText *>(component.get()));
        hoveringText.SetTextColor(color);
        hoveringText.ShowMessage(QString::fromUtf8(text.c_str()));
    }
#endif
}

bool Primitive::HandleResourceEvent(event_id_t event_id, IEventData* data)
{
    if (event_id == Resource::Events::RESOURCE_READY)
    {
        Resource::Events::ResourceReady* event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);
        Foundation::ResourcePtr res = event_data->resource_;
        asset_type_t asset_type = -1;
        
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
        else if (res->GetType() == OgreRenderer::OgreSkeletonResource::GetTypeStatic())
            asset_type = RexTypes::RexAT_Skeleton;

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
        case RexAT_Skeleton:
            HandleSkeletonReady(i->second, res);
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

void Primitive::HandleMeshReady(entity_id_t entityid, Foundation::ResourcePtr res)
{     
    if (!res) return;
    if (res->GetType() != OgreRenderer::OgreMeshResource::GetTypeStatic()) return;
    
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity) return;
    EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();

    EC_Mesh* mesh = entity->GetComponent<EC_Mesh>().get();
    if (!mesh) return;

    // Use optionally skeleton if it's used and we already have the resource
    Foundation::ResourcePtr skel_res;
    if (!RexTypes::IsNull(prim->AnimationPackageID))
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        skel_res = renderer->GetResource(prim->AnimationPackageID, OgreRenderer::OgreSkeletonResource::GetTypeStatic());
    }                
    
    if (!skel_res)
    {
        mesh->SetMesh(res->GetId());
    }
    else
    {
        //! \todo what if multiple entities use the same mesh, but different skeleton?
        mesh->SetMeshWithSkeleton(res->GetId(), skel_res->GetId());
    }

    // Update material map if mesh has more materials than the existing prim
    // This was added when setting mesh id (url based at least) left material count to 1 always
    uint mesh_material_count = mesh->GetNumMaterials();
    if (mesh_material_count > 0 && (mesh_material_count != prim->Materials.size()))
    {
        MaterialData mat_data;
        mat_data.Type = 0;
        mat_data.asset_id = RexUUID().ToString();
        for (uint material = prim->Materials.size(); material<mesh_material_count; ++material)
            prim->Materials[material] = mat_data;
    }

    // Set adjustment orientation for mesh (a legacy haxor, Ogre meshes usually have Y-axis as vertical)
    Quaternion adjust(PI/2, 0, PI);
    mesh->SetAdjustOrientation(adjust);

    HandlePrimScaleAndVisibility(entityid);
    
    // Check/set textures now that we have the mesh
    HandleMeshMaterials(entityid); 

    // Check/set animation
    HandleMeshAnimation(entityid);

    Scene::Events::EntityEventData event_data;
    event_data.entity = entity;
    Foundation::EventManagerPtr event_manager = rexlogicmodule_->GetFramework()->GetEventManager();
    event_manager->SendEvent("Scene", Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED, &event_data);
}

void Primitive::HandleSkeletonReady(entity_id_t entityid, Foundation::ResourcePtr res)
{
    if (!res) return;
    if (res->GetType() != OgreRenderer::OgreSkeletonResource::GetTypeStatic()) return;
    
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity) return;
    EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
    
    // The skeleton itself is not useful without the mesh. But if we have the mesh too, set it now
    boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
        GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();   
    Foundation::ResourcePtr mesh_res = renderer->GetResource(prim->MeshID, OgreRenderer::OgreMeshResource::GetTypeStatic());
    if (mesh_res)
        HandleMeshReady(entityid, mesh_res);
}

void Primitive::HandleParticleScriptReady(entity_id_t entityid, Foundation::ResourcePtr res)
{
    if (!res) return;
    if (res->GetType() != OgreRenderer::OgreParticleResource::GetTypeStatic()) return;
    OgreRenderer::OgreParticleResource* partres = checked_static_cast<OgreRenderer::OgreParticleResource*>(res.get());
     
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity) return;

    EC_OgreParticleSystem* particle = entity->GetComponent<EC_OgreParticleSystem>().get();
    if (!particle) return;

    particle->RemoveParticleSystems();
    for (uint i = 0; i < partres->GetNumTemplates(); ++i)
    {
        particle->AddParticleSystem(partres->GetTemplateName(i));
    }

    // Set adjustment orientation for system (legacy haxor)
    //Quaternion adjust(PI, 0, PI);
    //particle.SetAdjustOrientation(adjust);
}

void Primitive::HandleTextureReady(entity_id_t entityid, Foundation::ResourcePtr res)
{
    assert(res.get());
    if (!res)
        return;
    assert(res->GetType() == OgreRenderer::OgreTextureResource::GetTypeStatic());
    if (res->GetType() != OgreRenderer::OgreTextureResource::GetTypeStatic()) 
        return;
    
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity) return;
    EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();            
    if (prim->DrawType == RexTypes::DRAWTYPE_MESH)
    {
        EC_Mesh* meshptr = entity->GetComponent<EC_Mesh>().get();
        if (!meshptr) return;
        // If don't have the actual mesh entity yet, no use trying to set texture
        if (!meshptr->GetEntity()) return;
        
        MaterialMap::const_iterator i = prim->Materials.begin();
        while (i != prim->Materials.end())
        {
            uint idx = i->first;
            if ((i->second.Type == RexTypes::RexAT_Texture || i->second.Type == RexTypes::RexAT_TextureJPEG) && (i->second.asset_id.compare(res->GetId()) == 0))
            {
                // Use a legacy material with the same name as the texture
                OgreRenderer::GetOrCreateLegacyMaterial(res->GetId(), OgreRenderer::LEGACYMAT_NORMAL);
                meshptr->SetMaterial(idx, res->GetId());
                
                Scene::Events::EntityEventData event_data;
                event_data.entity = entity;
                Foundation::EventManagerPtr event_manager = rexlogicmodule_->GetFramework()->GetEventManager();
                event_manager->SendEvent("Scene", Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED, &event_data);
            }
            ++i;
        }
    }
}

void Primitive::HandleMaterialResourceReady(entity_id_t entityid, Foundation::ResourcePtr res)
{
    assert(res.get());
    if (!res) 
        return;
    assert(res->GetType() == OgreRenderer::OgreMaterialResource::GetTypeStatic());
    if (res->GetType() != OgreRenderer::OgreMaterialResource::GetTypeStatic()) 
        return;
    
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity) return;
    EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
    
    // Handle material ready for prim
    if (prim->DrawType == RexTypes::DRAWTYPE_PRIM)
    {
        EC_OgreCustomObject* custom = entity->GetComponent<EC_OgreCustomObject>().get();
        if (custom && prim->Materials.size() && res->GetId() == prim->Materials[0].asset_id && prim->Materials[0].Type == RexTypes::RexAT_MaterialScript)
        {
            // Update geometry now that the material exists
            if (prim->HasPrimShapeData)
            {
                Ogre::ManualObject* manual = CreatePrimGeometry(rexlogicmodule_->GetFramework(), *prim);
                custom->CommitChanges(manual);

                Scene::Events::EntityEventData event_data;
                event_data.entity = entity;
                Foundation::EventManagerPtr event_manager = rexlogicmodule_->GetFramework()->GetEventManager();
                event_manager->SendEvent("Scene", Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED, &event_data);
            }
        }
    }
    
    // Handle material ready for mesh
    if (prim->DrawType == RexTypes::DRAWTYPE_MESH)
    {
        EC_Mesh* meshptr = entity->GetComponent<EC_Mesh>().get();
        if (meshptr) 
        {
            // If don't have the actual mesh entity yet, no use trying to set the material
            if (!meshptr->GetEntity()) return;
            
            MaterialMap::const_iterator i = prim->Materials.begin();
            while (i != prim->Materials.end())
            {
                uint idx = i->first;
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
                        event_manager->SendEvent("Scene", Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED, &event_data);
                                    
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

void Primitive::DiscardRequestTags(entity_id_t entityid, Primitive::EntityResourceRequestMap& map)
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

void Primitive::HandlePrimScaleAndVisibility(entity_id_t entityid)
{
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity)
        return;
        
    EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();            
    EC_Placeable *ogrepos = entity->GetComponent<EC_Placeable>().get();
    if (!ogrepos)
        return;
            
    // Handle scale mesh to prim-setting
    EC_Mesh* mesh = entity->GetComponent<EC_Mesh>().get();
    if (mesh)
    {
        Vector3df adjust_scale(1.0, 1.0, 1.0);
        if (prim->ScaleToPrim && mesh->GetEntity())
        {
            Vector3df min, max, size;

            mesh->GetBoundingBox(min, max);
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
        const Vector3df& prim_scale = prim->Scale;
        if ((prim_scale.x != 0.0) && (prim_scale.y != 0.0) && (prim_scale.z != 0.0))
        {
            adjust_scale.y /= prim_scale.y;
            adjust_scale.z /= prim_scale.z;

            adjust_scale.y *= prim_scale.z;
            adjust_scale.z *= prim_scale.y;
        }

        mesh->SetAdjustScale(adjust_scale);
    }

    ogrepos->SetScale(prim->Scale);

    // Handle visibility
    ogrepos->GetSceneNode()->setVisible(prim->IsVisible.Get());
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
    prim.PrimRepeatU.clear();
    prim.PrimRepeatV.clear();
    prim.PrimOffsetU.clear();
    prim.PrimOffsetV.clear();
    prim.PrimUVRotation.clear();
    
    int idx = 0;
    uint32_t bits;
    int num_bits;
    
    if (idx >= length)
        return;
    
    prim.PrimDefaultTextureID = ReadUUIDFromBytes(bytes, idx).ToString();   
    while ((idx < length) && (ReadTextureEntryBits(bits, num_bits, bytes, idx)))
    {
        if (idx >= length)
            return;
        RexUUID texture_id = ReadUUIDFromBytes(bytes, idx);
        for (int i = 0; i < num_bits; ++i)
        {
            if (bits & 1)
                prim.PrimTextures[i] = texture_id.ToString();
            bits >>= 1;
        }
    }
    
    if (idx >= length)
        return;
    
    prim.PrimDefaultColor = ReadColorFromBytesInverted(bytes, idx); 
    while ((idx < length) && (ReadTextureEntryBits(bits, num_bits, bytes, idx)))
    {
        if (idx >= length)
            return;
        Color color = ReadColorFromBytesInverted(bytes, idx);
        for (int i = 0; i < num_bits; ++i)
        {
            if (bits & 1)
                prim.PrimColors[i] = color;
            bits >>= 1;
        }
    }
    
    if (idx >= length)
        return;
            
    prim.PrimDefaultRepeatU = ReadFloatFromBytes(bytes, idx);
    while ((idx < length) && (ReadTextureEntryBits(bits, num_bits, bytes, idx)))
    {
        if (idx >= length)
            return;
        float repeat = ReadFloatFromBytes(bytes, idx);
        for (int i = 0; i < num_bits; ++i)
        {
            if (bits & 1)
                prim.PrimRepeatU[i] = repeat;
            bits >>= 1;
        }
    }    
   
    if (idx >= length)
        return;
            
    prim.PrimDefaultRepeatV = ReadFloatFromBytes(bytes, idx);
    while ((idx < length) && (ReadTextureEntryBits(bits, num_bits, bytes, idx)))
    {
        if (idx >= length)
            return;
        float repeat = ReadFloatFromBytes(bytes, idx);
        for (int i = 0; i < num_bits; ++i)
        {
            if (bits & 1)
                prim.PrimRepeatV[i] = repeat;
            bits >>= 1;
        }
    }   
    
    prim.PrimDefaultOffsetU = ReadSInt16FromBytes(bytes, idx) / 32767.0f;
    while ((idx < length) && (ReadTextureEntryBits(bits, num_bits, bytes, idx)))
    {
        if (idx >= length)
            return;
        float offset = ReadSInt16FromBytes(bytes, idx) / 32767.0f;
        for (int i = 0; i < num_bits; ++i)
        {
            if (bits & 1)
                prim.PrimOffsetU[i] = offset;
            bits >>= 1;
        }
    }       
    
    prim.PrimDefaultOffsetV = ReadSInt16FromBytes(bytes, idx) / 32767.0f;
    while ((idx < length) && (ReadTextureEntryBits(bits, num_bits, bytes, idx)))
    {
        if (idx >= length)
            return;
        float offset = ReadSInt16FromBytes(bytes, idx) / 32767.0f;
        for (int i = 0; i < num_bits; ++i)
        {
            if (bits & 1)
                prim.PrimOffsetV[i] = offset;
            bits >>= 1;
        }
    }            
    
    prim.PrimDefaultUVRotation = ReadSInt16FromBytes(bytes, idx) / 32767.0f * 2 * PI;
    while ((idx < length) && (ReadTextureEntryBits(bits, num_bits, bytes, idx)))
    {
        if (idx >= length)
            return;
        float rotation = ReadSInt16FromBytes(bytes, idx) / 32767.0f * 2 * PI;
        for (int i = 0; i < num_bits; ++i)
        {
            if (bits & 1)
                prim.PrimUVRotation[i] = rotation;
            bits >>= 1;
        }
    }  
    
    prim.PrimDefaultMaterialType = bytes[idx++];    
    while ((idx < length) && (ReadTextureEntryBits(bits, num_bits, bytes, idx)))
    {
        if (idx >= length)
            return;
        uint8_t materialtype = bytes[idx++];
        for (int i = 0; i < num_bits; ++i)
        {
            if (bits & 1)
                prim.PrimMaterialTypes[i] = materialtype;
            bits >>= 1;
        }
    }
}

void Primitive::HandleAmbientSound(entity_id_t entityid)
{
    boost::shared_ptr<ISoundService> soundsystem =
        rexlogicmodule_->GetFramework()->GetServiceManager()->GetService<ISoundService>(Foundation::Service::ST_Sound).lock();
    if (!soundsystem)
        return;
        
    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity)
        return;

    EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
    RexUUID sounduuid(prim->SoundID);

    if (prim->SoundID.empty() || (RexUUID::IsValid(prim->SoundID) && sounduuid.IsNull()))
    {
        // If sound ID is empty or null, stop any previous sound
        EC_AttachedSound* attachedsound = entity->GetComponent<EC_AttachedSound>().get();
        if (attachedsound)
            attachedsound->RemoveSound(EC_AttachedSound::RexAmbientSound);
    }
    else
    {
        ComponentPtr attachedsoundptr = entity->GetOrCreateComponent(EC_AttachedSound::TypeNameStatic());
        EC_AttachedSound* attachedsound = checked_static_cast<EC_AttachedSound*>(attachedsoundptr.get());
        
        // If not already playing the same sound, start it now
        bool same = false;
        const std::vector<sound_id_t>& sounds = attachedsound->GetSounds();
        sound_id_t rex_ambient_sound = sounds[EC_AttachedSound::RexAmbientSound];
        if (rex_ambient_sound)
        {
            QString sound_name = soundsystem->GetSoundName(rex_ambient_sound);
            if (sound_name.toStdString() == prim->SoundID)
                same = true;
        }
        if (!same)
        {
            // Get initial position of sound from the placeable (will be updated later if the entity moves)
            Vector3df position(0.0f, 0.0f, 0.0f);
            EC_Placeable *placeable = entity->GetComponent<EC_Placeable>().get();
            if (placeable)
            {
                position = placeable->GetPosition();
            }        
            rex_ambient_sound = soundsystem->PlaySound3D(QString::fromStdString(prim->SoundID), ISoundService::Ambient, false, position);
            // The ambient sounds will always loop
            soundsystem->SetLooped(rex_ambient_sound, true);
            
            // Now add the sound to entity
            attachedsound->AddSound(rex_ambient_sound, EC_AttachedSound::RexAmbientSound); 
        }
        
        // Adjust the range & gain parameters
        if (rex_ambient_sound)
        {
            // Set positionality depending on the radius
            if (prim->SoundRadius > 0.0)
            {
                soundsystem->SetPositional(rex_ambient_sound, true);
                soundsystem->SetRange(rex_ambient_sound, 0.0f, prim->SoundRadius, 2.0f);
            }
            else
            {
                soundsystem->SetPositional(rex_ambient_sound, false);
            }
         
            soundsystem->SetGain(rex_ambient_sound, prim->SoundVolume);
        }
    }
}

bool Primitive::HandleOSNE_AttachedSound(ProtocolUtilities::NetworkEventInboundData* data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    std::string asset_id = msg.ReadUUID().ToString();
    RexUUID entityid = msg.ReadUUID();
    msg.ReadUUID(); // OwnerID
    float gain = msg.ReadF32();
    u8 flags = msg.ReadU8();

    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity)
        return false;

    boost::shared_ptr<ISoundService> soundsystem =
        rexlogicmodule_->GetFramework()->GetServiceManager()->GetService<ISoundService>(Foundation::Service::ST_Sound).lock();
    if (!soundsystem)
        return false;

    ComponentPtr attachedsoundptr = entity->GetOrCreateComponent(EC_AttachedSound::TypeNameStatic());
    EC_AttachedSound* attachedsound = checked_static_cast<EC_AttachedSound*>(attachedsoundptr.get());

    // Get initial position of sound from the placeable (will be updated later if the entity moves)
    Vector3df position(0.0f, 0.0f, 0.0f);
    EC_Placeable *placeable = entity->GetComponent<EC_Placeable>().get();
    if (placeable)
        position = placeable->GetPosition();

    // Start new sound
    sound_id_t new_sound = soundsystem->PlaySound3D(QString::fromStdString(asset_id), ISoundService::Triggered, false, position);
    soundsystem->SetGain(new_sound, gain);
    if (flags & RexTypes::ATTACHED_SOUND_LOOP)
        soundsystem->SetLooped(new_sound, true);

    // Attach sound that the position will be updated / sound will be stopped if entity destroyed
    // Note that because we use the OpenSimAttachedSound sound slot, any previous sound in that slot
    // will be stopped.
    attachedsound->AddSound(new_sound, EC_AttachedSound::OpenSimAttachedSound);

    return false;
}

bool Primitive::HandleOSNE_AttachedSoundGainChange(ProtocolUtilities::NetworkEventInboundData* data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    RexUUID entityid = msg.ReadUUID();
    float gain = msg.ReadF32();

    Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(entityid);
    if (!entity)
        return false;   

    boost::shared_ptr<ISoundService> soundsystem = rexlogicmodule_->GetFramework()->GetServiceManager()->GetService<ISoundService>(Foundation::Service::ST_Sound).lock();
    if (!soundsystem)
        return false;

    // Get attachedsound component
    EC_AttachedSound* attachedsound = entity->GetComponent<EC_AttachedSound>().get(); 
    if (!attachedsound)
        return false; 
    // Change gain of sound(s) this entity is playing 
    const std::vector<sound_id_t>& sounds = attachedsound->GetSounds();
    for (uint i = 0; i < sounds.size(); ++i)
        soundsystem->SetGain(sounds[i], gain);

    return false;
}

void Primitive::HandleLogout()
{
    prim_resource_request_tags_.clear();
    pending_rexprimdata_.clear();
    pending_rexfreedata_.clear();
    local_dirty_entities_.clear();
}


RexUUID Primitive::UuidForRexObjectUpdatePacket(RexTypes::RexAssetID id)
{
    if (RexUUID::IsValid(id))
        return RexUUID(id);
    else
        if (QUrl(id.c_str()).isValid())
        {
            // todo: more clever parse logic
            QString uuid_candidate = QUrl(id.c_str()).path().right(36);
            if (RexUUID::IsValid(uuid_candidate.toStdString()))
                return RexUUID(uuid_candidate.toStdString());
            else
                return RexUUID(); // zero uuid
        }
        else
            return RexUUID(); // zero uuid
}

std::string Primitive::UrlForRexObjectUpdatePacket(RexTypes::RexAssetID id)
{
    QUrl url(QString(id.c_str()));
    if (url.isValid())
        return url.toString().toStdString();
    else
        return "";
}

void Primitive::RegisterToComponentChangeSignals(Scene::ScenePtr scene)
{
    connect(scene.get(), SIGNAL( AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type) ),
        this, SLOT( OnAttributeChanged(IComponent*, IAttribute*, AttributeChange::Type) ));
    connect(scene.get(), SIGNAL( ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type) ),
        this, SLOT( OnEntityChanged(Scene::Entity*, IComponent*, AttributeChange::Type) ));
    connect(scene.get(), SIGNAL( ComponentRemoved(Scene::Entity*, IComponent*, AttributeChange::Type) ),
        this, SLOT( OnEntityChanged(Scene::Entity*, IComponent*, AttributeChange::Type) ));
}

void Primitive::OnAttributeChanged(IComponent* comp, IAttribute* attribute, AttributeChange::Type change)
{
    if ((!comp) || (!comp->IsSerializable()) || (!comp->GetNetworkSyncEnabled()))
        return;
    Scene::Entity* parent_entity = comp->GetParentEntity();
    if (!parent_entity)
        return;
    entity_id_t entityid = parent_entity->GetId();
    
    if (change == AttributeChange::Replicate)
    {
        //std::cout << "Added component " + comp->TypeName().toStdString() + " to replication list" << std::endl;
        local_dirty_entities_.insert(entityid);
    }
}

void Primitive::OnEntityChanged(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if ((!comp) || (!comp->IsSerializable()) || (!comp->GetNetworkSyncEnabled()))
        return;
    if (!entity)
        return;
    
    // Entity has had either a component added or removed. We will do full EC-data sync
    // (actually the component pointer is of no interest right now)
    entity_id_t entityid = entity->GetId();
    
    if (change == AttributeChange::Replicate)
    {
        //std::cout << "Added to replication list due to component add/remove" << std::endl;
        local_dirty_entities_.insert(entityid);
    }
}

void Primitive::OnRexPrimDataChanged(Scene::Entity* entity)
{
    SendRexPrimData(entity->GetId());
}

void Primitive::OnPrimShapeChanged(const EC_OpenSimPrim& prim)
{
    WorldStreamPtr connection = rexlogicmodule_->GetServerConnection();
    if (connection)
        connection->SendObjectShapeUpdate(prim);
}

void Primitive::OnPrimNameChanged(const EC_OpenSimPrim& prim)
{
    WorldStreamPtr connection = rexlogicmodule_->GetServerConnection();
    if (connection)
    {
        ProtocolUtilities::ObjectNameInfo name_info;
        name_info.local_id_ = prim.LocalId;
        name_info.name_ = prim.Name.Get().toStdString();

        connection->SendObjectNamePacket(name_info);
    }
}

void Primitive::OnPrimDescriptionChanged(const EC_OpenSimPrim& prim)
{
    WorldStreamPtr connection = rexlogicmodule_->GetServerConnection();
    if (connection)
    {
        ProtocolUtilities::ObjectDescriptionInfo desc_info;
        desc_info.local_id_ = prim.LocalId;
        desc_info.description_ = prim.Description.Get().toStdString();

        connection->SendObjectDescriptionPacket(desc_info);
    }
}

void Primitive::SerializeECsToNetwork()
{
    // Process the local change list for entities we have modified ourselves and have to send the EC data for
    for (EntityIdSet::iterator i = local_dirty_entities_.begin(); i != local_dirty_entities_.end(); ++i)
    {
        Scene::EntityPtr entity = rexlogicmodule_->GetPrimEntity(*i);
        if (!entity)
            continue;
        
        //std::cout << "Processing locally dirty entity" << std::endl;
        
        const Scene::Entity::ComponentVector& components = entity->GetComponentVector();
        
        // Get/create freedata component
        ComponentPtr freeptr = entity->GetOrCreateComponent(EC_FreeData::TypeNameStatic());
        if (!freeptr)
            continue;
        EC_FreeData& free = *(dynamic_cast<EC_FreeData*>(freeptr.get()));
        
        QDomDocument temp_doc;
        QDomElement entity_elem = temp_doc.createElement("entity");
        
        QString id_str;
        id_str.setNum(entity->GetId());
        entity_elem.setAttribute("id", id_str);
        
        for (uint j = 0; j < components.size(); ++j)
        {
            if ((components[j]->IsSerializable()) && (components[j]->GetNetworkSyncEnabled()))
                components[j]->SerializeTo(temp_doc, entity_elem);
        }
        
        temp_doc.appendChild(entity_elem);
        QByteArray bytes = temp_doc.toByteArray();
        
        if (bytes.size() > 1000)
        {
            RexLogicModule::LogError("Entity component serialized data is too large (>1000 bytes), not sending update");
            continue;
        }
        
        //std::cout << "Sending freedata" << std::endl;
        free.FreeData = std::string(bytes.data(), bytes.size());
        SendRexFreeData(*i);
    }
    local_dirty_entities_.clear();
}

void Primitive::DeserializeECsFromFreeData(Scene::EntityPtr entity, QDomDocument& doc)
{
    StringVector type_names;
    StringVector names;
    QDomElement entity_elem = doc.firstChildElement("entity");
    std::vector<ComponentPtr> deserialized;
    if (!entity_elem.isNull())
    {
        QDomElement comp_elem = entity_elem.firstChildElement("component");
        while (!comp_elem.isNull())
        {
            std::string type_name = comp_elem.attribute("type").toStdString();
            std::string name = comp_elem.attribute("name").toStdString();
            type_names.push_back(type_name);
            names.push_back(name);
            // Signal the add of component right away
            ComponentPtr new_comp = entity->GetOrCreateComponent(type_name.c_str(), name.c_str(), AttributeChange::LocalOnly);
            // If it's an existing component, and has network sync disabled, skip
            if ((new_comp) && (new_comp->GetNetworkSyncEnabled()))
            {
                // Deserialize in disconnected manner, signal the attribute changes later
                new_comp->DeserializeFrom(comp_elem, AttributeChange::Disconnected);
                deserialized.push_back(new_comp);
            }
            else
                RexLogicModule::LogWarning("Could not create entity component from XML data: " + type_name);
            comp_elem = comp_elem.nextSiblingElement("component");
        }
    }
    
    // If the entity has extra serializable EC's, we must remove them if they are no longer in the freedata.
    // However, at present time majority of EC's are not serializable, are handled internally, and must not be removed
    Scene::Entity::ComponentVector all_components = entity->GetComponentVector();
    for (uint i = 0; i < all_components.size(); ++i)
    {
        if ((all_components[i]->IsSerializable()) && (all_components[i]->GetNetworkSyncEnabled()))
        {
            bool found = false;
            for (uint j = 0; j < type_names.size(); ++j)
            {
                if (type_names[j] == all_components[i]->TypeName().toStdString() && names[j] == all_components[i]->Name().toStdString())
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                entity->RemoveComponent(all_components[i], AttributeChange::LocalOnly);
        }
    }
    
    // Finally trigger localonly change for the components we deserialized.
    //! \todo All attributes will reflect a change, even if they had the same value as before. Optimize this away.
    //! \todo ComponentChanged() currently triggers deprecated OnChanged() signal. This will be removed. Do not rely on it!
    for (uint i = 0; i < deserialized.size(); ++i)
        deserialized[i]->ComponentChanged(AttributeChange::LocalOnly);
}

} // namespace RexLogic
