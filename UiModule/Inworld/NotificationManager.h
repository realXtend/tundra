// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_NotificationManager_h
#define incl_UiModule_NotificationManager_h

#include "UiModuleApi.h"
#include "UiModule.h"

#include <QObject>
#include <QRectF>
#include <QPointF>

class QGraphicsScene;

namespace CoreUi
{
    class ControlPanelManager;
    class NotificationBaseWidget;
    class NotificationBrowserWidget;
}

namespace UiServices
{
    class InworldSceneController;

    class UI_MODULE_API NotificationManager : public QObject
    {

    Q_OBJECT

    public:
        NotificationManager(InworldSceneController *inworld_scene_controller);
        virtual ~NotificationManager();

    public slots:
        /// @param notification_widget The widget to show
        void ShowNotification(CoreUi::NotificationBaseWidget *notification_widget);

        /// Clear the notification history.
        /// Call after login or another logical context switch.
        void ClearHistory();

    private slots:
        void InitSelf();
        void UpdatePosition(const QRectF &scene_rect);
        void UpdateStack();
        void NotificationHideHandler(CoreUi::NotificationBaseWidget *completed_notification);

        void ToggleNotificationBrowser();
        void HideAllNotifications();

        void SceneAboutToChange(const QString &old_name, const QString &new_name);

	signals:
		void ShowNotificationCalled(const QString& msg);

    private:
        InworldSceneController *inworld_scene_controller_;
        QGraphicsScene *scene_;

        int notice_max_width_;
        QPointF notice_start_pos_;

        QList<CoreUi::NotificationBaseWidget *> notifications_history_;
        QList<CoreUi::NotificationBaseWidget *> visible_notifications_;

        CoreUi::NotificationBrowserWidget *browser_widget_;
        CoreUi::ControlPanelManager *panel_;

    };
}

#endif // incl_UiModule_NotificationManager_h