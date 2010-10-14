// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_CameraControl_h
#define incl_RexLogic_CameraControl_h

#include <QWidget>
#include <QAbstractButton>

#include "InputEvents.h"
#include "IEventData.h"
#include "ui_CameraControl.h"
#include "ui_RotationWidget.h"
#include "ui_MovementWidget.h"
#include "ui_ZoomWidget.h"

#include "RexLogicModule.h"
#include "CameraControllable.h"
#include "Framework.h"
#include "UiProxyWidget.h"

namespace RexLogic
{
    enum CameraMode
        {
            Tripod,
            Free,
            Object,
            Avatar,
            None
        };
    class RotationWidget : public QWidget, public Ui::RotationWidget
    {
        Q_OBJECT
    
    public:
        RotationWidget(QWidget *parent = 0);

    signals:
        void CameraUp();
        void CameraDown();
        void CameraLeft();
        void CameraRight();
    };

    class MovementWidget : public QWidget, public Ui::MovementWidget
    {
        Q_OBJECT
    
    public:
        MovementWidget(QWidget *parent = 0);
        QString release_style_;
        QMap<event_id_t, QString> event_to_image_;

    signals:
        void MoveForward();
        void MoveBackward();
        void MoveLeft();
        void MoveRight();
        void StateChanged(CameraMode camera_mode);
    };

    class ZoomWidget : public QWidget, public Ui::ZoomWidget
    {
        Q_OBJECT
    
    public:
        ZoomWidget(QWidget *parent = 0);
    
    signals:
        void ZoomIn();
        void ZoomOut();
    };

    class CameraControl : public QWidget, public Ui::CameraControl
    {
        Q_OBJECT
    
    public:
        CameraControl(RexLogicModule *rex_logic, QWidget *parent = 0);

        RotationWidget* GetRotationWidget() { return rotation_widget; }
        MovementWidget* GetMovementWidget() { return movement_widget; }
        ZoomWidget* GetZoomWidget() { return zoom_widget; }

        void AltKeyPressed();
        void AltKeyReleased();
        void ModifyRotationWidget(CameraMode camera_mode);

    public slots:
        void CameraStateChange(CameraMode camera_mode);
        
        void MoveForwardPressed();
        void MoveForwardReleased();
        void MoveBackwardPressed();
        void MoveBackwardReleased();
        void MoveRightPressed();
        void MoveRightReleased();
        void MoveLeftPressed();
        void MoveLeftReleased();

        void CameraUpPressed();
        void CameraUpReleased();
        void CameraDownPressed();
        void CameraDownReleased();
        void CameraRightPressed();
        void CameraRightReleased();
        void CameraLeftPressed();
        void CameraLeftReleased();

        void ZoomInPressed();
        void ZoomInReleased();
        void ZoomOutPressed();
        void ZoomOutReleased();

        void Update(float);

    private slots:
        void CameraTripod(bool);
        void CameraFree(bool);
        void CameraFocus(bool);
        void CameraAvatar(bool);
        
    private:
        CameraMode camera_mode_;
        RotationWidget *rotation_widget;
        MovementWidget *movement_widget;
        ZoomWidget *zoom_widget;
        
        RexLogicModule* rex_logic_;
        Foundation::Framework* framework_;
        CameraControllable* camera_controllable_;
        
        CameraMode widget_state_;

        bool camera_up_pressed_;
        bool camera_down_pressed_;
        bool camera_left_pressed_;
        bool camera_right_pressed_;
        bool zoom_in_pressed_;
        bool zoom_out_pressed_;

        UiProxyWidget* proxy_widget_;
    };
}

#endif
