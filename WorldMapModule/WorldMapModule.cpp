/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   WorldMapModule.cpp
 *  @brief  Simple OpenSim world map module. Listens to region handshake and map block replies. 
 *          Draws current region map and avatar positions to widget.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "WorldMapModule.h"
#include "WorldMapWidget.h"

#include "WorldStream.h"
#include "EventManager.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "NetworkMessages/NetInMessage.h"
#include "TextureServiceInterface.h"
#include "TextureInterface.h"
#include "AssetEvents.h"
#include "WorldLogicInterface.h"
#include "ConsoleCommandServiceInterface.h"
#include "UiSettingsServiceInterface.h"

#include "EC_Placeable.h"
#include "EC_OpenSimPresence.h"
#include "SceneManager.h"

#include <Inworld/ControlPanel/TeleportWidget.h>

#include "MemoryLeakCheck.h"

namespace WorldMap
{

    WorldMapModule::WorldMapModule() :
        IModule(NameStatic()), 
        time_from_last_update_ms_(0),
        networkStateEventCategory_(0),
        networkInEventCategory_(0),
        frameworkEventCategory_(0),
        resource_event_category_(0),
        worldmap_widget_(0)
    {
    }

    WorldMapModule::~WorldMapModule()
    {    
    }

    void WorldMapModule::Load()
    {
    }

    void WorldMapModule::PostInitialize()
    {
        frameworkEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Framework");
        resource_event_category_ = framework_->GetEventManager()->QueryEventCategory("Resource");
        
        Foundation::UiSettingsServiceInterface *ui_settings_service = framework_->GetService<Foundation::UiSettingsServiceInterface>();
        if (ui_settings_service)
        {
            CoreUi::TeleportWidget *teleport_widget = dynamic_cast<CoreUi::TeleportWidget*>(ui_settings_service->GetTeleportWidget());
            if (teleport_widget)
            {
                worldmap_widget_ = new WorldMapWidget();    
                teleport_widget->InsertMapWidget(worldmap_widget_);
            }
        }
    }

    void WorldMapModule::Update(f64 frametime)
    {
        time_from_last_update_ms_ += frametime;
        if (time_from_last_update_ms_ < 0.5)
            return;
        time_from_last_update_ms_ = 0;

        UpdateAvatarPositions();
    }

    bool WorldMapModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData *data)
    {
        if (category_id == frameworkEventCategory_)
        {
            if (event_id == Foundation::NETWORKING_REGISTERED)
            {
                ProtocolUtilities::NetworkingRegisteredEvent *event_data = checked_static_cast<ProtocolUtilities::NetworkingRegisteredEvent *>(data);
                if (event_data)
                {
                    networkStateEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
                    networkInEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");
                    return false;
                }
            }

            if(event_id == Foundation::WORLD_STREAM_READY)
            {
                ProtocolUtilities::WorldStreamReadyEvent *event_data = checked_static_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
                if (event_data)
                    currentWorldStream_ = event_data->WorldStream;

                if (currentWorldStream_)
                    currentWorldStream_->SendMapBlockRequest();

                networkInEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");

                return false;
            }
        }

        if (category_id == networkStateEventCategory_)
        {
            if (event_id == ProtocolUtilities::Events::EVENT_SERVER_CONNECTED)
            {
                
            }
            else if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
            {
                if (worldmap_widget_)
                    worldmap_widget_->ClearAllContent();
            }
            else if (event_id == ProtocolUtilities::Events::EVENT_USER_DISCONNECTED)
            {
                ProtocolUtilities::UserConnectivityEvent *event_data = checked_static_cast<ProtocolUtilities::UserConnectivityEvent *>(data);
                assert(event_data);
                if (event_data && worldmap_widget_)
                    worldmap_widget_->RemoveAvatar(event_data->agentId.ToQString());
            }
        }

        if (category_id == resource_event_category_)
        {
            HandleResourceEvent(event_id, data);
        }

        if (category_id == networkInEventCategory_)
        {
            using namespace ProtocolUtilities;
            NetworkEventInboundData *netdata = checked_static_cast<NetworkEventInboundData *>(data);
            assert(netdata);
            if (!netdata)
                return false;

            switch(event_id)
            {
                case RexNetMsgMapBlockReply:
                    return HandleOSNE_MapBlock(netdata);
                case RexNetMsgRegionHandshake:
                    return HandleOSNE_RegionHandshake(netdata);
                default:
                    break;
            }
        }

        return false;
    }

    bool WorldMapModule::HandleResourceEvent(event_id_t event_id, IEventData* data)
    {

        if (event_id != Resource::Events::RESOURCE_READY)
            return false;
        
        Resource::Events::ResourceReady *res = dynamic_cast<Resource::Events::ResourceReady*>(data);
        assert(res);
        if (!res)
            return false;                

        for(uint index = 0; index < map_asset_requests_.size(); index++)
        {
            if(map_asset_requests_.at(index) == res->tag_)
            {                
                Foundation::TextureInterface *tex = dynamic_cast<Foundation::TextureInterface *>(res->resource_.get());
                if(!tex && tex->GetLevel() != 0)
                    return false;

                QImage img = ConvertToQImage(*tex);
                if(worldmap_widget_)
                    worldmap_widget_->StoreMapData(QPixmap::fromImage(img), QString(res->id_.c_str()));

            }
        }

        return false;
    }

    bool WorldMapModule::HandleOSNE_MapBlock (ProtocolUtilities::NetworkEventInboundData *data)
    {
        ProtocolUtilities::NetInMessage &msg = *data->message;
        msg.ResetReading();

        RexUUID agent_id = msg.ReadUUID();
        uint32_t flags_ = msg.ReadU32();
        size_t instance_count = msg.ReadCurrentBlockInstanceCount();

        QList<ProtocolUtilities::MapBlock> mapBlocks;
        for (size_t i = 0; i < instance_count; ++i)
        {
            ProtocolUtilities::MapBlock block;
            block.agentID = agent_id;
            block.flags = flags_;
            block.regionX = msg.ReadU16();
            block.regionY = msg.ReadU16();
            block.regionName = msg.ReadString();
            block.access = msg.ReadU8();
            block.regionFlags = msg.ReadU32();
            block.waterHeight = msg.ReadU8();
            block.agents = msg.ReadU8();
            block.mapImageID = msg.ReadUUID();
            mapBlocks.append(block);

            boost::shared_ptr<Foundation::TextureServiceInterface> texture_service = framework_->GetServiceManager()->GetService<Foundation::TextureServiceInterface>(Foundation::Service::ST_Texture).lock();            
            if (texture_service)
            {
                texture_service->DeleteFromCache(block.mapImageID.ToString());
                request_tag_t tag = texture_service->RequestTexture(block.mapImageID.ToString());
                if (tag)
                    map_asset_requests_.push_back(tag);                
            }
        }

        worldmap_widget_->SetMapBlocks(mapBlocks);
        return false;
        
    }

    bool WorldMapModule::HandleOSNE_RegionHandshake(ProtocolUtilities::NetworkEventInboundData* data)
    {
        ///\note This message is mostly handled by EnvironmentModule.
        ProtocolUtilities::NetInMessage &msg = *data->message;
        msg.ResetReading();

        msg.SkipToNextVariable(); // RegionFlags U32
        msg.SkipToNextVariable(); // SimAccess U8

        std::string sim_name = msg.ReadString(); // SimName
        
        if (worldmap_widget_)
        {
            WorldMapModule::LogDebug("Setting current sim to world map: " + sim_name);
            worldmap_widget_->SetSimName(QString(sim_name.c_str()));
        }

        return false;
    }

    void WorldMapModule::UpdateAvatarPositions()
    {
        if (!worldmap_widget_)
            return;

        Scene::ScenePtr current_scene = framework_->GetDefaultWorldScene();        
        if (!current_scene.get())
            return;

        using namespace Foundation;
        if (worldmap_widget_->GetMyAvatarId().isEmpty())
        {
            Scene::EntityPtr my_avatar;
            boost::shared_ptr<WorldLogicInterface> worldLogic = framework_->GetServiceManager()->GetService<WorldLogicInterface>(Service::ST_WorldLogic).lock();
            if (worldLogic)            
                my_avatar = worldLogic->GetUserAvatarEntity();

            if (my_avatar)
            {
                EC_OpenSimPresence *user_presence = my_avatar->GetComponent<EC_OpenSimPresence>().get();
                if (user_presence)
                    worldmap_widget_->SetMyAvatarId(user_presence->agentId.ToQString());
            }               
        }

        Vector3df top_vector = Vector3df::UNIT_Z, position, direction;

        for(Scene::SceneManager::iterator iter = current_scene->begin(); iter != current_scene->end(); ++iter)
        {
            Scene::Entity &entity = *iter->second;
            EC_OpenSimPresence *presence_component = entity.GetComponent<EC_OpenSimPresence>().get();
            if (!presence_component)
                continue;

            EC_Placeable *ogre_placeable = entity.GetComponent<EC_Placeable>().get();
            if (ogre_placeable)
            {                
                Quaternion q = ogre_placeable->GetOrientation();
                position = ogre_placeable->GetPosition(); 
                direction = q*Vector3df::UNIT_X;

                worldmap_widget_->UpdateAvatarPosition(position, presence_component->agentId.ToQString(), presence_component->GetFullName().c_str());
            }
        }
                
    }

    QImage WorldMapModule::ConvertToQImage(Foundation::TextureInterface &tex)
    {
        uint img_width        = tex.GetWidth(); 
        uint img_height       = tex.GetHeight(); 
        uint img_components   = tex.GetComponents();
        u8 *data              = tex.GetData();
        uint img_width_step   = img_width * img_components;
        QImage image;

        if(img_width > 0 && img_height > 0 && img_components > 0)
        {
            if(img_components == 3)// For RGB888
            {
                image = QImage(QSize(img_width, img_height), QImage::Format_RGB888);
                for(uint height = 0; height < img_height; height++)
                {
                    for(uint width = 0; width < img_width; width++)
                    {
                        u8 color[3];
                        for(uint comp = 0; comp < img_components; comp++)
                        {
                            uint index = (height % img_height) * (img_width_step) + ((width * img_components) % (img_width_step)) + comp;
                            color[comp] = data[index];
                        }
                        image.setPixel(width, height, qRgb(color[0], color[1], color[2]));
                    }
                }
            }
            else if(img_components == 4)// For ARGB32
            {
                image = QImage(QSize(img_width, img_height), QImage::Format_ARGB32);
                for(uint height = 0; height < img_height; height++)
                {
                    for(uint width = 0; width < img_width; width++)
                    {
                        u8 color[4];
                        for(uint comp = 0; comp < img_components; comp++)
                        {
                            uint index = (height % img_height) * (img_width_step) + ((width * img_components) % (img_width_step)) + comp;
                            color[comp] = data[index];
                        }
                        image.setPixel(width, height, qRgba(color[0], color[1], color[2], color[3]));
                    }
                }
            }
        }

        return image;
    }

    const std::string WorldMapModule::moduleName = std::string("WorldMapModule");

    const std::string &WorldMapModule::NameStatic()
    {
        return moduleName;
    }

    extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
    void SetProfiler(Foundation::Profiler *profiler)
    {
        Foundation::ProfilerSection::SetProfiler(profiler);
    }

}

using namespace WorldMap;

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(WorldMapModule)
POCO_END_MANIFEST
