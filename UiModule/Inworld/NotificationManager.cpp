// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "NotificationManager.h"
#include "InworldSceneController.h"

#include "ui_NotificationWidget.h"
#include "Inworld/Notifications/NotifyProxyWidget.h"
#include "Inworld/Notifications/NotifyLabel.h"
#include "CoreDefines.h"

#include <QLabel>

#include "MemoryLeakCheck.h"

namespace UiServices
{
    NotificationManager::NotificationManager(Foundation::Framework *framework, QGraphicsView *ui_view) 
        : QObject(),
          framework_(framework),
          ui_view_(ui_view),
          notification_widget_(new QWidget()),
          notification_ui_(new Ui::NotificationWidget()),
          notification_proxy_widget_(0),
          visible_notifications_(0)
    {
        notification_ui_->setupUi(notification_widget_);

        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        notification_widget_->setSizePolicy(sizePolicy);

        notification_proxy_widget_ = new CoreUi::NotifyProxyWidget(notification_widget_);
        ui_view_->scene()->addItem(notification_proxy_widget_);
    }

    NotificationManager::~NotificationManager()
    {
        SAFE_DELETE(notification_widget_);
        SAFE_DELETE(notification_ui_);
    }

    void NotificationManager::ShowInformationString(const QString &text, int duration_msec)
    {
        if (!notification_proxy_widget_->isVisible())
            notification_proxy_widget_->show();
        visible_notifications_++;

        CoreUi::NotifyLabel *notify_label = new CoreUi::NotifyLabel(text, duration_msec);
        connect(notify_label, SIGNAL( DestroyMe(CoreUi::NotifyLabel *)), SLOT( DestroyNotifyLabel(CoreUi::NotifyLabel *) ));

        notification_ui_->verticalMainLayout->addWidget(notify_label);
        ResizeAndPositionNotifyArea();
    }

    void NotificationManager::DestroyNotifyLabel(CoreUi::NotifyLabel *notification)
    {
        int index = notification_ui_->verticalMainLayout->indexOf(notification);
        if (index != -1)
        {
            visible_notifications_--;
            notification_ui_->verticalMainLayout->removeItem(notification_ui_->verticalMainLayout->itemAt(index));
            SAFE_DELETE(notification);
            if (visible_notifications_ == 0)
                notification_proxy_widget_->hide();
            else
                ResizeAndPositionNotifyArea();
        }
    }

    void NotificationManager::ResizeAndPositionNotifyArea()
    {
        int count = notification_ui_->verticalMainLayout->count();
        if (count > 0)
        {
            int min_height = notification_ui_->verticalMainLayout->itemAt(0)->maximumSize().height();
            notification_widget_->setMinimumHeight(count*min_height);
            notification_widget_->resize(0, 18);
            notification_proxy_widget_->CheckPosition();
        }
    }
}