// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ProgressNotification_h
#define incl_UiModule_ProgressNotification_h

#include "NotificationBaseWidget.h"
#include "UiModuleApi.h"

class QTimeLine;

namespace UiServices
{
    class UI_MODULE_API ProgressController : public QObject
    {

    Q_OBJECT

    public:
        //! Use this to manually update values. No timers.
        ProgressController();

        //! Use this to have a linear time from start to finish.
        //! Step updates will be made automatically
        ProgressController(int linear_progress_length);

        //! Timeline in the second constructors case,
        //! public to give ProgressNotification access
        QTimeLine *progress_timeline;

    public slots:
        //! Start the internal timer with value, quivalent to calling SetValue 0
        void Start(int value = 0);

        //! Set value from 0 to 100. If 100 will hide the notification immidiately
        void SetValue(int value);

        //! Finish the progress job, this will hide the notification immidiately
        void Finish();

        void FailWithReason(QString reason);

        bool IsFinished() { return finished_; }
        qreal JobDurationInSeconds() { return job_duration_sec_; }

    signals:
        void Started();
        void Finished();
        void Failed(QString reason);

        void StepUpdate(int value);

    private:
        bool finished_;
        QTime job_timer_;
        qreal job_duration_sec_;

    };

    class UI_MODULE_API ProgressNotification : public CoreUi::NotificationBaseWidget
    {

    Q_OBJECT

    public:
        ProgressNotification(QString message, ProgressController *controller, int hide_in_msec = 5000);
        virtual ~ProgressNotification();

    private slots:
        void Finished();
        void Failed(QString reason);

    private:
        ProgressController *controller_;

    signals:
        void Completed();

    };
}

#endif