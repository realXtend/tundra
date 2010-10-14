// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UiHelper.h"

#include <QTimer>

namespace Avatar
{
    namespace Helpers
    {
        UiHelper::UiHelper(QObject *parent) :
            QObject(parent)
        {
        }

        UiHelper::~UiHelper()
        {
            foreach(QWidget *widget, cleanup_widgets_)
                SAFE_DELETE(widget);
        }

        QGraphicsProxyWidget *UiHelper::CreateToolbar()
        {
            QWidget *toolbar = new QWidget();
            toolbar_ui_.setupUi(toolbar);
            connect(toolbar_ui_.button_exit, SIGNAL(clicked()), SIGNAL(ExitRequest()));
            return CreateProxy(toolbar);
        }

        QGraphicsProxyWidget *UiHelper::CreateInfoWidget()
        {
            QWidget *info = new QWidget();
            info_ui_.setupUi(info);
            info_proxy_ = CreateProxy(info);
            return info_proxy_;
        }

        QGraphicsProxyWidget *UiHelper::CreateProxy(QWidget *widget)
        {
            cleanup_widgets_ << widget;
            QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(0, Qt::Widget);
            proxy->setWidget(widget);
            return proxy;
        }

        void UiHelper::ShowStatus(const QString &message, int msec_timeout)
        {
            info_ui_.info_label->setText(message);
            info_ui_.icon_label->hide();
            info_proxy_->show();

            if (msec_timeout > 0)
                QTimer::singleShot(msec_timeout, this, SLOT(HideInfo()));
        }

        void UiHelper::ShowError(const QString &error, int msec_timeout)
        {
            info_ui_.icon_label->setStyleSheet("QLabel#icon_label{background-image:url('./data/ui/images/icon_warning.png');}");
            if (!info_ui_.icon_label->isVisible())
                info_ui_.icon_label->show();

            info_ui_.info_label->setText(error);
            info_proxy_->show();

            if (msec_timeout > 0)
                QTimer::singleShot(msec_timeout, this, SLOT(HideInfo()));
        }

        void UiHelper::HideInfo()
        {
            info_proxy_->hide();
        }
    }
}