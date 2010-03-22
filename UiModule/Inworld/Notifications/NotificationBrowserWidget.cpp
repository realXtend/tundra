// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NotificationBrowserWidget.h"

#include "NotificationBaseWidget.h"
#include "NotificationLogWidget.h"

#include <QPropertyAnimation>

namespace CoreUi
{
    NotificationBrowserWidget::NotificationBrowserWidget() :
        QGraphicsProxyWidget(0, Qt::Widget),
        internal_widget_(new QWidget()),
        next_bg_color_("white")
    {
        setupUi(internal_widget_);
        setWidget(internal_widget_);

        visibility_animation_ = new QPropertyAnimation(this, "opacity", this);
        visibility_animation_->setDuration(500);
        visibility_animation_->setEasingCurve(QEasingCurve::InOutSine);
        visibility_animation_->setStartValue(0);
        visibility_animation_->setEndValue(1);
        connect(visibility_animation_, SIGNAL(finished()), SLOT(AnimationsFinished()));
    }

    // Private

    void NotificationBrowserWidget::MoveActiveToLog(QWidget *active_widget, QString result_title, QString result)
    {
        NotificationLogWidget *found_log_widget_ = 0;
        foreach(NotificationLogWidget *log_widget, notification_log_widgets_)
        {
            if (log_widget->GetContentWidget() == active_widget)
            {
                found_log_widget_ = log_widget;
                break;
            }
        }

        if (!found_log_widget_)
            return;
        if (activeLayout->indexOf(found_log_widget_) == -1)
            return;

        // TODO: iterate log layout, order by timestamp and insert widget into correct index
        found_log_widget_->Deactivate(result_title, result);
        logLayout->insertWidget(0, found_log_widget_);
        TabCheck();
    }

    void NotificationBrowserWidget::LayoutCheck()
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
    }

    void NotificationBrowserWidget::TabCheck()
    {
        // If there are no active items, set log tab to show
        if (activeLayout->count() <= 1)
            categoryTabWidget->setCurrentWidget(logTab);
        else
            categoryTabWidget->setCurrentWidget(activeTab);
    }

    void NotificationBrowserWidget::AnimationsFinished()
    {
        if (visibility_animation_->direction() == QAbstractAnimation::Backward)
            hide();
    }

    // Protected

    void NotificationBrowserWidget::showEvent(QShowEvent *show_event)
    {
        setOpacity(0);
        QGraphicsProxyWidget::showEvent(show_event);
        AnimatedShow();
    }

    // Public

    void NotificationBrowserWidget::AnimatedShow()
    {
        visibility_animation_->setDirection(QAbstractAnimation::Forward);
        visibility_animation_->start();
    }

    void NotificationBrowserWidget::AnimatedHide()
    {
        visibility_animation_->setDirection(QAbstractAnimation::Backward);
        visibility_animation_->start();
    }
    
    void NotificationBrowserWidget::InsertNotifications(NotificationBaseWidget *notification)
    {
        QList<NotificationBaseWidget *> notifications;
        notifications.append(notification);
        ShowNotifications(notifications);
    }

    void NotificationBrowserWidget::ShowNotifications(QList<NotificationBaseWidget *> notifications)
    {
        LayoutCheck();

        // Add NotificationBaseWidgets to cleanup list
        cleanup_list_ += notifications;
 
        // List active and log notifications
        QList<NotificationBaseWidget *> active_notifications;
        QList<NotificationBaseWidget *> logged_notifications;

        foreach (NotificationBaseWidget *notification, notifications)
        {
            if (notification->IsActive())
                active_notifications.append(notification);
            else
                logged_notifications.append(notification);
        }

        // Add active log widgets to layout
        foreach (NotificationBaseWidget *active_notification, active_notifications)
        {
            NotificationLogWidget *log_widget = new NotificationLogWidget(true, active_notification->GetContentWidget(), active_notification->GetTimeStamp(),
                                                                          active_notification->GetResultTitle(), active_notification->GetResult());
            activeLayout->insertWidget(0, log_widget);
            notification_log_widgets_.append(log_widget);

            connect(active_notification, SIGNAL(ResultsAreIn(QWidget *, QString, QString)), SLOT(MoveActiveToLog(QWidget *, QString, QString)));
        }

        // Add inactive log widgets to layout
        foreach (NotificationBaseWidget *log_notification, logged_notifications)
        {
            NotificationLogWidget *log_widget = new NotificationLogWidget(false, log_notification->GetContentWidget(), log_notification->GetTimeStamp(),
                                                                          log_notification->GetResultTitle(), log_notification->GetResult());
            logLayout->insertWidget(0, log_widget);
            notification_log_widgets_.append(log_widget);
        }

        // Show with appropriate tab selected
        TabCheck();
        show();
    }

    void NotificationBrowserWidget::ClearAllContent()
    {
        // Clean up browser widget layouts
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

        // Clean notification objects
        foreach(NotificationBaseWidget *notification, cleanup_list_)
            SAFE_DELETE(notification);
        cleanup_list_.clear();
    }
}