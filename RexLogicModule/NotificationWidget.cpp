// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NotificationWidget.h"
#include "NaaliUI.h"
#include "Framework.h"
#include "Renderer.h"

namespace RexLogic
{
    NotificationWidget::NotificationWidget(RexLogicModule *rex_logic, QWidget *parent) :
        rex_logic_(rex_logic),
        framework_(rex_logic->GetFramework()),
        QGraphicsView(parent)
    {
        scene_ = framework_->Ui()->GraphicsScene();
        setScene(scene_);

        time_line_ = new QTimeLine(3000, this);

        frame_pic_ = new QFrame;
        scene_->addWidget(frame_pic_);
        frame_pic_->setFixedSize(100, 100);
        frame_pic_->hide();

        start_ = "QFrame {background-color: transparent;"
                         "border-image: url(./data/ui/images/notificationwidget/";
        end_ = ")}";
    }

    void NotificationWidget::SetupScene()
    {
        float width = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock()->GetWindowWidth();
        float height = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock()->GetWindowHeight();
        frame_pic_->move(width - frame_pic_->width() - 10, 40);
        
        QString style_sheet_ = start_ + picture_name_ + end_;
        
        frame_pic_->show();
        frame_pic_->setStyleSheet(style_sheet_);
        frame_pic_->setWindowOpacity(1.0);

        time_line_->setFrameRange(0, 100);
        connect(time_line_, SIGNAL(frameChanged(int)), this, SLOT(updateStep(int)));
        time_line_->start();

    }

    void NotificationWidget::updateStep(int i)
    {
      frame_pic_->setWindowOpacity((100-i)/100.0);
    }

    void NotificationWidget::FocusOnObject()
    {
        picture_name_ = "camera_focus.png" ;
        SetupScene();
    }

    bool NotificationWidget::HandleInputEvent(event_id_t event_id, IEventData* data)
    {
        if (event_id == InputEvents::CAMERA_TRIPOD)
        {
            picture_name_ = "tripod_camera.png" ;
            SetupScene();
        }
        else if (event_id == InputEvents::INPUTSTATE_FREECAMERA)
        {
            picture_name_ = "free_camera.png" ;
            SetupScene();
        }
        else if (event_id == InputEvents::INPUTSTATE_THIRDPERSON)
        {
            picture_name_ = "third_person.png";
            SetupScene();
        }
        else if (event_id == InputEvents::INPUTSTATE_FOCUSONOBJECT)
        {
            picture_name_ = "camera_focus_grey.png";
            SetupScene();
        }
        return false;
    }
}


