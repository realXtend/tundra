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

    // Private

    void NotificationBrowserWidget::MoveActiveToLog(QWidget *active_widget)
    {
        logLayout->insertWidget(0, active_widget);
        if (activeLayout->count() <= 1)
            categoryTabWidget->setCurrentWidget(logTab);
    }

    // Public
    
    void NotificationBrowserWidget::ShowNotifications(QList<NotificationBaseWidget *> all_notifications)
    {
        // Init layouts if needed
        if (activeLayout->count() == 0)
        {
            activeLayout->setSpacing(0);
            activeLayout->setContentsMargins(0,0,0,0);
            activeLayout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
        }
        if (logLayout->count() == 0)
        {
            logLayout->setSpacing(0);
            logLayout->setContentsMargins(0,0,0,0);
            logLayout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
        }
 
        // List active and log notifications
        QList<NotificationBaseWidget *> active_notifications;
        QList<NotificationBaseWidget *> logged_notifications;

        foreach (NotificationBaseWidget *notification, all_notifications)
        {
            if (notification->IsActive())
                active_notifications.append(notification);
            else
                logged_notifications.append(notification);
        }

        // Add notifications to layouts
        foreach (NotificationBaseWidget *active_notification, active_notifications)
        {
            activeLayout->insertWidget(0, active_notification->GetContentWidget());
            connect(active_notification, SIGNAL(InteractionsDone(QWidget *)),
                    SLOT(MoveActiveToLog(QWidget *)));
        }

        foreach (NotificationBaseWidget *log_notification, logged_notifications)
            logLayout->insertWidget(0, log_notification->GetContentWidget());

        // If there are no active items, set log tab to show
        if (activeLayout->count() <= 1)
            categoryTabWidget->setCurrentWidget(logTab);
        else
            categoryTabWidget->setCurrentWidget(activeTab);

        show();
    }

    void NotificationBrowserWidget::ClearAllContent()
    {
        QList<QLayout*> layouts;
        layouts << activeLayout << logLayout;

        foreach(QLayout *layout, layouts)
        {
            QList<QLayoutItem*> items;
            for (int i=0; i<layout->count(); ++i)
                items.append(layout->itemAt(i));
            foreach (QLayoutItem* item, items)
            {
                layout->removeItem(item);
                delete item->widget();
            }
        }
    }
}