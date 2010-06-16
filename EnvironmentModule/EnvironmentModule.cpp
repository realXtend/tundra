/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *  @file   EnvironmentModule.cpp
 *  @brief  Environment module. Environment module is be responsible of visual environment features like terrain, sky & water.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EnvironmentModule.h"
#include "Terrain.h"
#include "Water.h"
#include "Environment.h"
#include "Sky.h"
#include "EnvironmentEditor.h"
#include "EC_Water.h"
#include "PostProcessWidget.h"

#include "Renderer.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "OgreTextureResource.h"
#include "SceneManager.h"
#include "NetworkEvents.h"
#include "InputEvents.h"
#include "GenericMessageUtils.h"
#include "ModuleManager.h"
#include "EventManager.h"
#include "RexNetworkUtils.h"

#include "MemoryLeakCheck.h"

namespace Environment
{
    std::string EnvironmentModule::type_name_static_ = "Environment";

    EnvironmentModule::EnvironmentModule() :
        ModuleInterfaceImpl(type_name_static_),
        waiting_for_regioninfomessage_(false),
        environment_editor_(0),
        postprocess_dialog_(0),
        resource_event_category_(0),
        scene_event_category_(0),
        framework_event_category_(0),
        input_event_category_(0)
    {
    }

    EnvironmentModule::~EnvironmentModule()
    {
    }

    void EnvironmentModule::Load()
    {
        DECLARE_MODULE_EC(EC_Terrain);
        DECLARE_MODULE_EC(EC_Water);
    }

    void EnvironmentModule::Initialize()
    {
        //initialize postprocess dialog
        boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (renderer)
        {
            postprocess_dialog_ = new PostProcessWidget(renderer->GetCompositionHandler().GetAvailableCompositors());
            postprocess_dialog_->SetHandler(&renderer->GetCompositionHandler());
            postprocess_dialog_->AddSelfToScene(this);
        }
    }

    void EnvironmentModule::PostInitialize()
    {
        event_manager_ = framework_->GetEventManager();
        
        // Depends on rexlogic etc. handling messages first to create the scene, so lower priority
        event_manager_->RegisterEventSubscriber(framework_->GetModuleManager()->GetModule(this), 99);

        resource_event_category_ = event_manager_->QueryEventCategory("Resource");
        scene_event_category_ = event_manager_->QueryEventCategory("Scene");
        framework_event_category_ = event_manager_->QueryEventCategory("Framework");
        input_event_category_ = event_manager_->QueryEventCategory("Input");
    }

    void EnvironmentModule::SubscribeToNetworkEvents()
    {
        network_in_event_category_ = event_manager_->QueryEventCategory("NetworkIn");
        network_state_event_category_ = event_manager_->QueryEventCategory("NetworkState");
    }

    void EnvironmentModule::Uninitialize()
    {
        SAFE_DELETE(environment_editor_);
        SAFE_DELETE(postprocess_dialog_);
        terrain_.reset();
        water_.reset();
        environment_.reset();
        sky_.reset();
        event_manager_.reset();
        currentWorldStream_.reset();

        waiting_for_regioninfomessage_ = false;
    }

    void EnvironmentModule::Update(f64 frametime)
    {
        RESETPROFILER;
        // HACK Initialize editor_widget_ in correct time. 
        if (environment_editor_ == 0 && terrain_.get() != 0 && water_.get() != 0)
            environment_editor_ = new EnvironmentEditor(this);

        if ((currentWorldStream_) && currentWorldStream_->IsConnected())
        {
            if (environment_.get())
                environment_->Update(frametime);
        }
    }

    bool EnvironmentModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if(category_id == framework_event_category_)
        {
            HandleFrameworkEvent(event_id, data);
        }
        else if(category_id == resource_event_category_)
        {
            HandleResouceEvent(event_id, data);
        }
        else if(category_id == network_in_event_category_)
        {
            HandleNetworkEvent(event_id, data);
        }
        else if (category_id == network_state_event_category_)
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

            if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
            {
                if(postprocess_dialog_)
                    postprocess_dialog_->DisableAllEffects();
                ReleaseTerrain();
                ReleaseWater();
                ReleaseEnvironment();
                ReleaseSky();
            }
        }
        else if(category_id == input_event_category_)
        {
            HandleInputEvent(event_id, data);
        }
        return false;
    }

    bool EnvironmentModule::HandleResouceEvent(event_id_t event_id, Foundation::EventDataInterface* data)
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
                if (terrain_.get())
                    terrain_->OnTextureReadyEvent(res);

                // Pass the texture asset to the sky manager - the texture might be in the sky.
                if (sky_.get())
                    sky_->OnTextureReadyEvent(res);
            }
            Foundation::TextureInterface *decoded_tex = decoded_tex = dynamic_cast<Foundation::TextureInterface *>(res->resource_.get());
            if (decoded_tex)
                // Pass the texture asset to environment editor.
                if (environment_editor_)
                    environment_editor_->HandleResourceReady(res);
        }

        return false;
    }

    bool EnvironmentModule::HandleFrameworkEvent(event_id_t event_id, Foundation::EventDataInterface* data)
    {
        switch(event_id)
        {
            case Foundation::NETWORKING_REGISTERED:
            {
                // Begin to listen network events.
                SubscribeToNetworkEvents();
                return false;
            }
            case Foundation::WORLD_STREAM_READY:
            {
                ProtocolUtilities::WorldStreamReadyEvent *event_data = dynamic_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
                if (event_data)
                    currentWorldStream_ = event_data->WorldStream;

                return false;
            }
        }

        return false;
    }

    bool EnvironmentModule::HandleNetworkEvent(event_id_t event_id, Foundation::EventDataInterface* data)
    {
        ProtocolUtilities::NetworkEventInboundData *netdata = checked_static_cast<ProtocolUtilities::NetworkEventInboundData *>(data);
        assert(netdata);

        switch(event_id)
        {
        case RexNetMsgLayerData:
        {
            if(terrain_.get())
            {
                static int count = 0;
                bool kill_event = terrain_->HandleOSNE_LayerData(netdata);
                if (environment_editor_)
                    environment_editor_->UpdateTerrain();
                return kill_event;
            }
        }
        case RexNetMsgGenericMessage:
        {
            ProtocolUtilities::NetInMessage &msg = *netdata->message;
            std::string methodname = ProtocolUtilities::ParseGenericMessageMethod(msg);

            if (methodname == "RexPostP")
            {
                boost::shared_ptr<OgreRenderer::Renderer> renderer =
                    framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
                if (renderer)
                {
                    StringVector vec = ProtocolUtilities::ParseGenericMessageParameters(msg);
                    //Since postprocessing effect was enabled/disabled elsewhere, we have to notify the dialog about the event.
                    //Also, no need to put effect on from the CompositionHandler since the dialog will notify CompositionHandler when 
                    //button is checked
                    if (postprocess_dialog_)
                    {
                        QString effect_name = renderer->GetCompositionHandler().MapNumberToEffectName(vec.at(0)).c_str();
                        bool enabled = true;
                        if (vec.at(1) == "False")
                            enabled = false;

                        postprocess_dialog_->EnableEffect(effect_name,enabled);
                    }
                }
            }
            else if(methodname == "RexSky" && sky_.get())
            {
                return GetSkyHandler()->HandleRexGM_RexSky(netdata);
            }
            else if (methodname == "RexWaterHeight")
            {
                msg.ResetReading();
                msg.SkipToFirstVariableByName("Parameter");

                // Variable block begins, should have currently (at least) 1 instances.
                size_t instance_count = msg.ReadCurrentBlockInstanceCount();
                if (instance_count < 1)
                    return false;

                if (water_.get() != 0)
                {
                    std::string message = msg.ReadString();
                    // Convert to float.
                    try
                    {
                        float height = boost::lexical_cast<float>(message);
                        water_->SetWaterHeight(height);
                    }
                    catch(boost::bad_lexical_cast&)
                    {
                    }
                }
            }
            else if (methodname == "RexDrawWater")
            {
                msg.ResetReading();
                msg.SkipToFirstVariableByName("Parameter");

                // Variable block begins, should have currently (at least) 1 instances.
                size_t instance_count = msg.ReadCurrentBlockInstanceCount();
                if (instance_count < 1 )
                    return false;

                std::string message = msg.ReadString();
                bool draw = ParseBool(message);
                if (draw)
                    if (water_.get())
                        water_->CreateWaterGeometry();
                    else
                        CreateWater();
                else
                    water_->RemoveWaterGeometry();
            }
            else if (methodname == "RexFog")
            {
                /**
                 * Currently we interprent that this message information is for water fog ! Not for ground fog.
                 * @todo Someone needs to add more parameters to this package so that we can make ground fog also,
                 */

                StringVector parameters = ProtocolUtilities::ParseGenericMessageParameters(msg); 
                if ( parameters.size() < 5)
                    return false;

                // may have , instead of . so replace
                ReplaceCharInplace(parameters[0], ',', '.');
                ReplaceCharInplace(parameters[1], ',', '.');
                ReplaceCharInplace(parameters[2], ',', '.');
                ReplaceCharInplace(parameters[3], ',', '.');
                ReplaceCharInplace(parameters[4], ',', '.');
                float fogStart = 0.0, fogEnd = 0.0, fogC_r = 0.0, fogC_g = 0.0, fogC_b = 0.0;

                try
                {
                    fogStart = boost::lexical_cast<float>(parameters[0]);
                    fogEnd = boost::lexical_cast<float>(parameters[1]);
                    fogC_r = boost::lexical_cast<float>(parameters[2]);
                    fogC_g = boost::lexical_cast<float>(parameters[3]);
                    fogC_b = boost::lexical_cast<float>(parameters[4]);
                }
                catch(boost::bad_lexical_cast&)
                {
                    return false;
                }
                if (environment_ != 0)
                {
                    // Adjust fog.
                    QVector<float> color;
                    color<<fogC_r<<fogC_g<<fogC_b;
                    environment_->SetWaterFog(fogStart, fogEnd, color); 
                }
            }
            else if (methodname == "RexAmbientL")
            {
                /**
                 * Deals RexAmbientLight message. 
                 **/
                
                StringVector parameters = ProtocolUtilities::ParseGenericMessageParameters(msg); 
                if ( parameters.size() < 3)
                    return false; 

                // may have , instead of . so replace
                ReplaceCharInplace(parameters[0], ',', '.');
                ReplaceCharInplace(parameters[1], ',', '.');
                ReplaceCharInplace(parameters[2], ',', '.');

                const QChar empty(' ');
                StringVector sun_light_direction = SplitString(parameters[0].c_str(), empty.toAscii() );
                StringVector sun_light_color = SplitString(parameters[1].c_str(), empty.toAscii());
                StringVector ambient_light_color = SplitString(parameters[2].c_str(), empty.toAscii());

                if ( environment_ != 0 )
                {
                      environment_->SetSunDirection(environment_->ConvertToQVector<float>(sun_light_direction));
                      environment_->SetSunColor(environment_->ConvertToQVector<float>(sun_light_color));
                      environment_->SetAmbientLight(environment_->ConvertToQVector<float>(ambient_light_color));
                 }
            }
        }
        case RexNetMsgSimulatorViewerTimeMessage:
        {
            if (environment_!= 0)
                return environment_->HandleSimulatorViewerTimeMessage(netdata);
            break;
        }
        case RexNetMsgRegionHandshake:
        {
            bool kill_event = HandleOSNE_RegionHandshake(netdata);
            if (environment_editor_)
                environment_editor_->UpdateTerrainTextureRanges();
            return kill_event;
        }
        case RexNetMsgRegionInfo:
        {
            if (waiting_for_regioninfomessage_)
            {
                currentWorldStream_->SendTextureCommitMessage();
                waiting_for_regioninfomessage_ = false;
            }
        }
        }

        return false;
    }

    bool EnvironmentModule::HandleInputEvent(event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if(event_id == Input::Events::MOUSEDRAG || event_id == Input::Events::MOUSEDRAG_STOPPED)
            if(environment_editor_ && environment_editor_->GetTerrainPaintMode() == EnvironmentEditor::Paint3D)
                environment_editor_->HandleMouseDragEvent(event_id, data);
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
        Real water_height = msg.ReadF32();
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

        Real TerrainStartHeights[4];
        TerrainStartHeights[0] = msg.ReadF32();
        TerrainStartHeights[1] = msg.ReadF32();
        TerrainStartHeights[2] = msg.ReadF32();
        TerrainStartHeights[3] = msg.ReadF32();

        Real TerrainStartRanges[4];
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

    TerrainPtr EnvironmentModule::GetTerrainHandler() const
    {
        return terrain_;
    }

    EnvironmentPtr EnvironmentModule::GetEnvironmentHandler() const
    {
        return environment_;
    }

    SkyPtr EnvironmentModule::GetSkyHandler() const
    {
        return sky_;
    }

    WaterPtr EnvironmentModule::GetWaterHandler() const
    {
        return water_;
    }

    void EnvironmentModule::SendModifyLandMessage(f32 x, f32 y, u8 brush, u8 action, Real seconds, Real height)
    {
        if (currentWorldStream_.get())
            currentWorldStream_->SendModifyLandPacket(x, y, brush, action, seconds, height);
    }

    void EnvironmentModule::SendTextureHeightMessage(Real start_height, Real height_range, uint corner)
    {
        if (currentWorldStream_.get())
        {
            currentWorldStream_->SendTextureHeightsMessage(start_height, height_range, corner);
            waiting_for_regioninfomessage_ = true;
        }
    }

    void EnvironmentModule::SendTextureDetailMessage(const RexTypes::RexAssetID &new_texture_id, uint texture_index)
    {
        if (currentWorldStream_.get())
        {
            currentWorldStream_->SendTextureDetail(new_texture_id, texture_index);
            waiting_for_regioninfomessage_ = true;
        }
    }

    void EnvironmentModule::CreateTerrain()
    {
        terrain_ = TerrainPtr(new Terrain(this));

        Scene::EntityPtr entity = GetFramework()->GetDefaultWorldScene()->CreateEntity(GetFramework()->GetDefaultWorldScene()->GetNextFreeId());
        entity->AddComponent(GetFramework()->GetComponentManager()->CreateComponent("EC_Terrain"));

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
        sky_entity->AddComponent(GetFramework()->GetComponentManager()->CreateComponent("EC_OgreSky"));

        sky_->FindCurrentlyActiveSky();

        if (!GetEnvironmentHandler()->IsCaelum())
            sky_->CreateDefaultSky();
    }

    void EnvironmentModule::ReleaseTerrain()
    {
        terrain_.reset();
        waiting_for_regioninfomessage_ = false;
    }

    void EnvironmentModule::ReleaseWater()
    {
        water_.reset();
    }

    void EnvironmentModule::ReleaseEnvironment()
    {
        environment_.reset();
    }

    void EnvironmentModule::ReleaseSky()
    {
        sky_.reset();
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
