// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   EnvironmentModule.cpp
 *  @brief  Environment module. Environment module will be responsible of enviroment based operations like generating terrain from heightmap,
 *          loading sky from a cube and etc.
 */

#include "StableHeaders.h"
#include "EnvironmentModule.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "OgreTextureResource.h"
#include "SceneManager.h"
#include "SceneEvents.h"
#include "NetworkEvents.h"

#include "Terrain.h"
#include "Water.h"
#include "Environment.h"
#include "Sky.h"
#include "EnvironmentEditor.h"
#include "EC_Water.h"
#include <GenericMessageUtils.h>
#include <QVector>

namespace Environment
{
    EnvironmentModule::EnvironmentModule() : 
        ModuleInterfaceImpl(Foundation::Module::MT_Environment),
        waiting_for_reqioninfomessage_(false)
    {

    }

    EnvironmentModule::~EnvironmentModule()
    {

    }

    void EnvironmentModule::Load()
    {
        DECLARE_MODULE_EC(EC_Terrain);
        DECLARE_MODULE_EC(EC_Water);

        LogInfo("System " + Name() + " loaded.");
    }

    void EnvironmentModule::Unload()
    {
        LogInfo("System " + Name() + " unloaded.");
    }

    void EnvironmentModule::PreInitialize()
    {
        
    }

    void EnvironmentModule::Initialize()
    {
        //environment_editor_ = EnvironmentEditorPtr(new EnvironmentEditor(this));
    }

    void EnvironmentModule::PostInitialize()
    {
        event_manager_ = framework_->GetEventManager();

        resource_event_category_ = event_manager_->QueryEventCategory("Resource");
        if (resource_event_category_ == 0)
            LogError("Failed to query \"Resource\" event category");

        scene_event_category_ = event_manager_->QueryEventCategory("Scene");
        if (scene_event_category_ == 0)
            LogError("Failed to query \"Scene\" event category");

        framework_evet_category_ = event_manager_->QueryEventCategory("Framework");
        if (scene_event_category_ == 0)
            LogError("Failed to query \"Framework\" event category");
    }

    void EnvironmentModule::SubscribeToNetworkEvents()
    {
        network_in_event_category_ = event_manager_->QueryEventCategory("NetworkIn");
        if (network_in_event_category_ == 0)
            LogError("Failed to query \"NetworkIn\" event category");

        network_state_event_category_ = event_manager_->QueryEventCategory("NetworkState");
        if (network_state_event_category_ == 0)
            LogError("Failed to query \"NetworkState\" event category");
    }

    void EnvironmentModule::Uninitialize()
    {
        terrain_.reset();
        water_.reset();
        environment_.reset();
        sky_.reset();
        environment_editor_.reset();

        waiting_for_reqioninfomessage_ = false;

        LogInfo("System " + Name() + " uninitialized.");
    }

    void EnvironmentModule::Update(Core::f64 frametime)
    {
        // HACK Initialize editor_widget_ in correct time. 

        if ( environment_editor_.get() == 0 && terrain_.get() != 0 && water_.get() != 0)
            environment_editor_ = EnvironmentEditorPtr(new EnvironmentEditor(this));

        if((currentWorldStream_) && currentWorldStream_->IsConnected())
        {
            if(environment_.get())
                environment_->Update(frametime);
        }

    }

    bool EnvironmentModule::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if(category_id == framework_evet_category_)
        {
            if(event_id == Foundation::NETWORKING_REGISTERED)
            {
                // Begin to listen network events.
                SubscribeToNetworkEvents();
                return false;
            }

            if(event_id == Foundation::WORLD_STREAM_READY)
            {
                Foundation::WorldStreamReadyEvent *event_data = dynamic_cast<Foundation::WorldStreamReadyEvent *>(data);
                if (event_data)
                    currentWorldStream_ = event_data->WorldStream;

                return false;
            }
        }
        else if(category_id == resource_event_category_)
        {
            if (event_id == Resource::Events::RESOURCE_READY)
            {
                Resource::Events::ResourceReady *res = dynamic_cast<Resource::Events::ResourceReady*>(data);
                assert(res);
                if (!res)
                    return false;

                OgreRenderer::OgreTextureResource *tex = dynamic_cast<OgreRenderer::OgreTextureResource *>(res->resource_.get()); 
                if (tex)
                {
                    // Pass the texture asset to the terrain manager - the texture might be in the terrain.
                    if(terrain_.get())
                        terrain_->OnTextureReadyEvent(res);
                    
                    // Pass the texture asset to the sky manager - the texture might be in the sky.
                    if(sky_.get())
                        sky_->OnTextureReadyEvent(res);
                }
                Foundation::TextureInterface *decoded_tex = decoded_tex = dynamic_cast<Foundation::TextureInterface *>(res->resource_.get());
                if (decoded_tex)
                {
                    // Pass the texture asset to environment editor.
                    if(environment_editor_.get())
                        environment_editor_->HandleResourceReady(res);
                }
            }
        }
        else if(category_id == network_in_event_category_) // For NetworkIn events
        {
            ProtocolUtilities::NetworkEventInboundData *netdata = dynamic_cast<ProtocolUtilities::NetworkEventInboundData *>(data);
            if (event_id == RexNetMsgLayerData)
            {
                if(terrain_.get())
                {
                    bool kill_event = terrain_->HandleOSNE_LayerData(netdata);
                    if(environment_editor_.get())
                        environment_editor_->UpdateTerrain();
                    return kill_event;
                }
            }
            else if (event_id == RexNetMsgGenericMessage)
            {
                    ProtocolUtilities::NetInMessage &msg = *netdata->message;
                    std::string methodname = ProtocolUtilities::ParseGenericMessageMethod(msg);
                    
                    if( methodname == "RexSky" && sky_.get())
                        return GetSkyHandler()->HandleRexGM_RexSky(netdata);
                    else if ( methodname == "RexWaterHeight")
                    {
                        msg.ResetReading();
                        msg.SkipToFirstVariableByName("Parameter");
                        
                        // Variable block begins, should have currently (at least) 1 instances.
                        size_t instance_count = msg.ReadCurrentBlockInstanceCount();
                        if ( instance_count < 1 )
                            return false;


                        if ( water_.get() != 0 )
                        {
                            std::string message = msg.ReadString();
                            // Convert to float. 
                            try
                            {
                                float height = boost::lexical_cast<float>(message);
                                water_->SetWaterHeight(height);

                            } catch (boost::bad_lexical_cast&)
                            {}
                        }
                            
                    }
                    else if ( methodname == "RexDrawWater")
                    {
                        msg.ResetReading();
                        msg.SkipToFirstVariableByName("Parameter");
                        
                        // Variable block begins, should have currently (at least) 1 instances.
                        size_t instance_count = msg.ReadCurrentBlockInstanceCount();
                        if ( instance_count < 1 )
                            return false;

                        std::string message = msg.ReadString();
                        
                        // Convert to boolean
                        try
                        {
                            bool draw = boost::lexical_cast<bool>(message);
                            if ( draw ) 
                            {
                                if ( water_.get() ) 
                                    water_->CreateWaterGeometry();
                                else
                                    CreateWater();
                               
                            }
                            else
                            {
                                water_->RemoveWaterGeometry();
                            }
                               
                        } catch (boost::bad_lexical_cast&)
                        {}
                    }
                    else if ( methodname == "RexFog" )
                    {
                        Core::StringVector parameters = ProtocolUtilities::ParseGenericMessageParameters(msg); 
                        if ( parameters.size() < 5)
                            return false;

                        float fogStart = 0.0, fogEnd = 0.0, fogC_r = 0.0, fogC_g = 0.0, fogC_b = 0.0;

                        try
                        {
                            fogStart = boost::lexical_cast<float>(parameters[0]);
                            fogEnd = boost::lexical_cast<float>(parameters[1]);
                            fogC_r = boost::lexical_cast<float>(parameters[2]);
                            fogC_g = boost::lexical_cast<float>(parameters[3]);
                            fogC_b = boost::lexical_cast<float>(parameters[4]);

                        } catch ( boost::bad_lexical_cast&)
                        {
                            return false;
                        }
                        if ( environment_ != 0)
                        {
                            // Adjust fog.
                            QVector<float> color;
                            color<<fogC_r<<fogC_g<<fogC_b;
                            environment_->SetFog(fogStart, fogEnd, color); 
                        }
                    }
              
            }
            else if (event_id == RexNetMsgSimulatorViewerTimeMessage)
            {
                if (environment_!= 0)
                    return environment_->DecodeSimulatorViewerTimeMessage(netdata);
                                
            }
            else if (event_id == RexNetMsgRegionHandshake)
            {
                bool kill_event = HandleOSNE_RegionHandshake(netdata);
                if (environment_editor_.get())
                    environment_editor_->UpdateTerrainTextureRanges();
                return kill_event;
            }
            else if(event_id == RexNetMsgRegionInfo)
            {
                if(waiting_for_reqioninfomessage_)
                {
                    currentWorldStream_->SendTextureCommitMessage();
                    waiting_for_reqioninfomessage_ = false;
                }
            }
        }
        else if (category_id == network_state_event_category_) // For NetworkState category
        {
            if (event_id == ProtocolUtilities::Events::EVENT_SERVER_CONNECTED)
            {
                if (GetFramework()->GetDefaultWorldScene().get())
                {
                    CreateTerrain();
                    CreateWater();
                    CreateEnvironment();
                    CreateSky();
                }
            }
        }
        return false;
    }

    bool EnvironmentModule::HandleOSNE_RegionHandshake(ProtocolUtilities::NetworkEventInboundData* data)
    {
        ProtocolUtilities::NetInMessage &msg = *data->message;
        msg.ResetReading();

        msg.SkipToNextVariable(); // RegionFlags U32
        msg.SkipToNextVariable(); // SimAccess U8
        msg.SkipToNextVariable(); // SimName
        msg.SkipToNextVariable(); // SimOwner
        msg.SkipToNextVariable(); // IsEstateManager

        // Water height.
        Core::Real water_height = msg.ReadF32();
        if(water_.get())
            water_->SetWaterHeight(water_height);

        msg.SkipToNextVariable(); // BillableFactor
        msg.SkipToNextVariable(); // CacheID
        for(int i = 0; i < 4; ++i)
            msg.SkipToNextVariable(); // TerrainBase0..3

        // Terrain texture id
        RexAssetID terrain[4];
        terrain[0] = msg.ReadUUID().ToString();
        terrain[1] = msg.ReadUUID().ToString();
        terrain[2] = msg.ReadUUID().ToString();
        terrain[3] = msg.ReadUUID().ToString();

        Core::Real TerrainStartHeights[4];
        TerrainStartHeights[0] = msg.ReadF32();
        TerrainStartHeights[1] = msg.ReadF32();
        TerrainStartHeights[2] = msg.ReadF32();
        TerrainStartHeights[3] = msg.ReadF32();

        Core::Real TerrainStartRanges[4];
        TerrainStartRanges[0] = msg.ReadF32();
        TerrainStartRanges[1] = msg.ReadF32();
        TerrainStartRanges[2] = msg.ReadF32();
        TerrainStartRanges[3] = msg.ReadF32();

        if(terrain_.get())
        {
            terrain_->SetTerrainTextures(terrain);
            terrain_->SetTerrainHeightValues(TerrainStartHeights, TerrainStartRanges);
        }

        return false;
    }

    bool EnvironmentModule::HandleNetworkEvent(Foundation::EventDataInterface* data)
    {
        return false;
    }

    bool EnvironmentModule::HandleNetworkStateEvent(Foundation::EventDataInterface* data)
    {
        return false;
    }

    TerrainPtr EnvironmentModule::GetTerrainHandler()
    {
        return terrain_;
    }

    EnvironmentEditorPtr EnvironmentModule::GetEnvironmentEditor()
    {
        return environment_editor_;
    }

    EnvironmentPtr EnvironmentModule::GetEnvironmentHandler()
    {
        return environment_;
    }

    SkyPtr EnvironmentModule::GetSkyHandler()
    {
        return sky_;
    }

    WaterPtr EnvironmentModule::GetWaterHandler()
    {
        return water_;
    }

    void EnvironmentModule::SendModifyLandMessage(Core::f32 x, Core::f32 y, Core::u8 brush, Core::u8 action, Core::Real seconds, Core::Real height)
    {
        if(currentWorldStream_.get())
            currentWorldStream_->SendModifyLandPacket(x, y, brush, action, seconds, height);
    }

    void EnvironmentModule::SendTextureHeightMessage(Core::Real start_height, Core::Real height_range, Core::uint corner)
    {
        if(currentWorldStream_.get())
        {
            currentWorldStream_->SendTextureHeightsMessage(start_height, height_range, corner);
            waiting_for_reqioninfomessage_ = true;
        }
    }

    void EnvironmentModule::SendTextureDetailMessage(const RexTypes::RexAssetID &new_texture_id, Core::uint texture_index)
    {
        if(currentWorldStream_.get())
        {
            currentWorldStream_->SendTextureDetail(new_texture_id, texture_index);
            waiting_for_reqioninfomessage_ = true;
        }
    }

    void EnvironmentModule::CreateTerrain()
    {
        terrain_ = TerrainPtr(new Terrain(this));

        Scene::EntityPtr entity = GetFramework()->GetDefaultWorldScene()->CreateEntity(GetFramework()->GetDefaultWorldScene()->GetNextFreeId());
        entity->AddEntityComponent(GetFramework()->GetComponentManager()->CreateComponent("EC_Terrain"));

        terrain_->FindCurrentlyActiveTerrain();
    }

    void EnvironmentModule::CreateWater()
    {
        water_ = WaterPtr(new Water(this));
        water_->CreateWaterGeometry();
    }

    void EnvironmentModule::CreateEnvironment()
    {
        environment_ = EnvironmentPtr(new Environment(this));
        environment_->CreateEnvironment();
    }

    void EnvironmentModule::CreateSky()
    {
        sky_ = SkyPtr(new Sky(this));
        Scene::EntityPtr sky_entity = GetFramework()->GetDefaultWorldScene()->CreateEntity(GetFramework()->GetDefaultWorldScene()->GetNextFreeId());
        sky_entity->AddEntityComponent(GetFramework()->GetComponentManager()->CreateComponent("EC_OgreSky"));

        sky_->FindCurrentlyActiveSky();

        if (!GetEnvironmentHandler()->IsCaelum())
            sky_->CreateDefaultSky();
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace Environment;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(EnvironmentModule)
POCO_END_MANIFEST
