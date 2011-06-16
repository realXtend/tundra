/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_QML.h
 *  @brief  EC_QML is a component for showing QML-elements in a texture.
 *  @note   no notes
 *
 */

#ifndef incl_EC_QML_EC_QML_h
#define incl_EC_QML_EC_QML_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"
#include "InputFwd.h"

#include <QDeclarativeView>
#include <QString>
#include <QPoint>
#include <QTimer>

#include "EC_3DCanvas.h"
#include "EC_Mesh.h"
#include "EC_Placeable.h"

/**
<table class="header">
<tr>
<td>
<h2>EC_QML</h2>
EC_QML Component.

Registered by RexLogic::RexLogicModule.

<b>No Attributes.</b>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

</table>
*/

class EC_Mesh;
class EC_3DCanvas;
class EC_Placeable;
class QMouseEvent;
class RaycastResult;

class EC_QML : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_QML);

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_QML(IModule *module);

    Foundation::RenderServiceInterface *renderer_;

    EC_3DCanvas* canvas_;
    EC_Mesh *mesh_;
    bool qml_ready;

    InputContextPtr input_;

    QDeclarativeView *qmlview_;

    //! Internal timer for updating inworld EC_3DCanvas.
    QTimer *renderTimer_;

public:
    /// Destructor.
    ~EC_QML();

    //! Rendering target submesh index.
    Q_PROPERTY(int renderSubmeshIndex READ getrenderSubmeshIndex WRITE setrenderSubmeshIndex);
    DEFINE_QPROPERTY_ATTRIBUTE(int, renderSubmeshIndex);

    //! Boolean for interactive mode, if true it will show context menus on mouse click events.
    Q_PROPERTY(bool interactive READ getinteractive WRITE setinteractive);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, interactive);

    //! QString to qml-file
    Q_PROPERTY(QString qmlsource READ getqmlsource WRITE setqmlsource);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, qmlsource);

    Q_PROPERTY(int renderinterval READ getrenderinterval WRITE setrenderinterval);
    DEFINE_QPROPERTY_ATTRIBUTE(int, renderinterval);

    /// IComponent Override
    bool IsSerializable() const { return true; }

public slots:
    void Render();

    //! Handle MouseEvents
    void HandleMouseInputEvent(MouseEvent *mouse);

    void ServerHandleAttributeChange(IAttribute *attribute, AttributeChange::Type changeType);

private slots:
    //! Prepares everything related to the parent widget and other needed components.
    void PrepareQML();

    //! Monitors this entitys added components.
    void ComponentAdded(IComponent *component, AttributeChange::Type change);

    //! Monitors this entitys removed components.
    void ComponentRemoved(IComponent *component, AttributeChange::Type change);

    //! Monitors this components Attribute changes.
    void AttributeChanged(IAttribute *attribute, AttributeChange::Type changeType);

    //! Create EC_Mesh component to the parent entity. Returns 0 if parent entity is not present
    EC_Mesh* GetOrCreateMeshComponent();

    //! Get parent entitys EC_3DCanvas. Return 0 if not present.
    EC_3DCanvas* GetOrCreateSceneCanvasComponent();

    //! Get parent entitys EC_Placeable. Return 0 if not present.
    EC_Placeable* GetOrCreatePlaceableComponent();

    /// Handles changes in QML-status
    void QMLStatus(QDeclarativeView::Status qmlstatus);

signals:
    void OnAttributeChanged(IAttribute*, AttributeChange::Type);

};

#endif
