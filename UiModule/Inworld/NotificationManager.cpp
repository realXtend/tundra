// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "NotificationManager.h"
#include "InworldSceneController.h"

#include "Inworld/View/MainPanel.h"
#include "Inworld/Notifications/NotificationBaseWidget.h"
#include "Inworld/Notifications/NotificationBrowserWidget.h"

#include <QGraphicsScene>
#include <QDebug>

#include "MemoryLeakCheck.h"

namespace UiServices
{
    NotificationManager::NotificationManager(InworldSceneController *inworld_scene_controller) : 
        QObject(),
        inworld_scene_controller_(inworld_scene_controller),
        scene_(inworld_scene_controller->GetInworldScene()),
        notice_max_width_(200),
        notice_start_pos_(QPointF()),
        browser_widget_(new CoreUi::NotificationBrowserWidget())
    {
        InitSelf();
    }

    NotificationManager::~NotificationManager()
    {
        // Clean up whole notification history
        foreach(CoreUi::NotificationBaseWidget *notification, notifications_history_)
            SAFE_DELETE(notification);
    }

    // Private

    void NotificationManager::InitSelf()
    {
        browser_widget_->hide();
        scene_->addItem(browser_widget_);

        QPushButton *notification_toggle_button = inworld_scene_controller_->GetMainPanel()->GetNotificationsButton();
        if (notification_toggle_button)
            connect(notification_toggle_button, SIGNAL(clicked()), SLOT(ToggleNotificationBrowser()));
        connect(scene_, SIGNAL(sceneRectChanged(const QRectF&)), SLOT(UpdatePosition(const QRectF &)));
    }

    void NotificationManager::UpdatePosition(const QRectF &scene_rect)
    {
        notice_start_pos_.setY(inworld_scene_controller_->GetMainPanel()->size().height());
        notice_start_pos_.setX(scene_rect.right()-notice_max_width_);
        UpdateStack();
    }

    void NotificationManager::UpdateStack()
    {
        if (visible_notifications_.isEmpty())
            return;

        // Iterate from start of stack and animate all items to correct positions
        QPointF next_position = notice_start_pos_;
        foreach(CoreUi::NotificationBaseWidget *notification, visible_notifications_)
        {
            notification->AnimateToPosition(next_position);
            next_position.setY(next_position.y()+notification->size().height());
        }
    }

    void NotificationManager::NotificationHideHandler(CoreUi::NotificationBaseWidget *completed_notification)
    {
        if (visible_notifications_.contains(completed_notification))
        {
            visible_notifications_.removeOne(completed_notification);
            UpdateStack();
        }
    }

    void NotificationManager::ToggleNotificationBrowser()
    {
        if (!browser_widget_->isVisible())
        {
            browser_widget_->setPos(scene_->sceneRect().right()-browser_widget_->size().width(), inworld_scene_controller_->GetMainPanel()->size().height());
            browser_widget_->ShowNotifications(notifications_history_);
        }
        else
            browser_widget_->hide();
    }

    // Public

    void NotificationManager::ShowNotification(CoreUi::NotificationBaseWidget *notification_widget)
    {
        // Don't show same item twice before first is hidden
        if (visible_notifications_.contains(notification_widget))
            return;

        UpdatePosition(scene_->sceneRect());
        QPointF add_position = notice_start_pos_;

        // Get stacks last notifications y position
        if (!visible_notifications_.isEmpty())
        {
            CoreUi::NotificationBaseWidget *last_notification = visible_notifications_.last();
            add_position.setY(notice_start_pos_.y() + last_notification->mapRectToScene(last_notification->rect()).bottom());
        }

        // Set position and add to scene
        notification_widget->setPos(add_position);
        scene_->addItem(notification_widget);

        // Connect completed (hide) signal to managers handler
        connect(notification_widget, SIGNAL(Completed(CoreUi::NotificationBaseWidget *)),
                SLOT(NotificationHideHandler(CoreUi::NotificationBaseWidget *)));

        // Append to internal lists
        notifications_history_.append(notification_widget);
        visible_notifications_.append(notification_widget);

        // Start notification
        notification_widget->Start();
    }

    void NotificationManager::SetConnectionState(ConnectionState connection_state)
    {
        switch (connection_state)
        {
            case Connected:
                break;

            case Disconnected:
                foreach(CoreUi::NotificationBaseWidget *notification, notifications_history_)
                    SAFE_DELETE(notification);
                notifications_history_.clear();
                visible_notifications_.clear();
                break;
        }
    }
}