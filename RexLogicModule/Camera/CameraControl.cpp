// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CameraControl.h"
#include "EventManager.h"
#include <QKeyEvent>
#include <QApplication>
#include <QInputContext>
#include <QGraphicsView>
#include "InputContext.h"
#include "ObjectCameraController.h"
#include "UiServiceInterface.h"
#include "Frame.h"

namespace RexLogic
{
    RotationWidget::RotationWidget(QWidget *parent) :
        QWidget(parent)
    {
        setupUi(this);
    }

    MovementWidget::MovementWidget(QWidget *parent) : 
        QWidget(parent)
    {
        setupUi(this);

        // Init styles for actions
        QString style_start = "QPushButton{border-image:url(./data/ui/images/cameracontrol/";
        QString style_end = ");}";
        release_style_= styleSheet();

        using namespace InputEvents;
        event_to_image_[MOVE_FORWARD_PRESSED] = style_start + "arrow_up_green1.png" + style_end;
        event_to_image_[MOVE_BACK_PRESSED] = style_start + "arrow_down_green1.png" + style_end;
        event_to_image_[MOVE_LEFT_PRESSED] = style_start + "arrow_left_green1.png" + style_end;
        event_to_image_[MOVE_RIGHT_PRESSED] = style_start + "arrow_right_green1.png" + style_end;
    }

    ZoomWidget::ZoomWidget(QWidget *parent) : 
        QWidget(parent)
    {
        setupUi(this);

        connect(zoomIN_button, SIGNAL(clicked()), this, SIGNAL(ZoomIn()));
        connect(zoomOUT_button, SIGNAL(clicked()), this, SIGNAL(ZoomOut()));
    }

    CameraControl::CameraControl(RexLogicModule *rex_logic, QWidget *parent) :
        rex_logic_(rex_logic),
        framework_(rex_logic->GetFramework()),
        QWidget(parent),
        widget_state_(None),
        camera_up_pressed_(false),
        camera_down_pressed_(false),
        camera_left_pressed_(false),
        camera_right_pressed_(false),
        zoom_in_pressed_(false),
        zoom_out_pressed_(false)
    {
        setupUi(this);
        using namespace InputEvents;

        rotation_widget = new RotationWidget(this);
        horizontalLayout_9->addWidget(rotation_widget);
        rotation_widget->hide();
        
        movement_widget = new MovementWidget(this);
        horizontalLayout_8->addWidget(movement_widget);
        movement_widget->hide();

        zoom_widget = new ZoomWidget(this);
        horizontalLayout_10->addWidget(zoom_widget);
        zoom_widget->hide();

        connect(cameraTRIPOD_button, SIGNAL(toggled(bool)), this, SLOT(CameraTripod(bool)));
        connect(cameraFREE_button, SIGNAL(toggled(bool)), this, SLOT(CameraFree(bool)));
        connect(cameraFOCUS_button, SIGNAL(toggled(bool)), this, SLOT(CameraFocus(bool)));
        connect(cameraAVATAR_button, SIGNAL(toggled(bool)), this, SLOT(CameraAvatar(bool)));

        connect(movement_widget->moveFORWARD_button, SIGNAL(pressed()), this, SLOT(MoveForwardPressed()));
        connect(movement_widget->moveFORWARD_button, SIGNAL(released()), this, SLOT(MoveForwardReleased()));
        connect(movement_widget->moveBACKWARD_button, SIGNAL(pressed()), this, SLOT(MoveBackwardPressed()));
        connect(movement_widget->moveBACKWARD_button, SIGNAL(released()), this, SLOT(MoveBackwardReleased()));
        connect(movement_widget->moveRIGHT_button, SIGNAL(pressed()), this, SLOT(MoveRightPressed()));
        connect(movement_widget->moveRIGHT_button, SIGNAL(released()), this, SLOT(MoveRightReleased()));
        connect(movement_widget->moveLEFT_button, SIGNAL(pressed()), this, SLOT(MoveLeftPressed()));
        connect(movement_widget->moveLEFT_button, SIGNAL(released()), this, SLOT(MoveLeftReleased()));

        connect(rotation_widget->cameraUP_button, SIGNAL(pressed()), this, SLOT(CameraUpPressed()));
        connect(rotation_widget->cameraUP_button, SIGNAL(released()), this, SLOT(CameraUpReleased()));
        connect(rotation_widget->cameraDOWN_button, SIGNAL(pressed()), this, SLOT(CameraDownPressed()));
        connect(rotation_widget->cameraDOWN_button, SIGNAL(released()), this, SLOT(CameraDownReleased()));
        connect(rotation_widget->cameraRIGHT_button, SIGNAL(pressed()), this, SLOT(CameraRightPressed()));
        connect(rotation_widget->cameraRIGHT_button, SIGNAL(released()), this, SLOT(CameraRightReleased()));
        connect(rotation_widget->cameraLEFT_button, SIGNAL(pressed()), this, SLOT(CameraLeftPressed()));
        connect(rotation_widget->cameraLEFT_button, SIGNAL(released()), this, SLOT(CameraLeftReleased()));

        connect(zoom_widget->zoomIN_button, SIGNAL(pressed()), this, SLOT(ZoomInPressed()));
        connect(zoom_widget->zoomIN_button, SIGNAL(released()), this, SLOT(ZoomInReleased()));
        connect(zoom_widget->zoomOUT_button, SIGNAL(pressed()), this, SLOT(ZoomOutPressed()));
        connect(zoom_widget->zoomOUT_button, SIGNAL(released()), this, SLOT(ZoomOutReleased()));

        connect(framework_->GetFrame(), SIGNAL(Updated(float)), this, SLOT(Update(float)));

        UiServiceInterface *ui_service = framework_->GetService<UiServiceInterface>();
        if (ui_service)
		{
			proxy_widget_ = ui_service->AddWidgetToScene(this);
            proxy_widget_->setMaximumHeight(800);
            proxy_widget_->setMaximumWidth(600);
			ui_service->AddWidgetToMenu(this, tr("Camera Controls"), "", "./data/ui/images/menus/edbutton_WRLDTOOLS_icon.png");
		}
	}

    void CameraControl::CameraTripod(bool checked)
    {
        if (checked)
        {
            movement_widget->show();
            rotation_widget->hide();
            zoom_widget->hide();
            emit CameraStateChange(Tripod);
        }
    }

    void CameraControl::CameraFree(bool checked)
    {
        if (checked)
        {
            movement_widget->show();
            rotation_widget->show();
            zoom_widget->hide();
            emit CameraStateChange(Free);
        }
    }

    void CameraControl::CameraFocus(bool checked)
    {
        if (checked)
        {
            movement_widget->hide();
            rotation_widget->show();
            zoom_widget->show();
            emit CameraStateChange(Object);
        }
    }

    void CameraControl::CameraAvatar(bool checked)
    {
        if (checked)
        {
            movement_widget->show();
            rotation_widget->show();
            zoom_widget->show();
            emit CameraStateChange(Avatar);
        }
    }

    void CameraControl::CameraStateChange(CameraMode camera_mode)
    {
        if (camera_mode == Tripod)
        {
            switch (widget_state_)
            {
                case Avatar:
                    break;
                case Free:
                    {
                        event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
                        framework_->GetEventManager()->SendEvent(event_category, InputEvents::INPUTSTATE_FREECAMERA, 0);
                        framework_->GetEventManager()->SendEvent(event_category, InputEvents::INPUTSTATE_THIRDPERSON, 0);
                    }
                    break;
                case Object:
                    {
                        AltKeyReleased();
                        rex_logic_->GetObjectCameraController()->ReturnToAvatarCamera();
                    }
                    break;
            }
            ModifyRotationWidget(camera_mode);
            widget_state_ = camera_mode;
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::INPUTSTATE_CAMERATRIPOD, 0);
        }
        if (camera_mode == Avatar)
        {
            switch (widget_state_)
            {
                case Tripod:
                    break;
                case Free:
                    {
                        event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
                        framework_->GetEventManager()->SendEvent(event_category, InputEvents::INPUTSTATE_FREECAMERA, 0);
                    }
                    break;
                case Object:
                    {
                        AltKeyReleased();
                        rex_logic_->GetObjectCameraController()->ReturnToAvatarCamera();
                    }
                    break;
            }
            ModifyRotationWidget(camera_mode);
            widget_state_ = camera_mode;
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::INPUTSTATE_THIRDPERSON, 0);
        }
        if (camera_mode == Free)
        {
            switch (widget_state_)
            {
                case Tripod:
                    {
                        event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
                        framework_->GetEventManager()->SendEvent(event_category, InputEvents::INPUTSTATE_CAMERATRIPOD, 0);
                    }
                    break;
                case Avatar:
                    break;
                case Object:
                    {
                        AltKeyReleased();
                        rex_logic_->GetObjectCameraController()->ReturnToAvatarCamera();
                    }
                    break;
            }
            ModifyRotationWidget(camera_mode);
            widget_state_ = camera_mode;
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::INPUTSTATE_FREECAMERA, 0);
        }
        if (camera_mode == Object)
        {
            ModifyRotationWidget(camera_mode);
            widget_state_ = Object;
            AltKeyPressed();
        }
    }

    void CameraControl::MoveForwardPressed()
    {
        if (widget_state_ != None)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOVE_FORWARD_PRESSED, 0);
        }
    }

    void CameraControl::MoveForwardReleased()
    {
        if (widget_state_ != None)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOVE_FORWARD_RELEASED, 0);
        }
    }

    void CameraControl::MoveBackwardPressed()
    {
        if (widget_state_ != None)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOVE_BACK_PRESSED, 0);
        }
    }

    void CameraControl::MoveBackwardReleased()
    {
        if (widget_state_ != None)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOVE_BACK_RELEASED, 0);
        }
    }

    void CameraControl::MoveRightPressed()
    {
        if (widget_state_ != None)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOVE_RIGHT_PRESSED, 0);
        }
    }

    void CameraControl::MoveRightReleased()
    {
        if (widget_state_ != None)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOVE_RIGHT_RELEASED, 0);
        }
    }

    void CameraControl::MoveLeftPressed()
    {
        if (widget_state_ != None)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOVE_LEFT_PRESSED, 0);
        }
    }

    void CameraControl::MoveLeftReleased()
    {
        if (widget_state_ != None)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOVE_LEFT_RELEASED, 0);
        }
    }

    void CameraControl::ZoomInPressed()
    {
        if (widget_state_ == Free)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOVE_FORWARD_PRESSED, 0);
        }
        if (widget_state_ == Object || widget_state_ == Avatar)
        {
            zoom_in_pressed_ = true;
        }
    }

    void CameraControl::ZoomInReleased()
    {
        if (widget_state_ == Avatar)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::ZOOM_IN_RELEASED, 0);
            zoom_in_pressed_ = false;
        }
        if (widget_state_ == Free)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOVE_FORWARD_RELEASED, 0);
        }
        if (widget_state_ == Object)
        {
            zoom_in_pressed_ = false;
        }
    }

    void CameraControl::ZoomOutPressed()
    {
        if (widget_state_ == Free)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOVE_BACK_PRESSED, 0);
        }
        if (widget_state_ == Object || widget_state_ == Avatar)
        {
            zoom_out_pressed_ = true;
        }
    }

    void CameraControl::ZoomOutReleased()
    {
        if (widget_state_ == Avatar)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::ZOOM_OUT_RELEASED, 0);
            zoom_out_pressed_ = false;
        }
        if (widget_state_ == Free)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOVE_BACK_RELEASED, 0);
        }
        if (widget_state_ == Object)
        {
            zoom_out_pressed_ = false;
        }
    }

    void CameraControl::CameraUpPressed()
    {
        if (widget_state_ == Object || widget_state_ == Free)
        {
            camera_up_pressed_ = true;
        }
    }

    void CameraControl::CameraUpReleased()
    {
        if (widget_state_ == Object || widget_state_ == Free)
        {
            camera_up_pressed_ = false;
        }
    }

    void CameraControl::CameraDownPressed()
    {
        if (widget_state_ == Object || widget_state_ == Free)
        {
            camera_down_pressed_ = true;
        }
    }

    void CameraControl::CameraDownReleased()
    {
        if (widget_state_ == Object || widget_state_ == Free)
        {
            camera_down_pressed_ = false;
        }
    }

    void CameraControl::CameraRightPressed()
    {
        if (widget_state_ == Avatar)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::ROTATE_RIGHT_PRESSED, 0);
        }
        if (widget_state_ == Object || widget_state_ == Free)
        {
            camera_right_pressed_ = true;
        }
    }

    void CameraControl::CameraRightReleased()
    {
        if (widget_state_ == Avatar)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::ROTATE_RIGHT_RELEASED, 0);
        }
        if (widget_state_ == Object || widget_state_ == Free)
        {
            camera_right_pressed_ = false;
        }
    }

    void CameraControl::CameraLeftPressed()
    {
        if (widget_state_ == Avatar)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::ROTATE_LEFT_PRESSED, 0);
        }
        if (widget_state_ == Object || widget_state_ == Free)
        {
            camera_left_pressed_ = true;
        }
    }

    void CameraControl::CameraLeftReleased()
    {
        if (widget_state_ == Avatar)
        {
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::ROTATE_LEFT_RELEASED, 0);
        }
        if (widget_state_ == Object || widget_state_ == Free)
        {
            camera_left_pressed_ = false;
        }
    }   

    void CameraControl::AltKeyPressed()
    {
        KeyEvent alt_press_event;
        alt_press_event.keyCode = Qt::Key_Alt;
        alt_press_event.modifiers = Qt::NoModifier;
        alt_press_event.eventType = KeyEvent::KeyPressed;
        rex_logic_->GetObjectCameraController().get()->KeyPressed(&alt_press_event);
        QApplication::activeWindow()->setFocus();
    }

    void CameraControl::AltKeyReleased()
    {
        KeyEvent alt_rel_event;
        alt_rel_event.keyCode = Qt::Key_Alt;
        alt_rel_event.modifiers = Qt::NoModifier;
        alt_rel_event.eventType = KeyEvent::KeyReleased;
        rex_logic_->GetObjectCameraController().get()->KeyReleased(&alt_rel_event);
    }

    void CameraControl::ModifyRotationWidget(CameraMode camera_mode)
    {
        if (camera_mode == Avatar)
        {
            rotation_widget->cameraUP_button->setEnabled(false);
            rotation_widget->cameraUP_button->setVisible(false);
            rotation_widget->cameraDOWN_button->setEnabled(false);
            rotation_widget->cameraDOWN_button->setVisible(false);
        } else 
        {
            rotation_widget->cameraUP_button->setEnabled(true);
            rotation_widget->cameraUP_button->setVisible(true);
            rotation_widget->cameraDOWN_button->setEnabled(true);
            rotation_widget->cameraDOWN_button->setVisible(true);
        }
    }

    void CameraControl::Update(float frametime)
    {
        if (widget_state_ == Object)
        {
            if (camera_up_pressed_)
            {
                MouseEvent mouse_event;
                mouse_event.button = MouseEvent::LeftButton;
                mouse_event.relativeX = 0;
                mouse_event.relativeY = -5;
                rex_logic_->GetObjectCameraController().get()->MouseLeftPressed(&mouse_event);
                rex_logic_->GetObjectCameraController().get()->MouseMove(&mouse_event);
            }
            if (camera_down_pressed_)
            {
                MouseEvent mouse_event;
                mouse_event.button = MouseEvent::LeftButton;
                mouse_event.relativeX = 0;
                mouse_event.relativeY = 5;
                rex_logic_->GetObjectCameraController().get()->MouseLeftPressed(&mouse_event);
                rex_logic_->GetObjectCameraController().get()->MouseMove(&mouse_event);
            }
            if (camera_left_pressed_)
            {
                MouseEvent mouse_event;
                mouse_event.eventType = MouseEvent::MousePressed;
                mouse_event.button = MouseEvent::LeftButton;
                mouse_event.relativeX = -5;
                mouse_event.relativeY = 0;
                rex_logic_->GetObjectCameraController().get()->MouseLeftPressed(&mouse_event);
                rex_logic_->GetObjectCameraController().get()->MouseMove(&mouse_event);
            }
            if (camera_right_pressed_)
            {
                MouseEvent mouse_event;
                mouse_event.button = MouseEvent::LeftButton;
                mouse_event.relativeX = 5;
                mouse_event.relativeY = 0;
                rex_logic_->GetObjectCameraController().get()->MouseLeftPressed(&mouse_event);
                rex_logic_->GetObjectCameraController().get()->MouseMove(&mouse_event);
            }
            if (zoom_in_pressed_)
            {
                MouseEvent mouse_event;
                mouse_event.relativeZ = 30;
                rex_logic_->GetObjectCameraController().get()->MouseScroll(&mouse_event);
            }
            if (zoom_out_pressed_)
            {
                MouseEvent mouse_event;
                mouse_event.relativeZ = -30;
                rex_logic_->GetObjectCameraController().get()->MouseScroll(&mouse_event);
            }
        }
        if (widget_state_ == Free)
        {
            if (camera_up_pressed_)
            {
                InputEvents::Movement movement;
                movement.x_.abs_ = 0;
                movement.y_.abs_ = 0;
                movement.z_.abs_ = 0;
                movement.x_.rel_ = 0;
                movement.y_.rel_ = -5;
                movement.z_.rel_ = -1;
                event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
                framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOUSELOOK, &movement);
            }
            if (camera_down_pressed_)
            {
                InputEvents::Movement movement;
                movement.x_.abs_ = 0;
                movement.y_.abs_ = 0;
                movement.z_.abs_ = 0;
                movement.x_.rel_ = 0;
                movement.y_.rel_ = 5;
                movement.z_.rel_ = -1;
                event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
                framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOUSELOOK, &movement);
            }
            if (camera_left_pressed_)
            {
                InputEvents::Movement movement;
                movement.x_.abs_ = 0;
                movement.y_.abs_ = 0;
                movement.z_.abs_ = 0;
                movement.x_.rel_ = -5;
                movement.y_.rel_ = 0;
                movement.z_.rel_ = -1;
                event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
                framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOUSELOOK, &movement);
            }
            if (camera_right_pressed_)
            {
                InputEvents::Movement movement;
                movement.x_.abs_ = 0;
                movement.y_.abs_ = 0;
                movement.z_.abs_ = 0;
                movement.x_.rel_ = 5;
                movement.y_.rel_ = 0;
                movement.z_.rel_ = -1;
                event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
                framework_->GetEventManager()->SendEvent(event_category, InputEvents::MOUSELOOK, &movement);
            }
        }
        if (widget_state_ == Avatar)
        {
            if (zoom_in_pressed_)
            {
                event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
                framework_->GetEventManager()->SendEvent(event_category, InputEvents::ZOOM_IN_PRESSED, 0);
            }
            if (zoom_out_pressed_)
            {
                event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
                framework_->GetEventManager()->SendEvent(event_category, InputEvents::ZOOM_OUT_PRESSED, 0);
            }
        }
    }
}