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
#include "IAttribute.h"
#include "Declare_EC.h"

#include "Vector3D.h"

#include <QList>
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
    Q_ENUMS(DatumType)
    DECLARE_EC(EC_3DGizmo);

public:
    /// Destructor.
    ~EC_3DGizmo();
    
public slots:
    virtual bool IsTemporary() const { return true; }

    void Manipulate(float movedx, float movedy, Vector3df changevec);
	void AddEditableAttribute(IComponent* component, QString attribute_name, QString subprop = QString()); 
    void ClearEditableAttributes();
    
private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_3DGizmo(IModule *module);

    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;
    
    QList<IAttribute *> attributes_;
    QStringList subproperties_;
};

#endif
