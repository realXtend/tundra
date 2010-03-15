// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "NotificationManager.h"
#include "InworldSceneController.h"

#include "ui_NotificationWidget.h"
#include "Inworld/Notifications/NotifyProxyWidget.h"
#include "Inworld/Notifications/NotifyLabel.h"
#include "CoreDefines.h"

#include <QGraphicsScene>

#include "MemoryLeakCheck.h"

namespace UiServices
{
    NotificationManager::NotificationManager(Foundation::Framework *framework, QGraphicsScene *scene) 
        : QObject(),
          notice_size_(0,0,200,100),
          start_point_(scene->width()-notice_size_.width()-5, 30),
          framework_(framework),
          scene_(scene)
    {
        
        connect(scene_, SIGNAL(sceneRectChanged(const QRectF &)), SLOT(SceneRectChanged(const QRectF &)));
    }

    NotificationManager::~NotificationManager()
    {

    }

    void NotificationManager::SceneRectChanged(const QRectF &rect)
    {
        start_point_.setX(rect.width()-notice_size_.width());
        ResizeAndPositionNotifyArea();
    }

    void NotificationManager::ShowInformationString(const QString &text, int duration_msec)
    {
        CoreUi::NotifyLabel *notify_label = new CoreUi::NotifyLabel(text, duration_msec);
        notify_label->setParent(this);
        connect(notify_label, SIGNAL( DestroyMe(CoreUi::NotifyLabel *)),this, SLOT( DestroyNotifyLabel(CoreUi::NotifyLabel *) ));
        notifications_.append(notify_label);
        visible_notifications_.append(notify_label);
        scene_->addItem(notify_label);
        ResizeAndPositionNotifyArea();
        notify_label->ShowNotification();
    }

    void NotificationManager::DestroyNotifyLabel(CoreUi::NotifyLabel *notification)
    {
        visible_notifications_.removeOne(notification);
        ResizeAndPositionNotifyArea();
    }

    void NotificationManager::ResizeAndPositionNotifyArea()
    {
        QPointF current_pos = start_point_;

        for(int i =0; i<visible_notifications_.size(); i++)
        {
            CoreUi::NotifyLabel *notify_label = visible_notifications_.at(i);
            notify_label->setPos(current_pos);
            current_pos.setY(current_pos.y() + notify_label->geometry().height());
        }
    }
}