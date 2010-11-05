/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_3DGizmo.h
 *  @brief  EC_3DGizmo enables manipulators for values
 *  @author Nathan Letwory | http://www.letworyinteractive.com
 */
 
#ifndef incl_EC_3DGizmo_EC_3DGizmo_h
#define incl_EC_3DGizmo_EC_3DGizmo_h

#include "IComponent.h"
#include "Declare_EC.h"

#include <QVector2D>
#include <QVector3D>
#include <QQuaternion>

namespace OgreRenderer
{
    class Renderer;
}

namespace Ogre
{
    class SceneNode;
    class ManualObject;
}

class EC_OpenSimPrim;

class EC_3DGizmo : public IComponent
{


    Q_OBJECT
    DECLARE_EC(EC_3DGizmo);
    

public:
    /// Destructor.
    ~EC_3DGizmo();
    
public slots:
    void Update3DGizmo();
    void SetGizmo(QString gizmo);
    void SetDatum(float scalar);
    void SetDatum(QVector2D& vec2);
    void SetDatum(QVector3D& vec3);
    void SetDatum(QQuaternion& quad);
    
private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_3DGizmo(IModule *module);

    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;
};

#endif
