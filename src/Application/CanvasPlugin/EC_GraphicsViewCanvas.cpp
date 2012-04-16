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

#include <Ogre.h>

#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>

EC_GraphicsViewCanvas::EC_GraphicsViewCanvas(Scene *scene) :
    IComponent(scene),
    outputTexture(this, "Output texture"),
    width(this, "Texture width"),
    height(this, "Texture height"),
    submesh(this, 0),
    graphicsScene(0),
    graphicsView(0),
    paintTarget(0),
    isActivated(false)
{
    graphicsView = new QGraphicsView();
    graphicsScene = new QGraphicsScene();
    graphicsView->setScene(graphicsScene);
    paintTarget = new RedirectedPaintWidget(); ///< @todo Never deleted - memory leak?
    paintTarget->setAttribute(Qt::WA_DontShowOnScreen, true);
    graphicsView->setViewport(paintTarget);
    graphicsView->setAttribute(Qt::WA_DontShowOnScreen, true);

    graphicsView->show();
    connect(graphicsScene, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(OnGraphicsSceneChanged(const QList<QRectF> &)), Qt::UniqueConnection);

    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setLineWidth(0);

    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(OnAttributeUpdated(IAttribute*)));

    inputContext = GetFramework()->Input()->RegisterInputContext("EC_GraphicsViewCanvas", 1000);
    connect(inputContext.get(), SIGNAL(MouseEventReceived(MouseEvent*)), this, SLOT(OnMouseEventReceived(MouseEvent*)));
}

EC_GraphicsViewCanvas::~EC_GraphicsViewCanvas()
{
    delete graphicsScene;
    delete graphicsView;
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
    OgreRenderer::RendererPtr renderer = framework->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
    if (!renderer || !renderer->MainCamera())
        return;

    // Test if mouse is on top of the 3D canvas
    QSize screenSize = framework->Ui()->GraphicsView()->size();
    float x = (float)mouseEvent->x / (float)screenSize.width();
    float y = (float)mouseEvent->y / (float)screenSize.height();
    EC_Camera *mainCamera = renderer->MainCameraComponent();
    if (!mainCamera)
        return;
    Ray mouseRay = mainCamera->GetMouseRay(x, y);

    RaycastResult *result = 0;
    EC_Mesh *mesh = ParentEntity() ? ParentEntity()->GetComponent<EC_Mesh>().get() : 0;
    if (mesh && mouseRay.Intersects(mesh->WorldOBB(), 0, 0))
        result = renderer->Raycast(mouseEvent->x, mouseEvent->y);

    const bool mouseOnTopOfThisCanvas = (result && result->entity == ParentEntity() /*&& (int)result->submesh == submesh.Get())*/ && GetFramework()->Input()->IsMouseCursorVisible());

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

    QPointF ptOnScene = graphicsView->mapToScene(graphicsView->width()*result->u, graphicsView->height()*result->v);

    if (!isActivated)
    {
        QEvent windowActivate(QEvent::WindowActivate);
        QApplication::sendEvent(graphicsScene, &windowActivate);
        isActivated = true;
    }

    switch(mouseEvent->eventType)
    {
    case MouseEvent::MouseMove:
        SendMouseEvent(QEvent::GraphicsSceneMouseMove, ptOnScene.x(), ptOnScene.y(), Qt::NoButton,
            (Qt::MouseButtons)mouseEvent->otherButtons, (Qt::KeyboardModifiers)mouseEvent->modifiers);
        break;
    case MouseEvent::MousePressed:
        SendMouseEvent(QEvent::GraphicsSceneMousePress, ptOnScene.x(), ptOnScene.y(), (Qt::MouseButton)mouseEvent->button,
            (Qt::MouseButtons)mouseEvent->otherButtons, (Qt::KeyboardModifiers)mouseEvent->modifiers);
        break;
    case MouseEvent::MouseReleased:
        SendMouseEvent(QEvent::GraphicsSceneMouseRelease, ptOnScene.x(), ptOnScene.y(), (Qt::MouseButton)mouseEvent->button,
            (Qt::MouseButtons)mouseEvent->otherButtons, (Qt::KeyboardModifiers)mouseEvent->modifiers);
        break;
    default:
        break;
    }

    QGraphicsItem *itemUnderMouse = graphicsScene->itemAt(ptOnScene);
    if (itemUnderMouse)
        inputContext->SetMouseCursorOverride(itemUnderMouse->cursor());
}

Ogre::MaterialPtr EC_GraphicsViewCanvas::OgreMaterial() const
{
    if (!ParentEntity())
        return Ogre::MaterialPtr();

    EC_Mesh *mesh = ParentEntity() ? ParentEntity()->GetComponent<EC_Mesh>().get() : 0;
    if (!mesh)
        return Ogre::MaterialPtr();

    // Check for submesh index validity
    int meshIndex = submesh.Get();
    if (meshIndex < 0 || meshIndex >= (int)mesh->GetNumMaterials())
        return Ogre::MaterialPtr();

    return Ogre::MaterialManager::getSingleton().getByName(mesh->GetMaterialName(meshIndex));
}

void EC_GraphicsViewCanvas::SendMouseEvent(QEvent::Type type, float x, float y, Qt::MouseButton button, Qt::MouseButtons mouseButtons, Qt::KeyboardModifiers keyboardModifiers)
{
    QPointF point(x, y);
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
    TextureAsset *textureAsset = dynamic_cast<TextureAsset*>(framework->Asset()->GetAsset(outputTexture.Get()).get());
    if (!textureAsset)
    {
        LogDebug("EC_GraphicsViewCanvas::UpdateTexture: textureAsset null.");
        return;
    }
    if (textureAsset->ogreTexture.isNull())
    {
        LogDebug("EC_GraphicsViewCanvas::UpdateTexture: textureAsset->ogreTexture null.");
        return;
    }

    if (!QString(textureAsset->ogreTexture->getName().c_str()).contains("Tex_NoMipMaps"))
    {
        // Re-create texture without mipmaps.
        OgreRenderer::RendererPtr renderer = GetFramework()->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
        QString textureName = renderer->GetUniqueObjectName("Tex_NoMipMaps").c_str();
        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(
            textureName.toStdString(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::TEX_TYPE_2D, 1, 1, 0, Ogre::PF_A8R8G8B8, 
            Ogre::TU_STATIC_WRITE_ONLY);
        if (texture.isNull())
            return;
        textureAsset->ogreTexture = texture;
        Ogre::MaterialPtr material = OgreMaterial();
        if (material.get())
            material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(textureName.toStdString());
    }

    textureAsset->SetContents(paintTarget->target.width(), paintTarget->target.height(), (const u8*)paintTarget->target.bits(),
        paintTarget->target.width() * paintTarget->target.height() * 4, Ogre::PF_A8R8G8B8, false, true, false);
}
