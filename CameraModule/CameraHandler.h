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
         * \param type Camera type to manages zoom and movement
         * \param parent Parent object
         */
        CameraHandler(Foundation::Framework *framework,CameraType type = Perspective, QObject *parent = 0);

        //! Destructor.
        virtual ~CameraHandler();

    public slots:
        /*! Destroys ogre texture.         
         */
        void DestroyCamera();

        /*! Creates new world entity with ec_ogrecamera and ec_placeable components
         */
        bool CreateCamera(Scene::ScenePtr scene);

        /*! Moves the camera.
         * \param x X movement. [-1,1]
         * \param y Y movement. [-1,1]
         */
        //! \todo syncs movement between xy view parameters and xy, xz, or zy world planes
        void Move(qreal x, qreal y);

        /*! Makes zoom to the camera.
         * \param delta mouse delta parameter
         */
        bool Zoom(qreal delta);

        /*! Focus the camera to world entity
         * \param entity World entity
         * \param offset
         */
        bool FocusToEntity(Scene::Entity *entity, Vector3df offset = Vector3df(-0.5f, -0.5f, -0.5f));

        /*! Gets QPixmap from ogre texture
         * \param imange_size Size of the image
         */
        QPixmap RenderCamera(QSize image_size);

    private:
        /*! Rotates the camera around pivot postion
         * \param pivot Position of the pivot
         * \param x Y movement. [-1,1]
         * \param y X movement. [-1,1]
         */
        void RotateCamera(Vector3df pivot,qreal x, qreal y);
        
        //! Framework
        Foundation::Framework *framework_;

        //! Pointer to new entity
        Scene::EntityPtr cam_entity_;
        
        //! The target entity
        Scene::Entity  *target_entity_;

        //! Ogre texture name
        std::string render_texture_name_;

        //! Unsigned chars to construct QPixmap
        Ogre::uchar *pixelData_;

        //! Camera type
        CameraType camera_type_;

        //! Zoom acceleration
        qreal zoom_acceleration_;

        //! movement acceleration 
        qreal x_acceleration_;

        //! movement acceleration 
        qreal y_acceleration_;

        //! Minimun zoom distance
        qreal min_zoom_distance_;

        //! Maximun zoom distance
        qreal max_zoom_distance_;
    };
}

#endif