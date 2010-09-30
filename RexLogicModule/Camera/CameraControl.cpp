// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CameraControl.h"

namespace RexLogic
{
    CameraControl::CameraControl(QWidget *parent) :
        QWidget(parent)
    {
        setupUi(this);

        connect(cameraUP_button, SIGNAL(clicked()), this, SIGNAL(CameraUp()));
        connect(cameraDOWN_button, SIGNAL(clicked()), this, SIGNAL(CameraDown()));
        connect(cameraLEFT_button, SIGNAL(clicked()), this, SIGNAL(CameraLeft()));
        connect(cameraRIGHT_button, SIGNAL(clicked()), this, SIGNAL(CameraRight()));

        connect(moveFORWARD_button, SIGNAL(clicked()), this, SIGNAL(MoveForward()));
        connect(moveBACKWARD_button, SIGNAL(clicked()), this, SIGNAL(MoveBackward()));
        connect(moveLEFT_button, SIGNAL(clicked()), this, SIGNAL(MoveLeft()));
        connect(moveRIGHT_button, SIGNAL(clicked()), this, SIGNAL(MoveRight()));

        connect(zoomIN_button, SIGNAL(clicked()), this, SIGNAL(ZoomIn()));
        connect(zoomOUT_button, SIGNAL(clicked()), this, SIGNAL(ZoomOut()));

        connect(cameraTRIPOD_button, SIGNAL(toggled(bool)), this, SLOT(CameraTripod(bool)));
        connect(cameraFREE_button, SIGNAL(toggled(bool)), this, SLOT(CameraFree(bool)));
        connect(cameraFOCUS_button, SIGNAL(toggled(bool)), this, SLOT(CameraFocus(bool)));
        connect(cameraAVATAR_button, SIGNAL(toggled(bool)), this, SLOT(CameraAvatar(bool)));

        // Init styles for actions
        QString style_start = "QPushButton{border-image:url(./data/ui/images/cameracontrol/";
        QString style_end = ");}";
        release_style_= styleSheet();

        using namespace Input::Events;
        event_to_image_[MOVE_FORWARD_PRESSED] = style_start + "arrow_up_green1.png" + style_end;
        event_to_image_[MOVE_BACK_PRESSED] = style_start + "arrow_down_green1.png" + style_end;
        event_to_image_[MOVE_LEFT_PRESSED] = style_start + "arrow_left_green1.png" + style_end;
        event_to_image_[MOVE_RIGHT_PRESSED] = style_start + "arrow_right_green1.png" + style_end;
	}

	CameraControl::~CameraControl()
	{
	}

    void CameraControl::HandleInputEvent(event_id_t event_id, IEventData* data)
    {
        using namespace Input::Events;

        QPushButton *manipulate_button = 0;
        bool released_ = false;

        switch (event_id)
        {
            case MOVE_FORWARD_PRESSED:
                manipulate_button = moveFORWARD_button;
                break;
            case MOVE_FORWARD_RELEASED:
                manipulate_button = moveFORWARD_button;
                released_ = true;
                break;
            case MOVE_BACK_PRESSED:
                manipulate_button = moveBACKWARD_button;
                break;
            case MOVE_BACK_RELEASED:
                manipulate_button = moveBACKWARD_button;
                released_ = true;
                break;
            case MOVE_LEFT_PRESSED:
                manipulate_button = moveLEFT_button;
                break;
            case MOVE_LEFT_RELEASED:
                manipulate_button = moveLEFT_button;
                released_ = true;
                break;
            case MOVE_RIGHT_PRESSED:
                manipulate_button = moveRIGHT_button;
                break;
            case MOVE_RIGHT_RELEASED:
                manipulate_button = moveRIGHT_button;
                released_ = true;
            
            case INPUTSTATE_THIRDPERSON:
                cameraAVATAR_button->setChecked(true);
            default:
                break;
        }

        if (manipulate_button)
        {
            if (!released_ && event_to_image_.contains(event_id))
                manipulate_button->setStyleSheet(event_to_image_[event_id]);
            else
                manipulate_button->setStyleSheet(release_style_);
        }
    }

    void CameraControl::CameraTripod(bool checked)
    {
        if (checked)
        {
            emit CameraStateChange(Tripod);
        }
    }

    void CameraControl::CameraFree(bool checked)
    {
        if (checked)
        {
            emit CameraStateChange(Free);
        }
    }

    void CameraControl::CameraFocus(bool checked)
    {
        if (checked)
        {
            emit CameraStateChange(Object);
        }
    }   

    void CameraControl::CameraAvatar(bool checked)
    {
        if (checked)
        {
            emit CameraStateChange(Avatar);
        }
    }
}
