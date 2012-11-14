/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_GraphicsViewCanvas.h
    @brief  Makes possible to to embed arbitrary Qt UI elements into a 3D model. */

#pragma once

#include "IComponent.h"
#include "InputFwd.h"
#include "OgreModuleFwd.h"

#include <QEvent>

class QGraphicsScene;
class QGraphicsView;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

class RedirectedPaintWidget;

/// Makes possible to to embed arbitrary Qt UI elements into a 3D model.
/** <table class="header">
    <tr>
    <td>
    <h2>GraphicsViewCanvas</h2>
    Makes possible to to embed arbitrary Qt UI elements into a 3D model.
    The mouse input to the 3D object is passed to the actual UI element.

    Registered by CanvasPlugin.

    @todo Implement keyboard input.

    <b>Attributes</b>:
    <ul>
    <li> QString: outputTexture.
    <div> @copydoc outputTexture
    <li> uint : width.
    <div> @copydoc width
    <li> uint : height
    <div> @copydoc height
    <li> uint : submesh
    <div> @copydoc submesh
    </ul>

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li>"GraphicsScene": @copydoc GraphicsScene
    <li>"GraphicsView": @copydoc GraphicsView
    </ul>

    <b>Depends on the component @ref EC_Mesh "Mesh".</b>
    The material at the EC_Mesh at index which @c submesh points to is cloned for the usage of this component.
    Only material with single texture unit supported.

    </table> */
class EC_GraphicsViewCanvas : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_GraphicsViewCanvas", 52)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_GraphicsViewCanvas(Scene* scene);

    ~EC_GraphicsViewCanvas();

    /// The name of the texture asset we will paint the canvas to.
    /** Use this texture in the material of the mesh you're painting the canvas to.*/
    Q_PROPERTY(QString outputTexture READ getoutputTexture WRITE setoutputTexture);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, outputTexture);

    /// Specifies the width of the graphics view.
    Q_PROPERTY(uint width READ getwidth WRITE setwidth);
    DEFINE_QPROPERTY_ATTRIBUTE(uint, width);

    /// Specifies the height of the graphics view.
    Q_PROPERTY(uint height READ getheight WRITE setheight);
    DEFINE_QPROPERTY_ATTRIBUTE(uint, height);

    /// Specifies the submesh of the mesh on which to show this content.
    Q_PROPERTY(uint submesh READ getsubmesh WRITE setsubmesh);
    DEFINE_QPROPERTY_ATTRIBUTE(uint, submesh);

public slots:
    /// Returns the graphics scene in which content can be added.
    /** @note Do not store the pointer, but access it through this each time you need it. */
    QGraphicsScene *GraphicsScene() const { return graphicsScene; }

    /// Returns the graphics graphics view used to display the contents of the graphics scene.
    /** @note Do not store the pointer, but access it through this each time you need it. */
    QGraphicsView *GraphicsView() const { return graphicsView; }

private slots:
    void UpdateSignals();

private slots:
    void OnGraphicsSceneChanged(const QList<QRectF> &);
    void OnMouseEventReceived(MouseEvent *e);
    void OnDragEnterEvent(QDragEnterEvent *e);
    void OnDragLeaveEvent(QDragLeaveEvent *e);
    void OnDragMoveEvent(QDragMoveEvent *e);
    void OnDropEvent(QDropEvent *e);
    void OnMaterialChanged(uint, const QString &);
    void UpdateTexture();

private:
    void AttributesChanged();
    void SendMouseEvent(QEvent::Type type, const QPointF &point, MouseEvent *e);
    void SendMouseScrollEvent(MouseEvent *e, const QPointF &ptOnScene);
    Ogre::MaterialPtr OgreMaterial() const;

    QGraphicsScene *graphicsScene;
    QGraphicsView *graphicsView;
    RedirectedPaintWidget *paintTarget;
    InputContextPtr inputContext;
    bool isActivated;
};
