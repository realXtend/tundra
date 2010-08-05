// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_CameraHandler_h
#define incl_WorldBuildingModule_CameraHandler_h

#include "Foundation.h"
#include "Entity.h"
#include "Vector3D.h"

#include <QObject>
#include <QPixmap>
#include <QSize>
#include <QMap>

#include <OgrePrerequisites.h>

namespace WorldBuilding
{
    namespace View
    {
        typedef int CameraID;

        class CameraHandler : public QObject
        {

        Q_OBJECT

        public:
            CameraHandler(Foundation::Framework *framework, QObject *parent = 0);
            virtual ~CameraHandler();

        public slots:
            CameraID CreateCustomCamera();
            void DestroyCamera(CameraID cam_id);

            void RotateCamera(Vector3df pivot,CameraID id, qreal x, qreal y);
            bool ZoomRelativeToPoint(Vector3df point, CameraID id,qreal delta, qreal min=5, qreal max = 100);

            bool FocusToEntity(CameraID cam_id, Scene::Entity *entity, Vector3df offset = Vector3df(-0.5f, -0.5f, -0.5f));
            QPixmap RenderCamera(CameraID cam_id, QSize image_size);

        private:
            Foundation::Framework *framework_;

            CameraID camera_count_;
            QMap<CameraID, Scene::Entity*> id_to_cam_entity_;

            std::string render_texture_name_;
            Ogre::uchar *pixelData_;

            Vector3df last_dir_;
        };
    }
}

#endif