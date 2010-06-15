// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgrePlaceable_h
#define incl_OgreRenderer_EC_OgrePlaceable_h

#include "ComponentInterface.h"
#include "OgreModuleApi.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "Declare_EC.h"
#include <QtGui/qquaternion.h>
#include <QtGui/qvector3d.h>

namespace Ogre
{
    class SceneNode;
}

namespace OgreRenderer
{
    class Renderer;
    
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;
    
    //! Ogre placeable (scene node) component
    /*! \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API EC_OgrePlaceable : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OgrePlaceable);

        Q_OBJECT
        Q_PROPERTY(QVector3D Position READ GetQPosition WRITE SetQPosition)
        Q_PROPERTY(QVector3D Scale READ GetQScale WRITE SetQScale)
        Q_PROPERTY(QQuaternion Orientation READ GetQOrientation WRITE SetQOrientation)
        Q_PROPERTY(QVector3D LocalXAxis READ GetQLocalXAxis)
        Q_PROPERTY(QVector3D LocalYAxis READ GetQLocalYAxis)
        Q_PROPERTY(QVector3D LocalZAxis READ GetQLocalZAxis)
        Q_PROPERTY(float Yaw READ GetYaw)
        Q_PROPERTY(float Pitch READ GetPitch)
        Q_PROPERTY(float Roll READ GetRoll)

    public:
        virtual ~EC_OgrePlaceable();
        
        //! sets parent placeable
        /*! set null placeable to attach to scene root (the default)
            \param placeable new parent
         */
        void SetParent(Foundation::ComponentPtr placeable);
        
        //! sets position
        /*! \param position new position
         */
        void SetPosition(const Vector3df& position);
        //! sets orientation
        /*! \param orientation new orientation
         */
        void SetOrientation(const Quaternion& orientation);

        //! orients to look at a point in space
        /*! \param look_at point to look at
         */
        void LookAt(const Vector3df& look_at);        
        //! yaws the node
        /*! \param radians how many radians to yaw
         */
        void SetYaw(Real radians);
        //! pitches the node
        /*! \param radians how many radians to pitch
         */
        void SetPitch(Real radians);        
        //! rolls the node
        /*! \param radians how many radians to roll
         */
        void SetRoll(Real radians);           

        float GetYaw() const;
        float GetPitch() const;
        float GetRoll() const;
        
        //! sets scale
        /*! \param scale new scale
         */
        void SetScale(const Vector3df& scale);

        //! sets select priority
        /*! \param priority new select priority
         */
        void SetSelectPriority(int priority) { select_priority_ = priority; }
        
        //! gets parent placeable
        Foundation::ComponentPtr GetParent() { return parent_; }
        
        //! returns position
        Vector3df GetPosition() const;
        //! returns orientation
        Quaternion GetOrientation() const;
        //! returns scale
        Vector3df GetScale() const;

        Vector3df GetLocalXAxis() const;
        Vector3df GetLocalYAxis() const;
        Vector3df GetLocalZAxis() const;
        QVector3D GetQLocalXAxis() const;
        QVector3D GetQLocalYAxis() const;
        QVector3D GetQLocalZAxis() const;
        
        //! returns Ogre scenenode for attaching geometry.
        /*! Do not manipulate the pos/orientation/scale of this node directly
         */
        Ogre::SceneNode* GetSceneNode() const { return scene_node_; }
       
        //! returns Ogre scenenode for linking another placeable in OpenSim compatible way.
        /*! Only used by other placeables, or in other rare cases. Do not use if GetSceneNode() works for you,
            as this doesn't take scaling into account!
         */
        Ogre::SceneNode* GetLinkSceneNode() const { return link_scene_node_; }
                       
        //! returns select priority
        int GetSelectPriority() const { return select_priority_; }

        //! experimental accessors that use the new 3d vector etc types in Qt 4.6, for qproperties
        QVector3D GetQPosition() const;
        void SetQPosition(const QVector3D newpos);

        QQuaternion GetQOrientation() const;
        void SetQOrientation(const QQuaternion newort);

        QVector3D GetQScale() const;
        void SetQScale(const QVector3D newscale);

    public slots:
        //! translate
        /* 0 = x, 1 = y, 2 = z. returns new position */
        QVector3D translate( int axis, float amount);


    private:
        //! constructor
        /*! \param module renderer module
         */
        explicit EC_OgrePlaceable(Foundation::ModuleInterface* module);
        
        //! attaches scenenode to parent
        void AttachNode();
        
        //! detaches scenenode from parent
        void DetachNode();
        
        //! renderer
        RendererWeakPtr renderer_;
        
        //! parent placeable
        Foundation::ComponentPtr parent_;
        
        //! Ogre scene node for geometry. scale is handled here
        Ogre::SceneNode* scene_node_;

        //! Ogre scene node for linking. position & orientation are handled here, and the geometry scene node is attached to this
        Ogre::SceneNode* link_scene_node_;
        
        //! attached to scene hierarchy-flag
        bool attached_;
        
        //! selection priority for picking
        int select_priority_;
    };
}

#endif
