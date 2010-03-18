// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "InputNotification.h"

#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>

namespace UiServices
{
    InputNotification::InputNotification(QString message, QString button_title, int hide_in_msec) :
        CoreUi::NotificationBaseWidget(hide_in_msec),
        message_box_(new QPlainTextEdit(message)),
        answer_line_edit_(new QLineEdit()),
        answer_button_(new QPushButton(button_title))
    {
        SetActive(true);

        // Ui init
        message_box_->setReadOnly(true);
        message_box_->setFrameShape(QFrame::NoFrame);
        QFontMetrics metric(message_box_->font());
        QRect text_rect = metric.boundingRect(QRect(0,0,200,400), Qt::AlignLeft|Qt::TextWordWrap, message);
        message_box_->setMaximumHeight(text_rect.height() + 2*metric.height());
        message_box_->setMinimumHeight(text_rect.height() + 2*metric.height());

        metric = answer_button_->font();
        answer_button_->setMinimumWidth(metric.width(button_title) + 10);
        answer_button_->setMinimumHeight(metric.height() + 5);
        answer_button_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        
        // Layout
        QWidget *content_widget = new QWidget();
        QVBoxLayout *v_layout = new QVBoxLayout();
        QHBoxLayout *h_layout = new QHBoxLayout();

        h_layout->addWidget(answer_line_edit_);
        h_layout->addWidget(answer_button_);

        v_layout->addWidget(message_box_);
        v_layout->addLayout(h_layout);

        content_widget->setLayout(v_layout);
        SetCentralWidget(content_widget);

        // Connect signals
        connect(answer_line_edit_, SIGNAL(returnPressed()), SLOT(ParseAndEmitInput()));
        connect(answer_button_, SIGNAL(clicked()), SLOT(ParseAndEmitInput()));
    }

    void InputNotification::ParseAndEmitInput()
    {
        emit InputRecieved(answer_line_edit_->text());
        SetActive(false);
        SetResult(answer_line_edit_->text());
        TimedOut();
    }
}