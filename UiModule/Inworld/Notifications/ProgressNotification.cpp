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
        progress_timeline(0),
        finished_(false),
        job_duration_sec_(-1)
    {

    }

    ProgressController::ProgressController(int linear_progress_length) :
        QObject(),
        finished_(false),
        job_duration_sec_(-1)
    {
        progress_timeline = new QTimeLine(linear_progress_length, this);
        progress_timeline->setFrameRange(0,100);
        connect(progress_timeline, SIGNAL(frameChanged(int)), SLOT(SetValue(int)));
    }

    void ProgressController::Start(int value)
    {
        if (value >= 0)
        {
            emit StepUpdate(value);

            if (value != 0)
            {
                emit Started();
                job_timer_.start();
            }
        }
        else
        {
            if (progress_timeline)
            {
                progress_timeline->start();
                job_timer_.start();
            }
        }
    }

    void ProgressController::SetValue(int value)
    {
        if (finished_)
            return;

        if (value == 0)
        {
            emit Started();
            job_timer_.start();
        }
        if (value >= 0 && value <= 100)
            emit StepUpdate(value);
        if (value == 100)
            Finish();
    }

    void ProgressController::Finish()
    {
        qreal ms = job_timer_.elapsed();
        job_duration_sec_ = ms / 1000;

        emit Finished();
        finished_ = true;
    }

    void ProgressController::FailWithReason(QString reason)
    {
        qreal ms = job_timer_.elapsed();
        job_duration_sec_ = ms / 1000;

        emit Failed(reason);
        finished_ = true;
    }

    // class ProgressNotification
    
    ProgressNotification::ProgressNotification(QString message, ProgressController *controller, int hide_in_msec) :
        CoreUi::NotificationBaseWidget(hide_in_msec, message),
        controller_(controller)
    {
        SetActive(true);

        // Init Ui
        QPlainTextEdit *message_box = new QPlainTextEdit(message);
        message_box->setReadOnly(true);
        message_box->setFrameShape(QFrame::NoFrame);

        QFontMetrics metric(message_box->font());
        QRect text_rect = metric.boundingRect(QRect(0,0,160,400), Qt::AlignLeft|Qt::TextWordWrap, message);
        message_box->setMaximumHeight(text_rect.height() + metric.height());
        message_box->setMinimumHeight(text_rect.height() + metric.height());

        QProgressBar *progress_bar = new QProgressBar();
        progress_bar->setValue(0);

        // Layout
        QWidget *content_widget = new QWidget();
        QVBoxLayout *v_layout = new QVBoxLayout();
        v_layout->addWidget(message_box);
        v_layout->addWidget(progress_bar);

        content_widget->setLayout(v_layout);
        SetCentralWidget(content_widget);
        
        // Connect Signals
        connect(controller_, SIGNAL(StepUpdate(int)), progress_bar, SLOT(setValue(int)));
        connect(controller_, SIGNAL(Finished()), SLOT(Finished()));
        connect(controller_, SIGNAL(Failed(QString)), SLOT(Failed(QString)));

        // Hide interaction elements when finished
        connect(this, SIGNAL(HideInteractionWidgets()), progress_bar, SLOT(hide()));

        controller_->Start(-1);
    }

    ProgressNotification::~ProgressNotification()
    {
        SAFE_DELETE(controller_);
    }

    void ProgressNotification::Finished()
    {
        emit Completed();
        QString duration = QString::number(controller_->JobDurationInSeconds());
        if (duration.indexOf(".") != -1)
            SetResult("Result", "Completed in " + duration.left(duration.indexOf(".")+2) + " seconds");
        else
            SetResult("Result", "Completed in " + duration + " seconds");
        SetActive(false);
    }

    void ProgressNotification::Failed(QString reason)
    {
        emit Completed();
        QString duration = QString::number(controller_->JobDurationInSeconds());
        if (duration.indexOf(".") != -1)
            SetResult("Failed at " + duration.left(duration.indexOf(".")+2) + "s", reason);
        else
            SetResult("Failed at " + duration + "s", reason);
        SetActive(false);
    }
}