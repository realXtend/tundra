//$ HEADER_NEW_FILE $
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   CameraHandler.h
 *  @brief  Camera handler creates new world entity with ec_ogrecamera and ec_placeable componets. Manages zoom and movement of the camera depending on the camera type
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
        /*! Destroys ogre texture.         
         */
        void DestroyCamera();

        /*! Creates new world entity with ec_ogrecamera and ec_placeable components
         */
        bool CreateCamera(Scene::SceneManager *scene);

        /*! Moves the camera.
         * \param x X movement. [-1,1]
         * \param y Y movement. [-1,1]
         */
        //! \todo syncs movement between xy view parameters and xy, xz, or zy world planes
        void Move(qreal x, qreal y);

        /*! Makes zoom to the camera.
         * \param delta mouse delta parameter
         * \param modifiers key modifiers
         */
        bool Zoom(qreal delta, Qt::KeyboardModifiers modifiers);

        /*! Focus the camera to world entity
         * \param entity World entity
         * \param offset
         */
        bool FocusToEntity(Scene::Entity *entity, Vector3df offset = Vector3df(-0.5f, -0.5f, -0.5f));

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

    private:
        /*! Rotates the camera around pivot postion
         * \param pivot Position of the pivot
         * \param x Y movement. [-1,1]
         * \param y X movement. [-1,1]
         */
        void RotateCamera(Vector3df pivot,qreal x, qreal y);        

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

        //! position of the camera in z: negative (-1) or positive (1)
        char z_signed;

    };
}

#endif