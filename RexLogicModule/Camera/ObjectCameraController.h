// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_ObjectCameraController_h
#define incl_RexLogic_ObjectCameraController_h

#include "InputEvents.h"
#include "ForwardDefines.h"
#include "Foundation.h"
#include "CameraControllable.h"
#include "Input.h"
//#include "ModuleInterface.h"
#include "IModule.h"
#include "IEventData.h"
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
        bool HandleInputEvent(event_id_t event_id, IEventData* data);

        //! Scene event handler for handling controllable events
        bool HandleSceneEvent(event_id_t event_id, IEventData* data);

        bool HandleFrameworkEvent(event_id_t event_id, IEventData* data);

        bool HandleAvatarEvent(event_id_t event_id, IEventData* data);

        bool HandleNetworkStateEvent(event_id_t event_id, IEventData* data);

        void RotateObject(qreal x, qreal y);

        void ClampCamera(Vector3df obj_position, qreal distance);

        void RotateCamera(Vector3df pivot, qreal x, qreal y);

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

        void Update(float);

    private:
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
        bool avatar_edit_mode_;
        bool returning_to_avatar_;
        bool dont_accept_clicks_;

        QTime update_timer_;
        QPointF last_pos_;

        Scene::Entity *selected_entity_;

        Vector3df last_dir_;

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
