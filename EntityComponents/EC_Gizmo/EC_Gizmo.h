/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Gizmo.h
 *  @brief  EC_Gizmo enables manipulators for values
 *  @author Nathan Letwory | http://www.letworyinteractive.com
 */
 
#ifndef incl_EC_Gizmo_EC_Gizmo_h
#define incl_EC_Gizmo_EC_Gizmo_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"
#include "OgreModuleFwd.h"
#include "Vector3D.h"

#include <QList>
#include <QVector2D>
#include <QVector3D>
#include <QQuaternion>

/// Enables manipulators for values
/**
<table class="header">
<tr>
<td>
<h2>Gizmo</h2>
Enables manipulators for values

Doesn't have any attributes.

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"Manipulate": 
<li>"AddEditableAttribute": ...
<li>"RemoveEditableAttribute": ...
<li>"ClearEditableAttributes": ...
</ul>

Doesn't react on any actions.

Does not emit any actions.

</table>
*/
class EC_Gizmo : public IComponent
{
    Q_OBJECT
    Q_ENUMS(DatumType)
    DECLARE_EC(EC_Gizmo);

public:
    /// Destructor.
    ~EC_Gizmo();

public slots:
    /// IComponent override. Returns true.
    virtual bool IsTemporary() const { return true; }

    ///
    /** @param
    */
    void Manipulate(const QVariant &datum);

    ///
    /** @param
        @param
    */
    void AddEditableAttribute(IComponent* component, const QString &attribute_name, const QString &subprop = QString()); 

    ///
    /** @param
        @param
   */
    void RemoveEditableAttribute(IComponent* component, const QString &attribute_name);

    ///
    void ClearEditableAttributes();

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_Gizmo(IModule *module);

    /// Renderer pointer.
    OgreRenderer::RendererWeakPtr renderer_;
    
    QString attribute_;
    QList<IAttribute *> attributes_;
    QStringList subproperties_;
};

#endif
