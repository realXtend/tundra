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
    class UI_MODULE_API NotificationBaseWidget : public QGraphicsProxyWidget, private Ui::NotificationBaseWidget
    {
    
    Q_OBJECT

    public:
        NotificationBaseWidget(int hide_in_msec);
        
    public slots:
        void Start();
        void AnimateToPosition(QPointF end_pos);

        QDateTime GetTimeStamp() { return timestamp_; }

    protected:
        void SetCentralWidget(QWidget *widget);

        void hoverEnterEvent(QGraphicsSceneHoverEvent *hover_enter_event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *hover_leave_event);

    private slots:
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

    signals:
        void Completed(CoreUi::NotificationBaseWidget *self);

    };
}

#endif