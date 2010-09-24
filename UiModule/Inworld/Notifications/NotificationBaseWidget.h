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
        NotificationBaseWidget(int hide_in_msec, const QString &message);
        
    public slots:
        //! Starts the notification timer, called by NotificationManager
        virtual void Start();

        //! Ends showing this notification in the scene, does animated hide and then emits Completed
        void Hide();

        //! Ends showing this notification in the scene, hides immidiately and emits Completed
        void HideNow();

        //! Animates the notification to a new position, called by NotificationManager
        void AnimateToPosition(const QPointF &end_pos);

        //! Getters
        bool IsActive() const { return is_active_; }
        QString GetMessage() const { return message_; }
        QString GetResultTitle() const { return result_title_; }
        QString GetResult() const { return result_; }
        QWidget *GetContentWidget() const { return content_widget_; }
        QDateTime GetTimeStamp() const { return timestamp_; }

        //! Setters
        void SetActive(bool active);
        void SetResult(QString title, QString result)  { result_title_ = title; result_ = result; }

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
        QWidget *content_widget_;
        QPropertyAnimation *fade_animation_;
        QPropertyAnimation *move_animation_;
        QPropertyAnimation *progress_animation_;
        QDateTime timestamp_;

        int hide_in_msec_;
        bool is_active_;
        
        QString message_;
        QString result_title_;
        QString result_;

    signals:
        void Completed(CoreUi::NotificationBaseWidget *self);
        void ResultsAreIn(QWidget *, QString, QString);
        void HideInteractionWidgets();

    };
}

#endif