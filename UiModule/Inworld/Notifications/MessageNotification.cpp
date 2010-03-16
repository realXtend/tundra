// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "MessageNotification.h"

#include <QPlainTextEdit>

namespace UiServices
{
    MessageNotification::MessageNotification(QString message, int hide_in_msec) :
        CoreUi::NotificationBaseWidget(hide_in_msec)
    {
        QPlainTextEdit *message_box = new QPlainTextEdit(message);
        message_box->setReadOnly(true);
        message_box->setFrameShape(QFrame::NoFrame);

        QFontMetrics metric(message_box->font());
        QRect text_rect = metric.boundingRect(QRect(0,0,200,200), Qt::AlignLeft|Qt::TextWordWrap, message);
        message_box->setMaximumHeight(text_rect.height()+10);

        SetCentralWidget(message_box);
    }
}