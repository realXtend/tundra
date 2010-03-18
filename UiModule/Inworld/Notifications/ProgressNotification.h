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
        //! Set value from 0 to 100. If 100 will hide the notification immidiately.
        void SetValue(int value);

        //! Finish the progress job, this will hide the notification immidiately
        void Finish();

    signals:
        void StepUpdate(int value);
        void Finished();

    };

    class UI_MODULE_API ProgressNotification : public CoreUi::NotificationBaseWidget
    {

    Q_OBJECT

    public:
        ProgressNotification(QString message, ProgressController *controller, int hide_in_msec = 5000);
        virtual ~ProgressNotification();

    private slots:
        void Finished();

    private:
        ProgressController *controller_;
    };
}

#endif