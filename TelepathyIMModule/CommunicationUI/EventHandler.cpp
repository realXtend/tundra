// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EventHandler.h"

#include "Foundation.h"
#include "SceneEvents.h"
#include "SceneManager.h"

#include "EC_OpenSimPresence.h"
#include "EC_Placeable.h"

#include <QDebug>

#include "MemoryLeakCheck.h"

namespace CommunicationUI
{
    EventHandler::EventHandler(Foundation::Framework *framework)
        : QObject(),
          framework_(framework),
          spatial_sound_update_timer_(new QTimer(this)),
          tracking_(false)
    {
        connect(spatial_sound_update_timer_, SIGNAL( timeout() ), SLOT( UpdateAvatarPosition() ));
    }

    EventHandler::~EventHandler()
    {
    }

    QMap<QString, QString> EventHandler::GetAvatarList()
    {
        QMap<QString, QString> id_to_name;
        Scene::ScenePtr current_scene = framework_->GetDefaultWorldScene();
        if (current_scene.get())
        {
            for(Scene::SceneManager::iterator iter = current_scene->begin(); iter != current_scene->end(); ++iter)
            {
                Scene::Entity &entity = *iter->second;
                EC_OpenSimPresence *presence_component = entity.GetComponent<EC_OpenSimPresence>().get();
                if (presence_component)
                    id_to_name.insert(presence_component->agentId.ToQString(), presence_component->GetFullName().c_str());
            }
            return id_to_name;
        }
        return QMap<QString, QString>();
    }

    void EventHandler::StartTrackingSelectedAvatar(QString avatar_id)
    {
        tracking_ = true;
        avatar_id_ = avatar_id;
        spatial_sound_update_timer_->start(100);
        emit TrackingAvatar(tracking_);
    }

    void EventHandler::StopTrackingSelectedAvatar()
    { 
        tracking_ = false;
        spatial_sound_update_timer_->stop();
        emit TrackingAvatar(tracking_);
    }

    void EventHandler::UpdateAvatarPosition()
    {
        Scene::ScenePtr current_scene = framework_->GetDefaultWorldScene();
        if (!current_scene.get())
            return;

        for(Scene::SceneManager::iterator iter = current_scene->begin(); iter != current_scene->end(); ++iter)
        {
            Scene::Entity &entity = *iter->second;
            EC_OpenSimPresence *presence_component = entity.GetComponent<EC_OpenSimPresence>().get();
            if (!presence_component)
                continue;

            if (avatar_id_ == presence_component->agentId.ToQString())
            {
                EC_Placeable *placeable_component = entity.GetComponent<EC_Placeable>().get();
                if (placeable_component)
                {
                    current_position_ = placeable_component->GetPosition();
                    emit AvatarPositionUpdated(current_position_);
                }
            }
        }
    }
}
