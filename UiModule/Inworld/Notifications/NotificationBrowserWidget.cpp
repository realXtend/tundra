// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NotificationBrowserWidget.h"
#include "NotificationBaseWidget.h"

namespace CoreUi
{
    NotificationBrowserWidget::NotificationBrowserWidget() :
        QGraphicsProxyWidget(0, Qt::Widget),
        internal_widget_(new QWidget())
    {
        setupUi(internal_widget_);
        setWidget(internal_widget_);
    }

    void NotificationBrowserWidget::ShowNotifications(QList<NotificationBaseWidget *> all_notifications)
    {
        // TODO: Sort list with timestamp

        QList<NotificationBaseWidget *> active_notifications;
        QList<NotificationBaseWidget *> logged_notifications;

        foreach (NotificationBaseWidget *notification, all_notifications)
        {
            if (notification->IsActive())
                active_notifications.append(notification);
            else
                logged_notifications.append(notification);
        }

        foreach (NotificationBaseWidget *active_notification, active_notifications)
            activeLayout->addWidget(active_notification->GetContentWidget());

        foreach (NotificationBaseWidget *log_notification, logged_notifications)
            logLayout->addWidget(log_notification->GetContentWidget());

        show();
    }
}