// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_NotificationBaseWidget_h
#define incl_UiModule_NotificationBaseWidget_h

#include "UiModuleApi.h"

#include <QGraphicsProxyWidget>
#include <QGraphicsSceneHoverEvent>
#include <QDateTime>

#include "ui_NotificationBaseWidget.h"

class QTimer;
class QPropertyAnimation;

namespace CoreUi
{
    class UI_MODULE_API NotificationBaseWidget : public QGraphicsProxyWidget, protected Ui::NotificationBaseWidget
    {
    
    Q_OBJECT

    public:
        NotificationBaseWidget(int hide_in_msec);
        
    public slots:
        //! Starts the notification timer, called by NotificationManager
        virtual void Start();

        //! Ends showing this notification in the scene
        void Hide();

        //! Animates the notification to a new position, called by NotificationManager
        void AnimateToPosition(QPointF end_pos);

        //! Getters/setters
        QDateTime GetTimeStamp()        { return timestamp_; }
        void SetActive(bool active)     { is_active_ = active; }
        bool IsActive()                 { return is_active_; }
        void SetResult(QString result)  { result_ = result; }
        QString GetResult()             { return result_; }
        QWidget *GetContentWidget()     { return contentWidget; }

    protected:
        //! Sets the content widget, called by subclasses
        void SetCentralWidget(QWidget *widget);

        //! Sets the content layout, called by subclasses
        void SetCentralLayout(QLayout *layout);

        //! Pauses the hide timer when mouse enters notification rect
        void hoverEnterEvent(QGraphicsSceneHoverEvent *hover_enter_event);

        //! Resumes the hide timer when mouse leaves notification rect
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *hover_leave_event);

    protected slots:
        void InitSelf();
        void TimedOut();
        void WidgetHidden();

    private:
        QWidget *internal_widget_;
        QPropertyAnimation *fade_animation_;
        QPropertyAnimation *move_animation_;
        QPropertyAnimation *progress_animation_;
        QDateTime timestamp_;

        int hide_in_msec_;
        bool is_active_;
        QString result_;

    signals:
        void Completed(CoreUi::NotificationBaseWidget *self);

    };
}

#endif