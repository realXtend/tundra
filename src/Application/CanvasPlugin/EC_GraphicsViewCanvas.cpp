/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_GraphicsViewCanvas.cpp
    @brief  Makes possible to to embed arbitrary Qt UI elements into a 3D model. */

#include "EC_GraphicsViewCanvas.h"

#include "AssetAPI.h"
#include "Renderer.h"
#include "OgreRenderingModule.h"
#include "UiAPI.h"
#include "UiGraphicsView.h"
#include "Scene.h"
#include "Framework.h"
#include "MouseEvent.h"
#include "TextureAsset.h"
#include "RedirectedPaintWidget.h"
#include "LoggingFunctions.h"
#include "Geometry/Ray.h"
#include "Entity.h"
#include "EC_Camera.h"
#include "EC_Mesh.h"
#include "InputAPI.h"
#include "OgreWorld.h"
#include "Profiler.h"

#include <Ogre.h>

#include <QApplication>
#include <QtGui>

EC_GraphicsViewCanvas::EC_GraphicsViewCanvas(Scene *scene) :
    IComponent(scene),
    outputTexture(this, "Output texture"),
    width(this, "Texture width"),
    height(this, "Texture height"),
    submesh(this, "Submesh", 0),
    graphicsScene(0),
    graphicsView(0),
    paintTarget(0),
    isActivated(false)
{
    graphicsView = new QGraphicsView();
    graphicsScene = new QGraphicsScene();
    graphicsView->setScene(graphicsScene);
    paintTarget = new RedirectedPaintWidget();
    paintTarget->setAttribute(Qt::WA_DontShowOnScreen, true);
    graphicsView->setViewport(paintTarget); // graphicsView takes ownership of the paintTarget viewport widget.
    graphicsView->setAttribute(Qt::WA_DontShowOnScreen, true);
    graphicsView->show();

    connect(graphicsScene, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(OnGraphicsSceneChanged(const QList<QRectF> &)), Qt::UniqueConnection);

    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setLineWidth(0);

    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(OnAttributeUpdated(IAttribute*)));

    inputContext = GetFramework()->Input()->RegisterInputContext("EC_GraphicsViewCanvas", 1000);
    connect(inputContext.get(), SIGNAL(MouseEventReceived(MouseEvent*)), this, SLOT(OnMouseEventReceived(MouseEvent*)));

    if (!framework->IsHeadless())
    {
        UiGraphicsView *gv = framework->Ui()->GraphicsView();
        connect(gv, SIGNAL(DragEnterEvent(QDragEnterEvent *, QGraphicsItem *)), SLOT(OnDragEnterEvent(QDragEnterEvent *)), Qt::UniqueConnection);
        connect(gv, SIGNAL(DragLeaveEvent(QDragLeaveEvent *)), SLOT(OnDragLeaveEvent(QDragLeaveEvent *)), Qt::UniqueConnection);
        connect(gv, SIGNAL(DragMoveEvent(QDragMoveEvent *, QGraphicsItem *)), SLOT(OnDragMoveEvent(QDragMoveEvent *)), Qt::UniqueConnection);
        connect(gv, SIGNAL(DropEvent(QDropEvent *, QGraphicsItem *)), SLOT(OnDropEvent(QDropEvent *)), Qt::UniqueConnection);
    }
    
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()));
}

EC_GraphicsViewCanvas::~EC_GraphicsViewCanvas()
{
    SAFE_DELETE(graphicsScene);
    SAFE_DELETE(graphicsView);
}

void EC_GraphicsViewCanvas::UpdateSignals()
{
    Entity* parent = ParentEntity();
    if (parent)
    {
        connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), SLOT(UpdateTexture()), Qt::UniqueConnection);

        EC_Mesh *mesh = ParentEntity()->GetComponent<EC_Mesh>().get();
        if (mesh)
        {
            connect(mesh, SIGNAL(MeshChanged()), this, SLOT(UpdateTexture()), Qt::UniqueConnection);
            connect(mesh, SIGNAL(MaterialChanged(uint, const QString &)), this, SLOT(UpdateTexture()), Qt::UniqueConnection);

            UpdateTexture();
        }
    }
}

void EC_GraphicsViewCanvas::OnGraphicsSceneChanged(const QList<QRectF> &)
{
    UpdateTexture();
}

void EC_GraphicsViewCanvas::OnAttributeUpdated(IAttribute *attribute)
{
    if (attribute == &outputTexture)
    {
        AssetPtr videoSurface = framework->Asset()->GetAsset(outputTexture.Get());
        if (!videoSurface && !framework->IsHeadless())
        {
            videoSurface = framework->Asset()->CreateNewAsset("Texture", outputTexture.Get());
            if (!videoSurface)
                ::LogError("Failed to create texture \"" + outputTexture.Get() + "\" for EC_GraphicsViewCanvas!");
        }
    }
    else if (attribute == &width || attribute == &height)
    {
        graphicsView->resize(width.Get(), height.Get());
        paintTarget->target = QImage(width.Get(), height.Get(), QImage::Format_ARGB32);
        paintTarget->target.fill(0);
    }
}

void EC_GraphicsViewCanvas::OnMouseEventReceived(MouseEvent *mouseEvent)
{
    OgreWorldPtr world = ParentScene()->GetWorld<OgreWorld>();
    if (!world)
        return;

    EC_Camera *mainCamera = world->Renderer()->MainCameraComponent();
    if (!mainCamera)
        return;

    // Test if mouse is on top of the 3D canvas
    QSize screenSize = framework->Ui()->GraphicsView()->size();
    float x = (float)mouseEvent->x / (float)screenSize.width();
    float y = (float)mouseEvent->y / (float)screenSize.height();

    Ray mouseRay = mainCamera->GetMouseRay(x, y);

    RaycastResult *result = 0;
    EC_Mesh *mesh = ParentEntity() ? ParentEntity()->GetComponent<EC_Mesh>().get() : 0;
    if (mesh && mouseRay.Intersects(mesh->WorldOBB(), 0, 0))
        result = world->Renderer()->Raycast(mouseEvent->x, mouseEvent->y);

    const bool mouseOnTopOfThisCanvas = result && result->entity == ParentEntity() &&
        (int)result->submesh == submesh.Get() && GetFramework()->Input()->IsMouseCursorVisible();

    if (!mouseOnTopOfThisCanvas)
    {
        inputContext->ClearMouseCursorOverride();
        if (isActivated)
        {
            isActivated = false;
            QEvent windowDeactivate(QEvent::WindowDeactivate);
            QApplication::sendEvent(graphicsScene, &windowDeactivate);
        }
        return;
    }

    if (!isActivated)
    {
        QEvent windowActivate(QEvent::WindowActivate);
        QApplication::sendEvent(graphicsScene, &windowActivate);
        isActivated = true;
    }

    QPointF ptOnScene = graphicsView->mapToScene(graphicsView->width()*result->u, graphicsView->height()*result->v);

    switch(mouseEvent->eventType)
    {
    case MouseEvent::MouseMove:
        SendMouseEvent(QEvent::GraphicsSceneMouseMove, ptOnScene, Qt::NoButton,
            (Qt::MouseButtons)mouseEvent->otherButtons, (Qt::KeyboardModifiers)mouseEvent->modifiers);
        break;
    case MouseEvent::MousePressed:
        SendMouseEvent(QEvent::GraphicsSceneMousePress, ptOnScene, (Qt::MouseButton)mouseEvent->button,
            (Qt::MouseButtons)mouseEvent->otherButtons, (Qt::KeyboardModifiers)mouseEvent->modifiers);
        break;
    case MouseEvent::MouseReleased:
        SendMouseEvent(QEvent::GraphicsSceneMouseRelease, ptOnScene, (Qt::MouseButton)mouseEvent->button,
            (Qt::MouseButtons)mouseEvent->otherButtons, (Qt::KeyboardModifiers)mouseEvent->modifiers);
        break;
    default:
        break;
    }

    QGraphicsItem *itemUnderMouse = graphicsScene->itemAt(ptOnScene);
    if (itemUnderMouse)
        inputContext->SetMouseCursorOverride(itemUnderMouse->cursor());
}

void EC_GraphicsViewCanvas::OnDragEnterEvent(QDragEnterEvent *e)
{
    QPoint mousePos = GetFramework()->Input()->MousePos();
    QGraphicsItem *itemUnderMouse = GetFramework()->Ui()->GraphicsView()->VisibleItemAtCoords(mousePos.x(), mousePos.y());
    const bool mouseOnTopOf2DMainUI = (itemUnderMouse != 0 && framework->Input()->IsMouseCursorVisible());
    if (!graphicsScene || !graphicsView || mouseOnTopOf2DMainUI)
        return;

    RaycastResult *result = ParentScene()->GetWorld<OgreWorld>()->Raycast(mousePos.x(), mousePos.y());
    const bool mouseOnTopOfThisCanvas = (result && result->entity == ParentEntity() && (int)result->submesh == submesh.Get());
    if (!mouseOnTopOfThisCanvas)
        return;

    QPointF ptOnView = graphicsView->mapToScene(graphicsView->width()*result->u, graphicsView->height()*result->v);

    QGraphicsSceneDragDropEvent sceneEvent(QEvent::GraphicsSceneDragEnter);
    sceneEvent.setScenePos(ptOnView);
    sceneEvent.setScreenPos(ptOnView.toPoint());
    sceneEvent.setButtons(e->mouseButtons());
    sceneEvent.setModifiers(e->keyboardModifiers());
    sceneEvent.setPossibleActions(e->possibleActions());
    sceneEvent.setProposedAction(e->proposedAction());
    sceneEvent.setDropAction(e->dropAction());
    sceneEvent.setMimeData(e->mimeData());
    sceneEvent.setWidget(graphicsView);
    sceneEvent.setSource(e->source());

    QApplication::sendEvent(graphicsScene, &sceneEvent);

    e->setAccepted(sceneEvent.isAccepted());
    if (sceneEvent.isAccepted())
        e->setDropAction(sceneEvent.dropAction());
}

void EC_GraphicsViewCanvas::OnDragLeaveEvent(QDragLeaveEvent *e)
{
    QPoint mousePos = GetFramework()->Input()->MousePos();
    QGraphicsItem *itemUnderMouse = GetFramework()->Ui()->GraphicsView()->VisibleItemAtCoords(mousePos.x(), mousePos.y());
    const bool mouseOnTopOf2DMainUI = (itemUnderMouse != 0 && framework->Input()->IsMouseCursorVisible());
    if (!graphicsScene || !graphicsView || mouseOnTopOf2DMainUI)
        return;

    RaycastResult *result = ParentScene()->GetWorld<OgreWorld>()->Raycast(mousePos.x(), mousePos.y());
    const bool mouseOnTopOfThisCanvas = (result && result->entity == ParentEntity() && (int)result->submesh == submesh.Get());
    if (!mouseOnTopOfThisCanvas)
        return;

    QPointF ptOnView = graphicsView->mapToScene(graphicsView->width()*result->u, graphicsView->height()*result->v);

    ///\todo QGraphicsView uses a mechanism of 'lastDragDropEvent'. Evaluate whether it would be better to apply it here as well.
    QGraphicsSceneDragDropEvent sceneEvent(QEvent::GraphicsSceneDragLeave);
    sceneEvent.setScenePos(ptOnView);
    sceneEvent.setScreenPos(ptOnView.toPoint());
    sceneEvent.setWidget(graphicsView);

    QApplication::sendEvent(graphicsScene, &sceneEvent);

    e->setAccepted(sceneEvent.isAccepted());
}

void EC_GraphicsViewCanvas::OnDragMoveEvent(QDragMoveEvent *e)
{
    QPoint mousePos = GetFramework()->Input()->MousePos();
    QGraphicsItem *itemUnderMouse = GetFramework()->Ui()->GraphicsView()->VisibleItemAtCoords(mousePos.x(), mousePos.y());
    const bool mouseOnTopOf2DMainUI = (itemUnderMouse != 0 && framework->Input()->IsMouseCursorVisible());
    if (!graphicsScene || !graphicsView || mouseOnTopOf2DMainUI)
        return;

    RaycastResult *result = ParentScene()->GetWorld<OgreWorld>()->Raycast(mousePos.x(), mousePos.y());
    const bool mouseOnTopOfThisCanvas = (result && result->entity == ParentEntity() && (int)result->submesh == submesh.Get());
    if (!mouseOnTopOfThisCanvas)
        return;

    QPointF ptOnView = graphicsView->mapToScene(graphicsView->width()*result->u, graphicsView->height()*result->v);

    QGraphicsSceneDragDropEvent sceneEvent(QEvent::GraphicsSceneDragMove);
    sceneEvent.setScenePos(ptOnView);
    sceneEvent.setScreenPos(ptOnView.toPoint());
    sceneEvent.setButtons(e->mouseButtons());
    sceneEvent.setModifiers(e->keyboardModifiers());
    sceneEvent.setPossibleActions(e->possibleActions());
    sceneEvent.setProposedAction(e->proposedAction());
    sceneEvent.setDropAction(e->dropAction());
    sceneEvent.setMimeData(e->mimeData());
    sceneEvent.setWidget(graphicsView);
    sceneEvent.setSource(e->source());

    QApplication::sendEvent(graphicsScene, &sceneEvent);

    e->setAccepted(sceneEvent.isAccepted());
    if (sceneEvent.isAccepted())
        e->setDropAction(sceneEvent.dropAction());
}

void EC_GraphicsViewCanvas::OnDropEvent(QDropEvent *e)
{
    QPoint mousePos = GetFramework()->Input()->MousePos();
    QGraphicsItem *itemUnderMouse = GetFramework()->Ui()->GraphicsView()->VisibleItemAtCoords(mousePos.x(), mousePos.y());
    const bool mouseOnTopOf2DMainUI = (itemUnderMouse != 0 && framework->Input()->IsMouseCursorVisible());
    if (!graphicsScene || !graphicsView || mouseOnTopOf2DMainUI)
        return;

    RaycastResult *result = ParentScene()->GetWorld<OgreWorld>()->Raycast(mousePos.x(), mousePos.y());
    const bool mouseOnTopOfThisCanvas = (result && result->entity == ParentEntity() && (int)result->submesh == submesh.Get());
    if (!mouseOnTopOfThisCanvas)
        return;

    QPointF ptOnView = graphicsView->mapToScene(graphicsView->width()*result->u, graphicsView->height()*result->v);

    QGraphicsSceneDragDropEvent sceneEvent(QEvent::GraphicsSceneDrop);
    sceneEvent.setScenePos(ptOnView);
    sceneEvent.setScreenPos(ptOnView.toPoint());
    sceneEvent.setButtons(e->mouseButtons());
    sceneEvent.setModifiers(e->keyboardModifiers());
    sceneEvent.setPossibleActions(e->possibleActions());
    sceneEvent.setProposedAction(e->proposedAction());
    sceneEvent.setDropAction(e->dropAction());
    sceneEvent.setMimeData(e->mimeData());
    sceneEvent.setWidget(graphicsView);
    sceneEvent.setSource(e->source());

    QApplication::sendEvent(graphicsScene, &sceneEvent);

    e->setAccepted(sceneEvent.isAccepted());
    if (sceneEvent.isAccepted())
        e->setDropAction(sceneEvent.dropAction());
}

Ogre::MaterialPtr EC_GraphicsViewCanvas::OgreMaterial() const
{
    EC_Mesh *mesh = ParentEntity() ? ParentEntity()->GetComponent<EC_Mesh>().get() : 0;
    if (!mesh)
        return Ogre::MaterialPtr();

    int meshIndex = submesh.Get();
    if (meshIndex < 0 || meshIndex >= (int)mesh->GetNumMaterials())
        return Ogre::MaterialPtr();

    return Ogre::MaterialManager::getSingleton().getByName(mesh->GetMaterialName(meshIndex));
}

void EC_GraphicsViewCanvas::SendMouseEvent(QEvent::Type type, const QPointF &point, Qt::MouseButton button,
    Qt::MouseButtons mouseButtons, Qt::KeyboardModifiers keyboardModifiers)
{
    QGraphicsSceneMouseEvent mouseEvent(type);
    mouseEvent.setButtonDownScenePos(button, point);
    mouseEvent.setButtonDownScreenPos(button, point.toPoint());

    mouseEvent.setScenePos(point);
    mouseEvent.setScreenPos(point.toPoint());
    mouseEvent.setLastScenePos(point);
    mouseEvent.setLastScreenPos(point.toPoint());
    mouseEvent.setButton(button);
    mouseEvent.setButtons(mouseButtons);

    mouseEvent.setModifiers(keyboardModifiers);
    mouseEvent.setAccepted(false);

    QApplication::sendEvent(graphicsView->scene(), &mouseEvent);
}

void EC_GraphicsViewCanvas::UpdateTexture()
{
    PROFILE(EC_GraphicsViewCanvas_UpdateTexture);

    if (!paintTarget || (paintTarget->target.width() == 0 || paintTarget->target.height() == 0))
        return;

    TextureAsset *textureAsset = dynamic_cast<TextureAsset*>(framework->Asset()->GetAsset(outputTexture.Get()).get());
    if (!textureAsset)
    {
        // Create a new programmatic texture asset if one didn't exist.
        OgreRenderer::RendererPtr renderer = GetFramework()->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
        textureAsset = dynamic_cast<TextureAsset*>(framework->Asset()->CreateNewAsset("Texture", renderer->GetUniqueObjectName("Tex_NoMipMaps").c_str()).get());
        if (!textureAsset)
        {
            LogWarning("EC_GraphicsViewCanvas::UpdateTexture: Could not create a texture asset to store the GraphicsViewCanvas contents!");
            return;
        }
    }

    // Allocate GPU memory for the texture if one didn't exist. Also make sure the texture is without mipmaps.
    if (textureAsset->ogreTexture.isNull() || !QString(textureAsset->ogreTexture->getName().c_str()).contains("Tex_NoMipMaps"))
    {
        // Re-create texture without mipmaps.
        OgreRenderer::RendererPtr renderer = GetFramework()->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
        QString textureName = renderer->GetUniqueObjectName("Tex_NoMipMaps").c_str();
        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(
            textureName.toStdString(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::TEX_TYPE_2D, paintTarget->target.width(), paintTarget->target.height(), 0, Ogre::PF_A8R8G8B8, Ogre::TU_STATIC_WRITE_ONLY);
        if (texture.isNull())
            return;
        textureAsset->ogreTexture = texture;
    }

    // Apply the texture onto the material. The material may change at runtime, so re-apply to retain the binding at all times.
    Ogre::MaterialPtr material = OgreMaterial();
    if (material.get() && !textureAsset->ogreTexture.isNull())
        material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTexture(textureAsset->ogreTexture);

    textureAsset->SetContents(paintTarget->target.width(), paintTarget->target.height(), (const u8*)paintTarget->target.bits(),
        paintTarget->target.width() * paintTarget->target.height() * 4, Ogre::PF_A8R8G8B8, false, true, false);
}
