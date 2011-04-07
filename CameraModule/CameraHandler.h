//$ HEADER_NEW_FILE $
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   CameraHandler.h
 *  @brief  Camera handler creates new world entity with ec_ogrecamera, ec_placeable and ec_rtttarget componets. Manages zoom and movement of the camera depending on the camera type
 *			
 */

#ifndef incl_CameraModule_CameraHandler_h
#define incl_CameraModule_CameraHandler_h

#include "Foundation.h"
#include "Entity.h"
#include "Vector3D.h"

#include <QObject>
#include <QPixmap>
#include <QSize>
#include <QMap>

#include <Ogre.h>
#include <OgrePrerequisites.h>


namespace Camera
{
    class CameraHandler : public QObject
    {

        Q_OBJECT
        Q_ENUMS(CameraType)

        
    public:

        //! Camera types
        enum CameraType
        {
            Perspective = 0,
            Front,
            Back,
            Left,
            Rigth,
            Top,
            Bottom
        };

        /*! Constructor. Creates ogre texture
         * \param framework Framework
         * \param parent Parent object
         */
        CameraHandler(Foundation::Framework *framework, QObject *parent = 0);

        //! Destructor.
        virtual ~CameraHandler();

    public slots:
        //! Destroys ogre texture.         
        void DestroyCamera();

        /*! Creates new world entity with ec_ogrecamera and ec_placeable components
         * \param scene XX
         */
        bool CreateCamera(Scene::SceneManager *scene);

        /*! Moves the camera.
         * \param x X movement. [-1,1]
         * \param y Y movement. [-1,1]
         */
        void Move(qreal x, qreal y);

        /*! Makes zoom to the camera.
         * \param delta mouse delta parameter
         * \param modifiers key modifiers
         */
        bool Zoom(qreal delta, Qt::KeyboardModifiers modifiers);

        /*! Focus the camera to world entity
         * \param entity World target entity
         */
        bool FocusToEntity(Scene::Entity *entity);

        /*! Gets QPixmap from ogre texture
         * \param imange_size Size of the image
         */
        QPixmap RenderCamera(QSize image_size);

        /*! Set Camera type 
         * \param camera_type CameraType enum
         */
        void SetCameraType(int camera_type);

        /*! Set projection type 
         * \param projection Ogre::ProjectionType: PT_ORTHOGRAPHIC 0, PT_PERSPECTIVE 1
         */
        void SetCameraProjection(int projection);

        /*! Enable or disable wireframe polygon mode
         * \param state enable (true) or disable (false) wireframe polygon mode renderer
         */
        void SetCameraWireframe(bool state);

        /*! Set Camera polygon mode 
         * \param polygon_mode Ogre::PolygonMode: PM_POINTS = 1, PM_WIREFRAME = 2, PM_SOLID = 3
         */
        void SetCameraPolygonMode(Ogre::PolygonMode polygon_mode);

        //! Get camera type. CameraType enum
        int GetCameraType(){ return camera_type_;};

        //! Get projection type. Perpective or orthographic
        int GetProjectionType() { return projection_type_;};

        //! Return if wireframe polygon mode is enabled or not
        bool IsWireframeEnabled();

        /*! Set camera near clip distance
         * \param distance camera near clip distance
         */
        void SetNearClipDistance(float distance);
        
        /*! Add target distance percent to near clip
         * \param percent The percent to add
         */
        void SetNearClipPercent(int percent);
        
        /*! Sum distance to near clip
         * \param distance distance to add
         */
        void SumNearClipDistance(float distance);

        /*! Set camera far clip distance
         * \param distance camera far clip distance
         */
        void SetFarClipDistance(float distance);
        
        /*! Add target distance percent to far clip
         * \param percent The percent to add
         */
        void SetFarClipPercent(int percent);
        
        /*! Sum distance to far clip
         * \param distance distance to add
         */
         void SumFarClipDistance(float distance);

    private:
        /*! Rotates the camera around pivot postion
         * \param pivot Position of the pivot
         * \param x Y movement. [-1,1]
         * \param y X movement. [-1,1]
         */
        void RotateCamera(Vector3df pivot,qreal x, qreal y);        

        //! set axis coordinates for camera type
        void SetCoordinates();
        
        //! Framework
        Foundation::Framework *framework_;

        //! Scene Manager
        Scene::SceneManager *scene_;

        //! Pointer to new entity
        Scene::EntityPtr cam_entity_;
        
        //! The target entity
        Scene::Entity  *target_entity_;

        //! Camera type
        CameraType camera_type_;

        //! Camera projection: 
        Ogre::ProjectionType projection_type_;

        //! Camera polygon mode: PM_POINTS = 1, PM_WIREFRAME = 2, PM_SOLID = 3
        Ogre::PolygonMode polygon_mode_;

        //! Zoom acceleration
        qreal zoom_acceleration_;

        //! Minimun zoom distance
        qreal min_zoom_distance_;

        //! Maximun zoom distance
        qreal max_zoom_distance_;

        //! x axis translation
        Vector3df x_vector;

        //! y axis translation
        Vector3df y_vector;

        //! z axis translation
        Vector3df z_vector;

        //! minimum clip distance. 0.1 by default
        float min_clip_distance_;

        //! maximun clip distance. 2000 by default
        float max_clip_distance_;

        //! target distance from camera entity
        float target_distance_;

        //! actual camera near clip distance. 0.1 by default
        float near_clip_distance_;

        //! actual camera far clip distance. 2000 by default
        float far_clip_distance_;
    };
}

#endif