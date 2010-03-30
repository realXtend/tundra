// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "QuestionNotification.h"

#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>

namespace UiServices
{

    QuestionNotification::QuestionNotification(QString question, QString first_button_title, QString second_button_title,
                                               QString third_button_title, int hide_in_msec) :
        CoreUi::NotificationBaseWidget(hide_in_msec, question),
        question_box_(new QPlainTextEdit(question)),
        first_button_(new QPushButton(first_button_title)),
        second_button_(new QPushButton(second_button_title)),
        third_button_(0)
    {
        SetActive(true);

        // Ui init
        question_box_->setReadOnly(true);
        question_box_->setFrameShape(QFrame::NoFrame);
        
        QFontMetrics metric(question_box_->font());
        QRect text_rect = metric.boundingRect(QRect(0,0,160,400), Qt::AlignLeft|Qt::TextWordWrap, question);
        question_box_->setMaximumHeight(text_rect.height() + metric.height());
        question_box_->setMinimumHeight(text_rect.height() + metric.height());

        metric = first_button_->font();
        first_button_->setMinimumWidth(metric.width(first_button_title) + 20);
        first_button_->setMinimumHeight(metric.height() + 5);
        first_button_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        
        metric = second_button_->font();
        second_button_->setMinimumWidth(metric.width(first_button_title) + 20);
        second_button_->setMinimumHeight(metric.height() + 5);
        second_button_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

        // Layout
        QWidget *content_widget = new QWidget();
        QVBoxLayout *v_layout = new QVBoxLayout();
        QHBoxLayout *h_layout = new QHBoxLayout();

        h_layout->addWidget(first_button_);
        h_layout->addWidget(second_button_);

        if (!third_button_title.isEmpty())
        {
            third_button_ = new QPushButton(third_button_title);
            metric = third_button_->font();
            third_button_->setMinimumWidth(metric.width(third_button_title) + 20);
            third_button_->setMinimumHeight(metric.height() + 5);
            third_button_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
            h_layout->addWidget(third_button_);
        }

        v_layout->addWidget(question_box_);
        v_layout->addLayout(h_layout);

        content_widget->setLayout(v_layout);
        SetCentralWidget(content_widget);

        // Connect signals
        connect(first_button_, SIGNAL(clicked()), SLOT(FirstButtonClicked()));
        connect(second_button_, SIGNAL(clicked()), SLOT(SecondButtonClicked()));
        if (third_button_)
            connect(third_button_, SIGNAL(clicked()), SLOT(ThirdButtonClicked()));

        // Hide interaction elements when finished
        connect(this, SIGNAL(HideInteractionWidgets()), first_button_, SLOT(hide()));
        connect(this, SIGNAL(HideInteractionWidgets()), second_button_, SLOT(hide()));
        if (third_button_)
            connect(this, SIGNAL(HideInteractionWidgets()), third_button_, SLOT(hide()));
    }

    void QuestionNotification::FirstButtonClicked()
    {
        EmitAnswer(first_button_->text());
    }

    void QuestionNotification::SecondButtonClicked()
    {
        EmitAnswer(second_button_->text());
    }

    void QuestionNotification::ThirdButtonClicked()
    {
        EmitAnswer(third_button_->text());
    }

    void QuestionNotification::EmitAnswer(QString clicked_button_title)
    {
        emit QuestionAnswered(clicked_button_title);
        SetResult("Answered", clicked_button_title);
        TimedOut();
        SetActive(false);
    }
}