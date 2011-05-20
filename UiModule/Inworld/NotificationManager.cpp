// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "NotificationManager.h"
#include "InworldSceneController.h"

//#include "Inworld/ControlPanelManager.h"
#include "Inworld/Notifications/NotificationBaseWidget.h"
#include "Inworld/Notifications/NotificationBrowserWidget.h"

//#include "Common/ControlButtonAction.h"
#include <QPushButton>

#include <QGraphicsScene>
#include <QDebug>

#include "MemoryLeakCheck.h"

namespace UiServices
{
    NotificationManager::NotificationManager(InworldSceneController *inworld_scene_controller) : 
        QObject(),
        inworld_scene_controller_(inworld_scene_controller),
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
        //Create and manage button to manage visibility in scene
        QPushButton *nottif_button = new QPushButton("Notifications");
        connect(nottif_button, SIGNAL(clicked()),SLOT(ToggleNotificationBrowser()));
        inworld_scene_controller_->AddAnchoredWidgetToScene(nottif_button, Qt::TopRightCorner, Qt::Horizontal, 50, true);

        //Add widget
        browser_widget_->hide();
        inworld_scene_controller_->AddAnchoredWidgetToScene(browser_widget_, Qt::TopRightCorner, Qt::Vertical, 75, true);
    }

    void NotificationManager::NotificationHideHandler(CoreUi::NotificationBaseWidget *completed_notification)
    {
        if (visible_notifications_.contains(completed_notification))
            visible_notifications_.removeOne(completed_notification);

        //Remove it from AnchorLayout
        inworld_scene_controller_->RemoveAnchoredWidgetFromScene(completed_notification);
    }

    void NotificationManager::ToggleNotificationBrowser()
    {
        if (!browser_widget_->isVisible())
        {
            // Hide visible notification stack
            foreach (CoreUi::NotificationBaseWidget *notification, visible_notifications_)
                notification->hide();
            visible_notifications_.clear();

            // Pass history to browser and clear local list
            browser_widget_->ShowNotifications(notifications_history_);
            notifications_history_.clear();
        }
        else
            browser_widget_->AnimatedHide();
    }

    void NotificationManager::HideAllNotifications()
    {
        foreach(CoreUi::NotificationBaseWidget *notification, visible_notifications_)
            notification->HideNow();
    }

    void NotificationManager::SceneAboutToChange()
    {
        //if (old_name.toLower() == "inworld") Done already because is connected with signal SceneChangedFromMain
            HideAllNotifications();
    }

    // Public

    void NotificationManager::ShowNotification(CoreUi::NotificationBaseWidget *notification_widget)
    {
        if (browser_widget_->isVisible())
            browser_widget_->InsertNotifications(notification_widget);
        else
        {
            // Don't show same item twice before first is hidden
            if (visible_notifications_.contains(notification_widget))
                return;

            inworld_scene_controller_->AddAnchoredWidgetToScene(notification_widget, Qt::TopRightCorner, Qt::Vertical, 80, true);

            // Connect completed (hide) signal to managers handler
            connect(notification_widget, SIGNAL(Completed(CoreUi::NotificationBaseWidget *)),
                    SLOT(NotificationHideHandler(CoreUi::NotificationBaseWidget *)));

            // Append to internal lists
            notifications_history_.append(notification_widget);
            visible_notifications_.append(notification_widget);

            // Start notification
            notification_widget->Start();
        }
		QString msg(notification_widget->GetMessage());
		emit ShowNotificationCalled(msg);
    }

    void NotificationManager::ClearHistory()
    {
        foreach(CoreUi::NotificationBaseWidget *notification, notifications_history_)
            SAFE_DELETE(notification);
        notifications_history_.clear();
        visible_notifications_.clear();
        
        browser_widget_->hide();
        browser_widget_->ClearAllContent();
    }
}