// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EventHandler.h"

#include "Foundation.h"
#include "SceneEvents.h"
#include "SceneManager.h"

#include "EntityComponent/EC_OpenSimPresence.h"
#include "EC_OgrePlaceable.h"

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
                Scene::Entity &entity = **iter;
                const Foundation::ComponentInterfacePtr &presence_component = entity.GetComponent("EC_OpenSimPresence");
                if (presence_component)
                {
                    RexLogic::EC_OpenSimPresence *avatar_precence = checked_static_cast<RexLogic::EC_OpenSimPresence *>(presence_component.get());
                    id_to_name.insert( QString(avatar_precence->FullId.ToString().c_str()), QString(avatar_precence->GetFullName().c_str()) );
                }
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
        if (current_scene.get())
        {
            for(Scene::SceneManager::iterator iter = current_scene->begin(); iter != current_scene->end(); ++iter)
            {
                Scene::Entity &entity = **iter;
                const Foundation::ComponentInterfacePtr &presence_component = entity.GetComponent("EC_OpenSimPresence");
                if (presence_component)
                {
                    RexLogic::EC_OpenSimPresence *avatar_precence = checked_static_cast<RexLogic::EC_OpenSimPresence *>(presence_component.get());
                    if ( avatar_id_ == QString(avatar_precence->FullId.ToString().c_str()) )
                    {
                        const Foundation::ComponentInterfacePtr &placeable_component = entity.GetComponent("EC_OgrePlaceable");
                        if (placeable_component)
                        {
                            OgreRenderer::EC_OgrePlaceable *ogre_placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(placeable_component.get());
                            current_position_ = ogre_placeable->GetPosition();
                            emit AvatarPositionUpdated(current_position_);
                        }
                    }
                }
            }

        }
    }
}