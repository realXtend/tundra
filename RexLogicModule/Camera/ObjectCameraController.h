// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_ObjectCameraController_h
#define incl_RexLogic_ObjectCameraController_h

#include "InputEvents.h"
#include "ForwardDefines.h"
#include "Foundation.h"
#include "CameraControllable.h"
#include "InputServiceInterface.h"
#include "ModuleInterface.h"
#include "Renderer.h"
#include "RexLogicModule.h"

#include <EC_OgrePlaceable.h>
#include <EC_OgreCamera.h>


#include <QObject>
#include <QStringList>
#include <QMap>
#include <QTime>
#include <QPointF>
#include <QTimeLine>
#include <QPair>
#include "Vector3D.h"

namespace Foundation
{
    class Framework;
}

namespace RexLogic
{
    typedef int CameraID;
    typedef QPair<Vector3df, Vector3df> AnimVectors;

    class ObjectCameraController : public QObject
    {
    
    Q_OBJECT

    public:
        ObjectCameraController(RexLogicModule* rex_logic, CameraControllable* camera_controllable, QObject *parent = 0);
        ~ObjectCameraController();

        //! Input event handler for handling controllable events
        bool HandleInputEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        //! Scene event handler for handling controllable events
        bool HandleSceneEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        bool HandleFrameworkEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        void RotateObject(qreal x, qreal y);

        void RotateCamera(Vector3df pivot, CameraID id, qreal x, qreal y);

        void CreateCustomCamera();

        void ZoomRelativeToPoint(Vector3df point, qreal delta, qreal min=5, qreal max = 100);

        void ReturnToAvatarCamera();

        void ZoomCloseToPoint(Vector3df point, qreal delta = 100, qreal min = 5);


    public slots:
        void PostInitialize();
        void EntityClicked(Scene::Entity* entity);

        void KeyPressed(KeyEvent*);
        void KeyReleased(KeyEvent*);
        void MouseMove(MouseEvent *mouse);
        void MouseLeftPressed(MouseEvent *mouse);
        void MouseLeftReleased(MouseEvent *mouse);
        void MouseScroll(MouseEvent* mouse);
        void MouseDoubleClicked(MouseEvent *mouse);

        void FrameChanged(int);
        void TimeLineFinished();

        void Update(double);

    private slots:
        //! Populate service_category_identifiers_
        void SubscribeToEventCategories();  

    private:
        //! Current query categories
        QStringList event_query_categories_;

        //! Current subscribed category events
        QMap<QString, event_category_id_t> service_category_identifiers_;

        //! World building input context
        InputContextPtr input_context_;

        RexLogicModule* rex_logic_;
        Foundation::Framework* framework_;
        CameraControllable* camera_controllable_;

        bool alt_key_pressed_;
        bool left_mousebutton_pressed_;
        bool object_selected_;
        bool zoom_close_;

        QTime update_timer_;
        QPointF last_pos_;

        Scene::Entity *selected_entity_;

        CameraID camera_count_;
        QMap<CameraID, Scene::Entity*> id_to_cam_entity_;

        Vector3df last_dir_;
 
        CameraID selected_camera_id_;

        Scene::EntityPtr camera_entity_;
        OgreRenderer::EC_OgreCamera *ec_camera_;
        OgreRenderer::EC_OgrePlaceable *cam_ec_placable_;

        QTimeLine* timeline_;
        QTimeLine* timeline_zoom_;
        AnimVectors vectors_lookat_;
        AnimVectors vectors_position_;

        Vector3df avatar_camera_position_;
        Vector3df avatar_camera_lookat_;
   };
}
#endif
