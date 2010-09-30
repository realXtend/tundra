// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_CameraControl_h
#define incl_RexLogic_CameraControl_h

#include <QWidget>
#include <QAbstractButton>

#include "InputEvents.h"
#include "ui_CameraControl.h"

namespace RexLogic
{
    class CameraControl : public QWidget, public Ui::CameraControl 
    {

    Q_OBJECT

    enum CameraMode
    {
        Tripod,
        Free,
        Object,
        Avatar,
        None
    };

    public:
        CameraControl(QWidget *parent = 0);
        ~CameraControl();

    public slots:
        void HandleInputEvent(event_id_t event_id, IEventData* data);

    private slots:
        void CameraTripod(bool);
        void CameraFree(bool);
        void CameraFocus(bool);
        void CameraAvatar(bool);

    signals:
        void CameraUp();
        void CameraDown();
        void CameraLeft();
        void CameraRight();

        void ZoomIn();
        void ZoomOut();

        void MoveForward();
        void MoveBackward();
        void MoveLeft();
        void MoveRight();

        void CameraStateChange(CameraMode mode);

    private:
        QString release_style_;
        QMap<event_id_t, QString> event_to_image_;
    };
}

#endif
