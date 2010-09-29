// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ObjectCameraController.h"
#include "EventManager.h"
#include "InputEvents.h"
#include "SceneEvents.h"
#include "Renderer.h"
#include "Frame.h"
#include "Entity.h"
#include <QApplication>

#include <SceneManager.h>
#include <EC_OgrePlaceable.h>
#include <EC_OgreCamera.h>
#include <EC_OgreMesh.h>
#include <EC_OgreCustomObject.h>

#include <Ogre.h>


#include <QDebug>

namespace RexLogic
{
    ObjectCameraController::ObjectCameraController(RexLogicModule* rex_logic, CameraControllable* camera_controllable, QObject *parent) :
        QObject(parent),
        rex_logic_(rex_logic),
        framework_(rex_logic->GetFramework()),
        camera_controllable_(camera_controllable),
        camera_count_(0),
        last_dir_(Vector3df::ZERO),
        selected_entity_(0),
        alt_key_pressed_(false),
        zoom_close_(false),
        left_mousebutton_pressed_(false),
        object_selected_(false)
    {
         event_query_categories_ << "Input" << "Action" << "Framework" << "NetworkState" << "Scene";
         
         timeline_ = new QTimeLine(1000, this);
         timeline_->setFrameRange(0, 100);
    }

    ObjectCameraController::~ObjectCameraController()
    {
    }

    void ObjectCameraController::SubscribeToEventCategories()
    {
        service_category_identifiers_.clear();
        foreach (QString category, event_query_categories_)
            service_category_identifiers_[category] = framework_->GetEventManager()->QueryEventCategory(category.toStdString());
    }

    void ObjectCameraController::PostInitialize()
    {
        // Get all category id's that we are interested in
        SubscribeToEventCategories();

        // Register building key context
        input_context_ = framework_->GetInput()->RegisterInputContext("ObjectCameraContext", 100);
        connect(input_context_.get(), SIGNAL(KeyPressed(KeyEvent*)), this, SLOT(KeyPressed(KeyEvent*)));
        connect(input_context_.get(), SIGNAL(KeyReleased(KeyEvent*)), this, SLOT(KeyReleased(KeyEvent*)));
        connect(input_context_.get(), SIGNAL(MouseMove(MouseEvent*)), this, SLOT(MouseMove(MouseEvent*)));
        connect(input_context_.get(), SIGNAL(MouseLeftPressed(MouseEvent*)), this, SLOT(MouseLeftPressed(MouseEvent*)));
        connect(input_context_.get(), SIGNAL(MouseLeftReleased(MouseEvent*)), this, SLOT(MouseLeftReleased(MouseEvent*)));
        connect(input_context_.get(), SIGNAL(MouseScroll(MouseEvent*)), this, SLOT(MouseScroll(MouseEvent*)));
        connect(input_context_.get(), SIGNAL(MouseDoubleClicked(MouseEvent*)), this, SLOT(MouseDoubleClicked(MouseEvent*)));

        connect(framework_->GetFrame(), SIGNAL(Updated(float)), this, SLOT(Update(float)));
        connect(timeline_, SIGNAL(frameChanged(int)), this, SLOT(FrameChanged(int)));
        connect(timeline_, SIGNAL(finished()), this, SLOT(TimeLineFinished()));
    }

    bool ObjectCameraController::HandleInputEvent(event_id_t event_id, IEventData* data)
    {
        return false;
    }

    bool ObjectCameraController::HandleSceneEvent(event_id_t event_id, IEventData* data)
    {
        return false;
    }

    bool ObjectCameraController::HandleFrameworkEvent(event_id_t event_id, IEventData* data)
    {
        if (event_id == Foundation::WORLD_STREAM_READY)
        {
            CreateCustomCamera();
        }
        return false;
    }

    

    void ObjectCameraController::EntityClicked(Scene::Entity* entity)
    {
        if (!entity || !camera_entity_)
            return;
        if (timeline_->state() == QTimeLine::Running)
            return;

        if (alt_key_pressed_)
        {
            ec_camera_ = camera_entity_->GetComponent<OgreRenderer::EC_OgreCamera>().get();
            cam_ec_placable_ = camera_entity_->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();

            OgreRenderer::EC_OgrePlaceable *camera_placeable = 0;
            vectors_position_.first = Vector3df::ZERO;
            vectors_position_.second = Vector3df::ZERO;
            
            if (!selected_entity_)
            {
                cam_ec_placable_->SetPosition(camera_controllable_->GetCameraEntity()->GetComponent<OgreRenderer::EC_OgrePlaceable>().get()->GetPosition());         
                vectors_lookat_.first = camera_controllable_->GetThirdPersonLookAt();

                avatar_camera_position_ = cam_ec_placable_->GetPosition();
                avatar_camera_lookat_ = camera_controllable_->GetThirdPersonLookAt();
            } 
            else
            {   
                camera_placeable = selected_entity_->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
                vectors_lookat_.first = camera_placeable->GetPosition();
            }

            OgreRenderer::EC_OgrePlaceable *entity_ec_placable = entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
            if (entity_ec_placable)
            {
                cam_ec_placable_->LookAt(vectors_lookat_.first);
                ec_camera_->SetActive();

                object_selected_ = true;
                selected_entity_ = entity;
                
                QCursor *current_cur = QApplication::overrideCursor();
                if (current_cur)
                    if (current_cur->shape() != Qt::SizeAllCursor)
                        QApplication::setOverrideCursor(Qt::SizeAllCursor);
                                
                vectors_lookat_.second = entity_ec_placable->GetPosition();
                timeline_->start();
            }
        }
    }

    

    void ObjectCameraController::KeyPressed(KeyEvent* key_event)
    {
        if (key_event->keyCode == Qt::Key_Alt)
        {   
            alt_key_pressed_ = true;
            QApplication::setOverrideCursor(Qt::PointingHandCursor);
        } 

        if (key_event->keyCode == Qt::Key_Left || key_event->keyCode == Qt::Key_Right || key_event->keyCode == Qt::Key_Up || key_event->keyCode == Qt::Key_Down)
        {
            if (object_selected_)
            {
                if (zoom_close_)
                    return;
                ReturnToAvatarCamera();
            }

        }
    }

    void ObjectCameraController::MouseMove(MouseEvent* mouse)
    {
        if (object_selected_ && left_mousebutton_pressed_)
        {
            float width = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock()->GetWindowWidth();
            float height = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock()->GetWindowHeight();

            RotateObject(2*PI*mouse->relativeX/width, 2*PI*mouse->relativeY/height);
        }
    }

    void ObjectCameraController::MouseLeftPressed(MouseEvent* mouse)
    {
        left_mousebutton_pressed_ = true;
    }

    void ObjectCameraController::MouseLeftReleased(MouseEvent* mouse)
    {
        left_mousebutton_pressed_ = false;
    }

    void ObjectCameraController::KeyReleased(KeyEvent* key_event)
    {
        if (key_event->keyCode == Qt::Key_Alt)
        {   
            QCursor *current_cur = QApplication::overrideCursor();
            if (!object_selected_)
            {
                while (current_cur)
                {
                    QApplication::restoreOverrideCursor();
                    current_cur = QApplication::overrideCursor();
                }
            }
            else
            {
                if (current_cur)
                    if (current_cur->shape() == Qt::PointingHandCursor)
                        QApplication::restoreOverrideCursor();
            }
            alt_key_pressed_ = false;
        }  
    }

    void ObjectCameraController::MouseScroll(MouseEvent* mouse)
    {
        if (object_selected_)
        {
            if (! selected_entity_)
                return;
            OgreRenderer::EC_OgrePlaceable *entity_ec_placable = selected_entity_->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
            if (!entity_ec_placable)
                return;

            ZoomRelativeToPoint(entity_ec_placable->GetPosition(), mouse->RelativeZ());
        }
    }

    void ObjectCameraController::MouseDoubleClicked(MouseEvent *mouse)
    {
        
        if (object_selected_)
        {
            if (! selected_entity_)
                return;
            zoom_close_ = true;
        }
    }

    void ObjectCameraController::RotateObject(qreal x, qreal y)
    {
        if (selected_entity_)
        {
            OgreRenderer::EC_OgrePlaceable *entity_ec_placable = selected_entity_->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
            if (entity_ec_placable)
            {
                qreal acceleration_x = 1;
                qreal acceleration_y = 1;
                RotateCamera(entity_ec_placable->GetPosition(),selected_camera_id_,x*acceleration_x,y*acceleration_y);
            }
        }
    }

    void ObjectCameraController::RotateCamera(Vector3df pivot, CameraID id, qreal x, qreal y)
    {
        if (!ec_camera_ || !cam_ec_placable_)
            return;

        Ogre::Camera* cam = ec_camera_->GetCamera();
        Vector3df pos = cam_ec_placable_->GetPosition();

        Vector3df dir(pos-pivot);
        Quaternion quat(-x,cam_ec_placable_->GetLocalYAxis());
        quat *= Quaternion(-y, cam_ec_placable_->GetLocalXAxis());
        dir = quat * dir;

        Vector3df new_pos(pivot+dir);
        cam_ec_placable_->SetPosition(new_pos);
        cam_ec_placable_->LookAt(pivot);

        last_dir_=pivot - cam_ec_placable_->GetPosition();
    }

    void ObjectCameraController::CreateCustomCamera()
    {
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
            return;

        Scene::EntityPtr cam_entity = scene->CreateEntity(scene->GetNextFreeId());
        if (!cam_entity.get())
            return;

        cam_entity->AddComponent(framework_->GetComponentManager()->CreateComponent(OgreRenderer::EC_OgrePlaceable::TypeNameStatic()));
        cam_entity->AddComponent(framework_->GetComponentManager()->CreateComponent(OgreRenderer::EC_OgreCamera::TypeNameStatic()));
        scene->EmitEntityCreated(cam_entity);
        ComponentPtr component_placable = cam_entity->GetComponent(OgreRenderer::EC_OgrePlaceable::TypeNameStatic());
        OgreRenderer::EC_OgreCamera *ec_camera = cam_entity->GetComponent<OgreRenderer::EC_OgreCamera>().get();

        if (!component_placable.get() || !ec_camera)
            return;
        ec_camera->SetPlaceable(component_placable);

        camera_entity_ = cam_entity;
    }

    void ObjectCameraController::ZoomRelativeToPoint(Vector3df point, qreal delta, qreal min, qreal max)
    {
        bool zoomed = false;
        if (!camera_entity_)
            return;
        
        if (!cam_ec_placable_)
            return;

        Vector3df pos = cam_ec_placable_->GetPosition();       
        Vector3df dir = point-pos;
        Vector3df distance = dir;
        dir.normalize();
        qreal acceleration = 0.01;
        dir *= (delta * acceleration);

        if (delta>0 && (distance.getLength()+dir.getLength() > min))
        {
            zoomed = true;
        }
        if (delta<0 && (distance.getLength()+dir.getLength() <max))
        {
            zoomed = true;
        }
        if (zoomed)
        {
            cam_ec_placable_->SetPosition(cam_ec_placable_->GetPosition() + dir);
        }
    }

    void ObjectCameraController::Update(float frametime)
    {
        if(zoom_close_)
        {
            ZoomCloseToPoint(selected_entity_->GetComponent<OgreRenderer::EC_OgrePlaceable>().get()->GetPosition());
        }
    }

    void ObjectCameraController::FrameChanged(int frame)
    {
        qreal step = frame;
        step /= 100;

        if (vectors_position_.first != Vector3df::ZERO && vectors_position_.second != Vector3df::ZERO)
        {
            Vector3df position_start = vectors_position_.first;
            Vector3df position_end = vectors_position_.second;

            Vector3df position;
            position.x = (position_end.x - position_start.x) * step + position_start.x;
            position.y = (position_end.y - position_start.y) * step + position_start.y;
            position.z = (position_end.z - position_start.z) * step + position_start.z;
            cam_ec_placable_->SetPosition(position);
        }

        if (vectors_lookat_.first != Vector3df::ZERO && vectors_lookat_.second != Vector3df::ZERO)
        {
            Vector3df lookat_start = vectors_lookat_.first;
            Vector3df lookat_end = vectors_lookat_.second;

            Vector3df look_at;
            look_at.x = (lookat_end.x - lookat_start.x) * step + lookat_start.x;
            look_at.y = (lookat_end.y - lookat_start.y) * step + lookat_start.y;
            look_at.z = (lookat_end.z - lookat_start.z) * step + lookat_start.z;
            cam_ec_placable_->LookAt(look_at);

        } 

        
    }

    void ObjectCameraController::TimeLineFinished()
    {
        if (vectors_position_.first != Vector3df::ZERO && vectors_position_.second != Vector3df::ZERO)
        {
            camera_controllable_->GetCameraEntity()->GetComponent<OgreRenderer::EC_OgreCamera>().get()->SetActive();

            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            framework_->GetEventManager()->SendEvent(event_category, InputEvents::INPUTSTATE_THIRDPERSON, 0);

            object_selected_ = false;
            selected_entity_ = 0;

            QCursor *current_cur = QApplication::overrideCursor();
            while (current_cur)
            {
                QApplication::restoreOverrideCursor();
                current_cur = QApplication::overrideCursor();
            }
        }
    }

    void ObjectCameraController::ReturnToAvatarCamera()
    {
        OgreRenderer::EC_OgrePlaceable *cam_placable = camera_entity_->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
        if (!cam_placable)
            return;
        if (timeline_->state() == QTimeLine::Running)
            return;
        
        vectors_position_.first = cam_placable->GetPosition();
        vectors_position_.second = avatar_camera_position_;

        vectors_lookat_.first = vectors_lookat_.second;
        vectors_lookat_.second = avatar_camera_lookat_;
        timeline_->start();
    }

    void ObjectCameraController::ZoomCloseToPoint(Vector3df point, qreal delta, qreal min)
    {
        if (!camera_entity_)
            return;
        
        if (!cam_ec_placable_)
            return;

        Vector3df pos = cam_ec_placable_->GetPosition();       
        Vector3df dir = point-pos;
        Vector3df distance = dir;
        dir.normalize();
        qreal acceleration = 0.005;
        dir *= (delta * acceleration);

        if (distance.getLength()+dir.getLength() > min)
        {
            cam_ec_placable_->SetPosition(cam_ec_placable_->GetPosition() + dir);
        } else
        {
            zoom_close_ = false;
        }
        
    }
}
