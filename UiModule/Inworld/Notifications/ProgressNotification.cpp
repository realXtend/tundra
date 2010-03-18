// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ProgressNotification.h"

#include <QTimeLine>
#include <QPlainTextEdit>

namespace UiServices
{
    // class ProgressController

    ProgressController::ProgressController() :
        QObject(),
        progress_timeline(0)
    {

    }

    ProgressController::ProgressController(int linear_progress_length) :
        QObject()
    {
        progress_timeline = new QTimeLine(linear_progress_length, this);
        progress_timeline->setFrameRange(0,100);
        connect(progress_timeline, SIGNAL(frameChanged(int)), SLOT(SetValue(int)));
    }

    void ProgressController::SetValue(int value)
    {
        if (value >= 0 && value <= 100)
            emit StepUpdate(value);
        if (value == 100)
            emit Finished();
    }

    void ProgressController::Finish()
    {
        emit Finished();
    }

    // class ProgressNotification
    
    ProgressNotification::ProgressNotification(QString message, ProgressController *controller, int hide_in_msec) :
        CoreUi::NotificationBaseWidget(hide_in_msec),
        controller_(controller)
    {
        SetActive(true);

        // Init Ui
        QPlainTextEdit *message_box = new QPlainTextEdit(message);
        message_box->setReadOnly(true);
        message_box->setFrameShape(QFrame::NoFrame);

        QFontMetrics metric(message_box->font());
        QRect text_rect = metric.boundingRect(QRect(0,0,200,400), Qt::AlignLeft|Qt::TextWordWrap, message);
        message_box->setMaximumHeight(text_rect.height() + 2*metric.height());
        message_box->setMinimumHeight(text_rect.height() + 2*metric.height());

        QProgressBar *progress_bar = new QProgressBar();
        progress_bar->setValue(0);

        // Layout
        QVBoxLayout *v_layout = new QVBoxLayout();
        v_layout->addWidget(message_box);
        v_layout->addWidget(progress_bar);

        SetCentralLayout(v_layout);
        
        // Connect Signals
        connect(controller_, SIGNAL(StepUpdate(int)), progress_bar, SLOT(setValue(int)));
        connect(controller_, SIGNAL(Finished()), SLOT(Finished()));

        if (controller_->progress_timeline)
            controller_->progress_timeline->start();
    }

    ProgressNotification::~ProgressNotification()
    {
        SAFE_DELETE(controller_);
    }

    void ProgressNotification::Finished()
    {
        SetActive(false);
        SetResult("Completed");
    }
}