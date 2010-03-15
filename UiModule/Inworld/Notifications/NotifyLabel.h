// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_NotifyLabel_h
#define incl_UiModule_NotifyLabel_h

#include <QGraphicsProxyWidget>
#include <ui_NotificationWidget.h>
#include <QPropertyAnimation>
#include <QTimer>

namespace CoreUi
{
    class NotifyLabel : public QGraphicsProxyWidget, private Ui::NotificationWidget
    {
    Q_OBJECT

    public:
        NotifyLabel(const QString &text, int duration_msec);
        virtual ~NotifyLabel();
    
    public slots:
        void TimeOut();
        void AnimationFinished();
        void ShowNotification();
        void CloseClicked();

    signals:
        void DestroyMe(CoreUi::NotifyLabel *me);

    private:
    QWidget *internal_widget_;
    QPropertyAnimation anim_;
    QTimer timer_;
    int current_time_;

    };
}

#endif // incl_UiModule_NotifyLabel_h