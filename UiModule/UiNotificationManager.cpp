// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UiNotificationManager.h"
#include "UiSceneManager.h"

#include "ui_NotificationWidget.h"
#include "Notify/NotifyProxyWidget.h"
#include "Notify/NotifyLabel.h"

#include <QLabel>

namespace UiServices
{
    UiNotificationManager::UiNotificationManager(Foundation::Framework *framework, QGraphicsView *ui_view) 
        : QObject(),
          framework_(framework),
          ui_view_(ui_view),
          notification_widget_(new QWidget()),
          notification_ui_(new Ui::NotificationWidget()),
          notification_proxy_widget_(0),
          visible_notifications_(0)
    {
        notification_ui_->setupUi(notification_widget_);
        notification_proxy_widget_ = new CoreUi::NotifyProxyWidget(notification_widget_);
        ui_view_->scene()->addItem(notification_proxy_widget_);
    }

    UiNotificationManager::~UiNotificationManager()
    {
        SAFE_DELETE(notification_widget_);
        SAFE_DELETE(notification_ui_);
    }

    void UiNotificationManager::ShowInformationString(const QString &text)
    {
        visible_notifications_++;
        CoreUi::NotifyLabel *notify_label = new CoreUi::NotifyLabel(text);
        notification_ui_->verticalMainLayout->addWidget(notify_label);
        notification_proxy_widget_->CheckPosition();
    }
}